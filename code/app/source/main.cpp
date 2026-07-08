#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/match.hpp>
#include <inkblot/os/window.hpp>

#include <print>

auto handle_window_event(const ink::os::window_event &WindowEvent) noexcept -> bool
{
    return WindowEvent >> ink::match {
        []([[maybe_unused]] const ink::os::window_quit_event &) noexcept {
            ink::log::info("Shutting down...");
            return false;
        },

        [](const ink::os::window_key_event &KeyEvent) noexcept {
            if (KeyEvent.State == ink::os::window_key_state::pressed && KeyEvent.Key == ink::os::window_key::escape) {
                ink::log::info("[ESC] Shutting down...");
                return false;
            }

            return true;
        }
    };
}

auto run() -> void
{
    ink::log::push_sink({
        .Name = "stdout", 
        .Output = 
            [](const ink::log::record &Record) {
                std::print(
                    "[{:%Y-%m-%d %H:%M:%S}] [{}] >> {} - {}({}:{})\n", 
                    std::chrono::floor<std::chrono::milliseconds>(Record.Timestamp), 
                    Record.Level, 
                    Record.Message, 
                    Record.SourceFile, 
                    Record.SourceLine, 
                    Record.SourceColumn
                );
            }
        });

    const auto [WindowHandle, WindowSuccess] = ink::os::window_make(1080u, 720u);
    if (!WindowSuccess) {
        ink::log::fatal("Failed to create window!");
        return;
    }

    auto KeepRunning = true;
    while (KeepRunning) {
        KeepRunning = ink::os::process_window_events(WindowHandle, handle_window_event);
    }
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int 
{
    run();
    return 0;
}