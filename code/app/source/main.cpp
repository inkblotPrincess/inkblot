#include <inkblot/basic/error.hpp>
#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/match.hpp>

#include <inkblot/gfx/renderer.hpp>

#include <inkblot/math/vector.hpp>

#include <inkblot/os/thread.hpp>
#include <inkblot/os/window.hpp>

#include <inkblot/profile/memory.hpp>

#include <print>

auto log_memory_snapshot() -> void
{
    const auto Snapshot = ink::profile::get_memory_metrics_snapshot();
    ink::log::debug(
        "Memory snapshot:\n -> Total Allocations: {}\n -> Live Allocations: {}\n -> Total Allocated Bytes: {}b\n -> Live Allocated Bytes: {}b",
        Snapshot.TotalAllocations,
        Snapshot.LiveAllocations,
        Snapshot.TotalAllocatedBytes,
        Snapshot.LiveAllocatedBytes
    );
}

auto run() -> void
{
    ink::log::register_logging_thread_name("main");
    ink::log::push_sink({
        .Name = "stdout", 
        .Output = 
            [](const ink::log::record &Record) {
                std::print(
                    "[{:%Y-%m-%d %H:%M:%S}] [{}] {:<{}} >> {} - {}({}:{})\n", 
                    std::chrono::floor<std::chrono::milliseconds>(Record.Timestamp), 
                    Record.Level, 
                    Record.ThreadName,
                    Record.MaxThreadNameLength,
                    Record.Message, 
                    Record.SourceFile, 
                    Record.SourceLine, 
                    Record.SourceColumn
                );
            }
        });

    constexpr auto WindowWidth  = 1080u;
    constexpr auto WindowHeight = 720u;

    auto Window   = ink::os::window{WindowWidth, WindowHeight};
    auto Renderer = ink::gfx::renderer{
        ink::gfx::api::vulkan, 
        {
            .WindowHandle = Window.native_handle(), 
            
            .Width    = WindowWidth, 
            .Height   = WindowHeight, 
            .UseVSync = true
        }
    };

    auto Running = true;
    const auto HandleEvent = [&Renderer, &Running](const ink::os::window_event &WindowEvent) noexcept {
        WindowEvent >> ink::match {
            [&Running]([[maybe_unused]] const ink::os::window_quit_event &) noexcept {
                ink::log::info("Shutting down...");
                Running = false;
            },

            [&Running](const ink::os::window_key_event &KeyEvent) noexcept {
                if (KeyEvent.State == ink::os::window_key_state::pressed) {
                    switch (KeyEvent.Key) {
                        case ink::os::window_key::escape:
                            Running = false;
                            ink::log::info("[ESC] Shutting down...");
                            break;

                        case ink::os::window_key::m:
                            log_memory_snapshot();
                            break;

                        default:
                            break;
                    }
                }
            },

            [&Renderer](const ink::os::window_resize_event &ResizeEvent) noexcept {
                Renderer.resize(ResizeEvent.Width, ResizeEvent.Height);
            }
        };
    };

    Window.set_callback(HandleEvent);

    while (Running) {
        Window.process_events();
        Renderer.submit([](ink::gfx::frame_context &FrameContext) {
            FrameContext.ClearColour = ink::math::vec4f{1.0f, 0.5f, 1.0f, 1.0f};
        });
    }
    
    Renderer.shutdown();
    log_memory_snapshot();
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int 
{
    try {
        run();
    } catch (ink::system_exception &Ex) {
        const auto &Code = Ex.error_code();
        ink::log::fatal("Unhandled system exception: {} ({}: {})", Ex.what(), Code.value(), Code.message());
        return 1;
    } catch (ink::exception &Ex) {
        ink::log::fatal("Unhandled exception: {}", Ex.what());
        return 1;
    } catch (std::runtime_error &Ex) {
        ink::log::fatal("Unhandled runtime error: {}", Ex.what());
        return 1;
    } catch (...) {
        ink::log::fatal("Unrecognised exception type.");
        return 1;
    }
    
    return 0;
}