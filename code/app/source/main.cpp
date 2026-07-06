#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/match.hpp>
#include <inkblot/os/window.hpp>

#include <print>

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

    auto [Window, WindowSuccess] = ink::os::window::make({.Width = 1080u, .Height = 720u});
    if (!WindowSuccess) {
        ink::log::fatal("Failed to create window!");
        return;
    }

    auto Running = true;
    while (Running) {
        Window.process_events([&](const ink::os::window_event &WindowEvent) noexcept -> bool {
            return WindowEvent >> ink::match {
                [&]([[maybe_unused]] const ink::os::window_quit_event &) noexcept {
                    ink::log::info("Shutting down...");
                    Running = false;

                    return false;
                }
            };
        });
    }
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int 
{
    run();
    return 0;
}