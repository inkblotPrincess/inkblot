#include <inkblot/os/thread.hpp>

#include "win32.hpp"

#include <memory>

namespace ink::os
{
    auto thread::handle_deleter::operator()(const thread::handle_type &Handle) const noexcept -> void
    {
        ::CloseHandle(Handle);
    }

    thread::~thread() noexcept
    {
        if (m_Handle) {
            request_stop();
            join();
        }
    }

    auto thread::create_thread(thread_id &ThreadId, thread::data_type Data, thread::proc_type Proc) noexcept -> thread::handle_type
    {
        struct startup_data
        {
            thread::data_type Data;
            thread::proc_type Proc;
        };

        auto StartupPtr = std::make_unique<startup_data>(Data, Proc);
        const auto NativeProc = +[](void *RawData) noexcept -> unsigned {
            auto Thread = std::unique_ptr<startup_data>(static_cast<startup_data *>(RawData));
            Thread->Proc(Thread->Data);

            return 0u;
        };

        const auto Handle = reinterpret_cast<::HANDLE>(::_beginthreadex(nullptr, 0u, NativeProc, StartupPtr.get(), 0u, &ThreadId));
        win32::ensure_os(Handle != nullptr, "Failed to create Win32 thread");

        StartupPtr.release();
        return Handle;
    }

    auto thread::exception() const noexcept -> std::exception_ptr
    {
        auto Lock = std::scoped_lock{m_ExceptionState->Mutex};
        return m_ExceptionState->Exception;
    }

    auto thread::id() const noexcept -> thread_id
    {
        return m_ThreadId;
    }

    auto thread::join() const noexcept -> void
    {
        if (::WaitForSingleObject(*m_Handle, INFINITE) == WAIT_FAILED) {
            log::error("In thread::join, failed to join thread! ({})", ::GetLastError());
        }
    }

    auto thread::native_handle() const noexcept -> thread::handle_type
    {
        return *m_Handle;
    }

    auto thread::request_stop() noexcept -> bool
    {
        return m_StopSource.request_stop();
    }
} // namespace ink::os