#include <inkblot/os/thread.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>

#include <memory>

namespace ink::os
{
    auto thread::handle_deleter::operator()(const thread::handle_type &Handle) const noexcept -> void
    {
        ::CloseHandle(Handle);
    }

    thread::thread(thread::id_type Id, std::stop_source StopSource, thread::handle_type Handle, std::shared_ptr<exception_state> ExceptionState) noexcept
        : m_ThreadId{Id}
        , m_StopSource{std::move(StopSource)}
        , m_ExceptionState{std::move(ExceptionState)}
        , m_Handle{Handle}
    {
    }

    thread::~thread() noexcept
    {
        if (m_Handle) {
            request_stop();
            join();
        }
    }

    auto thread::create_thread(thread::data_type Data, thread::proc_type Proc) noexcept -> std::pair<thread::handle_type, thread::id_type>
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

        auto ThreadId = thread::id_type{};
        const auto Handle = reinterpret_cast<::HANDLE>(::_beginthreadex(nullptr, 0u, NativeProc, StartupPtr.get(), 0u, &ThreadId));
        if (Handle == nullptr) {
            log::error("In thread::create_thread, failed to create thread! ({})", ::GetLastError());
            return {nullptr, 0u};
        }


        StartupPtr.release();
        return {Handle, ThreadId};
    }

    auto thread::current_thread_id() noexcept -> thread::id_type
    {
        return ::GetCurrentThreadId();
    }

    auto thread::exception() const noexcept -> std::exception_ptr
    {
        auto Lock = std::scoped_lock{m_ExceptionState->Mutex};
        return m_ExceptionState->Exception;
    }

    auto thread::id() const noexcept -> thread::id_type
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