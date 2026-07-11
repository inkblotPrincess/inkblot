#include <inkblot/os/thread_id.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ink::os
{
    auto current_thread_id() noexcept -> thread_id
    {
        return ::GetCurrentThreadId();
    }
} // namespace ink::os