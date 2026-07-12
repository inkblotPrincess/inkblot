#pragma once

#include <cstddef>

namespace ink::os {
    auto memory_allocate(std::size_t Size) noexcept -> void*;
    
    auto memory_free(void *Memory) noexcept -> void
        pre(Memory != nullptr);
} // namespace ink::os
