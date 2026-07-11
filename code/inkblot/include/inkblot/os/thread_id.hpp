#pragma once

#include <cstdint>

namespace ink::os
{
    using thread_id = std::uint32_t;

    [[nodiscard]] auto current_thread_id() noexcept -> thread_id;
} // namespace ink::os