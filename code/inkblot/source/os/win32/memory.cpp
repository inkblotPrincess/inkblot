#include <inkblot/os/memory.hpp>

#include "win32.hpp"

namespace ink::os 
{
    auto memory_allocate(std::size_t Size) noexcept -> void* 
    {
        auto *const Result = ::VirtualAlloc(nullptr, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        return Result;
    }

    auto memory_free(void *Memory) noexcept -> void 
    {
        ::VirtualFree(Memory, 0zu, MEM_RELEASE);
    }
} // namespace ink::os
