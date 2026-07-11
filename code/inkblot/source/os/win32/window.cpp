#include <inkblot/basic/logging.hpp>
#include <inkblot/os/window.hpp>

#include "win32.hpp"

namespace ink::os
{
    static constexpr auto CloseRequestedProp = L"ink.close_requested";

    constinit inline auto ClassRegistered = false;

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

    auto convert_win32_key_to_our_key(::WPARAM Key, ::LPARAM Lparam) noexcept -> window_key
    {
        const auto ScanCode = static_cast<std::uint32_t>((Lparam >> 16) & 0xFF);
        const auto Extended = (Lparam & (1u << 24)) != 0;

        switch (Key) {
            case 'A': return window_key::a;
            case 'B': return window_key::b;
            case 'C': return window_key::c;
            case 'D': return window_key::d;
            case 'E': return window_key::e;
            case 'F': return window_key::f;
            case 'G': return window_key::g;
            case 'H': return window_key::h;
            case 'I': return window_key::i;
            case 'J': return window_key::j;
            case 'K': return window_key::k;
            case 'L': return window_key::l;
            case 'M': return window_key::m;
            case 'N': return window_key::n;
            case 'O': return window_key::o;
            case 'P': return window_key::p;
            case 'Q': return window_key::q;
            case 'R': return window_key::r;
            case 'S': return window_key::s;
            case 'T': return window_key::t;
            case 'U': return window_key::u;
            case 'V': return window_key::v;
            case 'W': return window_key::w;
            case 'X': return window_key::x;
            case 'Y': return window_key::y;
            case 'Z': return window_key::z;

            case '0': return window_key::num0;
            case '1': return window_key::num1;
            case '2': return window_key::num2;
            case '3': return window_key::num3;
            case '4': return window_key::num4;
            case '5': return window_key::num5;
            case '6': return window_key::num6;
            case '7': return window_key::num7;
            case '8': return window_key::num8;
            case '9': return window_key::num9;

            case VK_F1:  return window_key::f1;
            case VK_F2:  return window_key::f2;
            case VK_F3:  return window_key::f3;
            case VK_F4:  return window_key::f4;
            case VK_F5:  return window_key::f5;
            case VK_F6:  return window_key::f6;
            case VK_F7:  return window_key::f7;
            case VK_F8:  return window_key::f8;
            case VK_F9:  return window_key::f9;
            case VK_F10: return window_key::f10;
            case VK_F11: return window_key::f11;
            case VK_F12: return window_key::f12;
            case VK_F13: return window_key::f13;
            case VK_F14: return window_key::f14;
            case VK_F15: return window_key::f15;
            case VK_F16: return window_key::f16;
            case VK_F17: return window_key::f17;
            case VK_F18: return window_key::f18;
            case VK_F19: return window_key::f19;
            case VK_F20: return window_key::f20;
            case VK_F21: return window_key::f21;
            case VK_F22: return window_key::f22;
            case VK_F23: return window_key::f23;
            case VK_F24: return window_key::f24;

            case VK_UP:    return window_key::up;
            case VK_DOWN:  return window_key::down;
            case VK_LEFT:  return window_key::left;
            case VK_RIGHT: return window_key::right;

            case VK_HOME:   return window_key::home;
            case VK_END:    return window_key::end;
            case VK_PRIOR:  return window_key::page_up;
            case VK_NEXT:   return window_key::page_down;
            case VK_INSERT: return window_key::insert;
            case VK_DELETE: return window_key::del;

            case VK_ESCAPE: return window_key::escape;
            case VK_BACK:   return window_key::backspace;
            case VK_TAB:    return window_key::tab;
            case VK_SPACE:  return window_key::space;

            case VK_CAPITAL:  return window_key::caps_lock;
            case VK_SCROLL:   return window_key::scroll_lock;
            case VK_NUMLOCK:  return window_key::num_lock;
            case VK_SNAPSHOT: return window_key::print_screen;
            case VK_PAUSE:    return window_key::pause;

            case VK_LWIN: return window_key::left_super;
            case VK_RWIN: return window_key::right_super;
            case VK_APPS: return window_key::menu;

            case VK_OEM_3:      return window_key::grave;
            case VK_OEM_MINUS:  return window_key::minus;
            case VK_OEM_PLUS:   return window_key::equal;

            case VK_OEM_4:      return window_key::left_bracket;
            case VK_OEM_6:      return window_key::right_bracket;
            case VK_OEM_5:      return window_key::backslash;

            case VK_OEM_1:      return window_key::semicolon;
            case VK_OEM_7:      return window_key::apostrophe;

            case VK_OEM_COMMA:  return window_key::comma;
            case VK_OEM_PERIOD: return window_key::period;
            case VK_OEM_2:      return window_key::slash;

            case VK_NUMPAD0: return window_key::keypad0;
            case VK_NUMPAD1: return window_key::keypad1;
            case VK_NUMPAD2: return window_key::keypad2;
            case VK_NUMPAD3: return window_key::keypad3;
            case VK_NUMPAD4: return window_key::keypad4;
            case VK_NUMPAD5: return window_key::keypad5;
            case VK_NUMPAD6: return window_key::keypad6;
            case VK_NUMPAD7: return window_key::keypad7;
            case VK_NUMPAD8: return window_key::keypad8;
            case VK_NUMPAD9: return window_key::keypad9;

            case VK_DECIMAL:  return window_key::keypad_decimal;
            case VK_DIVIDE:   return window_key::keypad_divide;
            case VK_MULTIPLY: return window_key::keypad_multiply;
            case VK_SUBTRACT: return window_key::keypad_subtract;
            case VK_ADD:      return window_key::keypad_add;

            case VK_RETURN:  return Extended ? window_key::keypad_enter : window_key::enter;
            case VK_CONTROL: return Extended ? window_key::right_control : window_key::left_control;
            case VK_MENU:    return Extended ? window_key::right_alt : window_key::left_alt;

            case VK_SHIFT:
                switch (::MapVirtualKeyW(ScanCode, MAPVK_VSC_TO_VK_EX)) {
                    case VK_LSHIFT: return window_key::left_shift;
                    case VK_RSHIFT: return window_key::right_shift;
                    default:        return window_key::unknown;
                }

            default:
                return window_key::unknown;
        }
    }

    auto window::handle_deleter::operator()(const window::handle_type &Handle) const noexcept -> void
    {
        const auto WindowHandle = static_cast<::HWND>(Handle);
        ::DestroyWindow(WindowHandle);
    }

    window::window(std::uint32_t Width, std::uint32_t Height)
        : m_Handle{}
    {
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

            win32::ensure_os(::RegisterClassExW(&WindowClass) != 0, "Failed to register Win32 class");
            ClassRegistered = true;
        }

        auto ClientRect = ::RECT{.left = 0, .top = 0, .right = static_cast<int>(Width), .bottom = static_cast<int>(Height)};
        constexpr auto WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

        win32::ensure_os(::AdjustWindowRect(&ClientRect, WindowStyle, FALSE), "Failed to adjust Win32 client rect");

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
        win32::ensure_os(Handle != NULL, "Failed to construct Win32 window");
        m_Handle.reset(Handle);
    }

    auto window::native_handle() const noexcept -> window::handle_type
    {
        return *m_Handle;
    }

    auto window::process_events(const window::callback_type &Callback) const noexcept -> bool
    {
        const auto Handle = m_Handle.as<::HWND>();

        auto Message = ::MSG{};
        while (::PeekMessageW(&Message, Handle, 0, 0, PM_REMOVE)) {
            switch (Message.message) {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN: {
                    if ((Message.lParam & (1u << 30)) != 0) { // 30th bit = is repeat press; we ignore these!
                        break;
                    }

                    const auto KeyEvent = window_key_event{
                        .Key   = convert_win32_key_to_our_key(Message.wParam, Message.lParam), 
                        .State = window_key_state::pressed
                    };

                    if (!Callback(KeyEvent)) {
                        return false;
                    }
                } break;
                
                case WM_KEYUP:
                case WM_SYSKEYUP: {
                    const auto KeyEvent = window_key_event{
                        .Key   = convert_win32_key_to_our_key(Message.wParam, Message.lParam), 
                        .State = window_key_state::released
                    };

                    if (!Callback(KeyEvent)) {
                        return false;
                    }
                } break;

                default:
                    ::TranslateMessage(&Message);
                    ::DispatchMessageW(&Message);
            }
        }

        //
        // ::WM_CLOSE
        // This has to be handled separately since it won't be sent to the queue that
        // PeekMessageW looks at.
        if (::GetPropW(Handle, CloseRequestedProp) != nullptr) {
            ::RemovePropW(Handle, CloseRequestedProp);
            if (!Callback(window_quit_event{})) {
                return false;
            }
        }

        return true;
    }
} // namespace ink::os