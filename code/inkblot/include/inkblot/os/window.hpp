#pragma once

#include <inkblot/basic/unique_handle.hpp>

#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ink::os
{
    struct window_quit_event
    {
        // No payload
    };

    using window_event = std::variant<window_quit_event>;

    class window
    {
      public:
        using handle_type = void*;

        struct config
        {
            std::uint32_t Width;
            std::uint32_t Height;
        };

        window() noexcept = default;

        ~window() noexcept = default;

        window(const window &) = delete;
        auto operator=(const window &) -> window& = delete;

        window(window &&) noexcept = default;
        auto operator=(window &&) noexcept -> window& = default;

        [[nodiscard]] static auto make(const window::config &Config) noexcept -> std::pair<window, bool>;

        template <typename func_type>
        requires std::is_nothrow_invocable_r_v<bool, func_type, const window_event &>
        auto process_events(func_type &&EventProcessFn) noexcept -> void
        {
            populate_event_queue();

            for (const auto &Event : m_EventQueue) {
                if (!EventProcessFn(Event)) {
                    break;
                }
            }

            m_EventQueue.clear();
        }

        [[nodiscard]] auto native_handle() const noexcept -> window::handle_type;

      private:
        struct handle_deleter
        {
            auto operator()(const window::handle_type &Handle) const noexcept -> void;
        };

        explicit window(window::handle_type Handle) noexcept
            pre(Handle != nullptr);
        
        auto populate_event_queue() noexcept -> void;

        unique_handle<window::handle_type, window::handle_deleter> m_Handle;
        std::vector<window_event> m_EventQueue;
    };
} // namespace ink::os