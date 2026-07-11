#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/match.hpp>
#include <inkblot/gfx/renderer.hpp>
#include <inkblot/os/thread.hpp>
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
    ink::log::register_logging_thread_name("main");
    ink::log::push_sink({
        .Name = "stdout", 
        .Output = 
            [](const ink::log::record &Record) {
                std::print(
                    "[{:%Y-%m-%d %H:%M:%S}] [{}] {} >> {} - {}({}:{})\n", 
                    std::chrono::floor<std::chrono::milliseconds>(Record.Timestamp), 
                    Record.Level, 
                    Record.ThreadName,
                    Record.Message, 
                    Record.SourceFile, 
                    Record.SourceLine, 
                    Record.SourceColumn
                );
            }
        });

    const auto Window   = ink::os::window{1080u, 720u};
    const auto Renderer = ink::gfx::renderer{ink::gfx::api::vulkan, Window.native_handle()};

    auto Thread = ink::os::thread{[]([[maybe_unused]] std::stop_token StopToken, int a) noexcept -> void { ink::log::debug("int a = {}", a); }, 30};

    auto KeepRunning = true;
    while (KeepRunning) {
        KeepRunning = Window.process_events(handle_window_event);
    }
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int 
{
    run();
    return 0;
}