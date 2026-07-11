#pragma once

#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/rvo.hpp>
#include <inkblot/basic/unique_handle.hpp>

#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::os
{
    class thread
    {
      public:
        using handle_type = void*;
        using id_type     = std::uint32_t;

        thread() = delete;
        
        ~thread() noexcept;

        thread(const thread &) = delete;
        auto operator=(const thread &) -> thread& = delete;

        thread(thread &&) noexcept = default;
        auto operator=(thread &&) noexcept -> thread& = default;

        [[nodiscard]] static auto current_thread_id() noexcept -> thread::id_type;

        template <typename func_type, typename... arg_types>
        requires std::invocable<std::decay_t<func_type>&&, std::stop_token, std::decay_t<arg_types>&&...>
        [[nodiscard]] static auto make(func_type &&Func, arg_types &&...Arguments) -> std::optional<thread>
        {
            auto StopSource     = std::stop_source{};
            auto ExceptionState = std::make_shared<thread::exception_state>();

            using invocation_type = std::tuple<std::decay_t<func_type>, std::shared_ptr<thread::exception_state>, std::stop_token, std::decay_t<arg_types>...>;
            auto Data = std::make_unique<invocation_type>(std::forward<func_type>(Func), ExceptionState, StopSource.get_token(), std::forward<arg_types>(Arguments)...);

            const auto [Handle, ThreadId] = create_thread(Data.get(), trampoline<invocation_type>);
            if (Handle == nullptr) {
                log::error("In thread::make, failed to create native thread!");
                return std::nullopt;
            }

            Data.release();

            return MAKE_OPTIONAL(thread{ThreadId, std::move(StopSource), Handle, std::move(ExceptionState)});
        }

        auto exception() const noexcept -> std::exception_ptr
            pre(native_handle() != nullptr);

        auto id() const noexcept -> thread::id_type;

        auto join() const noexcept -> void
            pre(native_handle() != nullptr);

        [[nodiscard]] auto native_handle() const noexcept -> thread::handle_type;

        auto request_stop() noexcept -> bool;

      private:
        using data_type = void*;
        using proc_type = void(*)(thread::data_type) noexcept;

        struct exception_state
        {
            std::exception_ptr Exception;
            std::mutex         Mutex;
        };

        struct handle_deleter
        {
            auto operator()(const thread::handle_type &Handle) const noexcept -> void;
        };

        explicit thread(thread::id_type Id, std::stop_source StopSource, thread::handle_type Handle, std::shared_ptr<exception_state> ExceptionState) noexcept
            pre(Handle != nullptr)
            pre(ExceptionState != nullptr);

        static auto create_thread(thread::data_type Data, thread::proc_type Proc) noexcept -> std::pair<thread::handle_type, thread::id_type>
            pre(Data != nullptr)
            pre(Proc != nullptr);

        template <typename invocation_type>
        static auto trampoline(void *RawData) noexcept -> void
        {
            const auto ThreadId = current_thread_id();
            log::info("Starting thread #{}", ThreadId);

            auto Data = std::unique_ptr<invocation_type>(static_cast<invocation_type *>(RawData));
            std::apply(
                []<typename callable_type, typename... arg_types>
                (callable_type &&Callable, std::shared_ptr<exception_state> ExceptionState, std::stop_token StopToken, arg_types &&...Arguments) {
                    try {
                        std::invoke(std::forward<callable_type>(Callable), StopToken, std::forward<arg_types>(Arguments)...);
                    } catch (...) {
                        {
                            auto Lock = std::scoped_lock{ExceptionState->Mutex};
                            ExceptionState->Exception = std::current_exception();
                        }
                        log::error("Exception thrown from thread!");        
                    }
                },
                std::move(*Data)
            );

            log::info("Ending thread #{}", ThreadId);
        }

        thread::id_type                                            m_ThreadId;
        std::stop_source                                           m_StopSource;
        std::shared_ptr<exception_state>                           m_ExceptionState;
        unique_handle<thread::handle_type, thread::handle_deleter> m_Handle;
    };
} // namespace ink::os