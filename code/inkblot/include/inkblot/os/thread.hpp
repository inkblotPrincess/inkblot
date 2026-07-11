#pragma once

#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/unique_handle.hpp>
#include <inkblot/os/thread_id.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
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

        thread() = delete;

        template <typename func_type, typename... arg_types>
        requires std::invocable<std::decay_t<func_type>&&, std::stop_token, std::decay_t<arg_types>&&...>
        explicit thread(func_type &&Func, arg_types &&...Arguments)
            : m_ThreadId{}
            , m_StopSource{}
            , m_ExceptionState{std::make_shared<thread::exception_state>()}
            , m_Handle{}
        {
            using invocation_type = std::tuple<std::decay_t<func_type>, std::shared_ptr<thread::exception_state>, std::stop_token, std::decay_t<arg_types>...>;
            auto Data = std::make_unique<invocation_type>(std::forward<func_type>(Func), m_ExceptionState, m_StopSource.get_token(), std::forward<arg_types>(Arguments)...);

            const auto Handle = create_thread(m_ThreadId, Data.get(), trampoline<invocation_type>);
            m_Handle.reset(Handle);

            Data.release();
        }
        
        ~thread() noexcept;

        thread(const thread &) = delete;
        auto operator=(const thread &) -> thread& = delete;

        thread(thread &&) noexcept = default;
        auto operator=(thread &&) noexcept -> thread& = default;

        auto exception() const noexcept -> std::exception_ptr
            pre(native_handle() != nullptr);

        auto id() const noexcept -> thread_id;

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

        static auto create_thread(thread_id &ThreadId, thread::data_type Data, thread::proc_type Proc) noexcept -> thread::handle_type
            pre(Data != nullptr)
            pre(Proc != nullptr);

        template <typename invocation_type>
        static auto trampoline(void *RawData) noexcept -> void
        {
            const auto ThreadId = current_thread_id();
            log::register_logging_thread_name(std::format("ink#{}", ThreadId), ThreadId);
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
            log::unregister_logging_thread_name(ThreadId);
        }

        thread_id                                  m_ThreadId;
        std::stop_source                           m_StopSource;
        std::shared_ptr<exception_state>           m_ExceptionState;
        unique_handle<handle_type, handle_deleter> m_Handle;
    };
} // namespace ink::os