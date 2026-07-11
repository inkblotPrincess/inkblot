#pragma once

#include <inkblot/basic/unique_handle.hpp>

#include <cstdint>
#include <format>
#include <type_traits>
#include <variant>
#include <vector>

namespace ink::os
{
    enum class window_key
    {
        unknown,
        a, b, c, d, e, f, g, h, i, j, k, l, m, 
        n, o, p, q, r, s, t, u, v, w, x, y, z,
        num0, num1, num2, num3, num4, num5, num6, num7, num8, num9,
        f1,  f2,  f3,  f4,  f5,  f6,  f7,  f8,  f9,  f10, f11, f12,
        f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24,
        left_shift,   right_shift, 
        left_control, right_control,
        left_alt,     right_alt,
        left_super,   right_super,
        menu,
        up, down, left, right,
        home, end, page_up, page_down, insert, del,
        enter, escape, backspace, tab, space,
        caps_lock, scroll_lock, num_lock, print_screen, pause,
        grave, minus, equal,
        left_bracket, right_bracket, backslash, 
        semicolon, apostrophe, comma, period, slash,
        keypad0, keypad1, keypad2, keypad3, keypad4,
        keypad5, keypad6, keypad7, keypad8, keypad9,
        keypad_decimal, keypad_divide, keypad_multiply, keypad_subtract,
        keypad_add, keypad_enter, keypad_equal,
    };

    enum class window_key_state
    {
        pressed,
        released
    };

    struct window_key_event
    {
        window_key       Key;
        window_key_state State;
    };

    struct window_quit_event
    {
        // No payload
    };

    using window_event = std::variant<window_key_event, window_quit_event>;

    class window
    {
      public:
        using callback_type = bool(*)(const window_event &) noexcept;
        using handle_type   = void*;

        window() = delete;
        explicit window(std::uint32_t Width, std::uint32_t Height);

        ~window() = default;

        window(const window &) = delete;
        auto operator=(const window &) -> window& = delete;

        window(window &&) noexcept = default;
        auto operator=(window &&) noexcept -> window& = default;

        [[nodiscard]] auto native_handle() const noexcept -> window::handle_type;

        [[nodiscard]] auto process_events(const window::callback_type &Callback) const noexcept -> bool
            pre(native_handle() != nullptr)
            pre(Callback != nullptr);

      private:
        struct handle_deleter
        {
            auto operator()(const window::handle_type &Handle) const noexcept -> void;
        };
        
        unique_handle<window::handle_type, window::handle_deleter> m_Handle;
    };

    auto format_to(auto Out, const window_key_event &KeyEvent)
    {
        return std::format_to(Out, "key({}, {})", KeyEvent.Key, KeyEvent.State);
    }
} // namespace ink::os