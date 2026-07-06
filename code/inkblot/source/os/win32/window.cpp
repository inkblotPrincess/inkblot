#include <inkblot/basic/logging.hpp>
#include <inkblot/os/window.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ink::os
{
    static constexpr auto CloseRequestedProp = L"ink.close_requested";

    auto CALLBACK win32_callback(::HWND Handle, ::UINT Message, ::WPARAM Wparam, ::LPARAM Lparam) -> ::LRESULT {
        switch (Message) {
            //
            // ::WM_CLOSE
            // WM_CLOSE is sent directly to our callback so we have to do this workaround to inform our window
            // when a close request happened
            case WM_CLOSE:
                ::SetPropW(Handle, CloseRequestedProp, reinterpret_cast<::HANDLE>(1));
                return 0;

            default:
                return ::DefWindowProcW(Handle, Message, Wparam, Lparam);
        }
    }

    auto window::handle_deleter::operator()(const window::handle_type &Handle) const noexcept -> void
    {
        const auto Hwnd = static_cast<::HWND>(Handle);
        ::DestroyWindow(Hwnd);
    }

    window::window(window::handle_type Handle) noexcept
        : m_Handle{Handle}
        , m_EventQueue{}
    {
    }

    auto window::make(const window::config &Config) noexcept -> std::pair<window, bool>
    {
        static auto ClassRegistered = false;
        if (!ClassRegistered) {
            const auto WindowClass = ::WNDCLASSEXW{
                .cbSize        = sizeof(::WNDCLASSEXW),
                .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                .lpfnWndProc   = win32_callback,
                .hInstance     = ::GetModuleHandleW(nullptr),
                .hCursor       = ::LoadCursorA(nullptr, IDC_ARROW),
                .hbrBackground = static_cast<::HBRUSH>(::GetStockObject(GRAY_BRUSH)),
                .lpszClassName = L"inkblot_wndclass"
            };

            if (::RegisterClassExW(&WindowClass) == 0) {
                log::error("In window::make, failed to register Win32 class! ({})", ::GetLastError());
                return std::make_pair(window{}, false);
            }

            ClassRegistered = true;
        }

        auto ClientRect = ::RECT{.left = 0, .top = 0, .right = static_cast<int>(Config.Width), .bottom = static_cast<int>(Config.Height)};
        constexpr auto WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

        if (!::AdjustWindowRect(&ClientRect, WindowStyle, FALSE) != FALSE) {
            log::error("In window::make, could not adjust Win32 rect! ({})", ::GetLastError());
            return std::make_pair(window{}, false);
        }

        const auto WindowWidth  = ClientRect.right - ClientRect.left;
        const auto WindowHeight = ClientRect.bottom - ClientRect.top;

        const auto Handle = ::CreateWindowExW(
            WS_EX_APPWINDOW, 
            L"inkblot_wndclass", 
            L"Ink Window", 
            WindowStyle, 
            CW_USEDEFAULT, 
            CW_USEDEFAULT, 
            WindowWidth, 
            WindowHeight, 
            nullptr, 
            nullptr, 
            ::GetModuleHandleW(nullptr), 
            nullptr);
        if (Handle == NULL) {
            log::error("In window::make, could not construct Win32 window! ({})", ::GetLastError());
            return std::make_pair(window{}, false);
        }

        return std::make_pair(window{Handle}, true);
    }

    auto window::native_handle() const noexcept -> window::handle_type
    {
        return m_Handle.get();
    }

    auto window::populate_event_queue() noexcept -> void
    {
        const auto Handle = static_cast<::HWND>(*m_Handle);

        auto Message = ::MSG{};
        while (::PeekMessageW(&Message, Handle, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&Message);
            ::DispatchMessageW(&Message);
        }

        //
        // ::WM_CLOSE
        // This has to be handled separately since it won't be sent to the queue that
        // PeekMessageW looks at.
        if (::GetPropW(Handle, CloseRequestedProp) != nullptr) {
            ::RemovePropW(Handle, CloseRequestedProp);
            m_EventQueue.emplace_back(window_quit_event{});
        }
    }
} // namespace ink::os