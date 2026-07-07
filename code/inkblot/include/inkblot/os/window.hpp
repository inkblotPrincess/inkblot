#pragma once

#include <inkblot/basic/unique_handle.hpp>

#include <cstdint>
#include <functional>
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

    using native_window_handle = void*;

    struct native_window_handle_deleter
    {
        auto operator()(const native_window_handle &Handle) const noexcept -> void;
    };

    using window_handle   = unique_handle<native_window_handle, native_window_handle_deleter>;
    using window_callback = std::function<void(const window_event&)>;

    [[nodiscard]] auto window_make(std::uint32_t Width, std::uint32_t Height) noexcept -> std::pair<window_handle, bool>
        post(R: (*R.first == nullptr && !R.second) || (*R.first != nullptr && R.second));

    auto process_window_events(const window_handle &WindowHandle, const window_callback &Callback) noexcept -> void
        pre(*WindowHandle != nullptr)
        pre(Callback != nullptr);
} // namespace ink::os