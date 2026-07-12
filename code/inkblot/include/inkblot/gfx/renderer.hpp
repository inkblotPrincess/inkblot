#pragma once

#include <inkblot/os/thread.hpp>
#include <inkblot/os/window.hpp>

#include <array>
#include <atomic>
#include <concepts>
#include <memory>
#include <optional>
#include <semaphore>

namespace ink::gfx
{
    enum class api
    {
        dx12,
        vulkan
    };

    enum class frame_state
    {
        write_ready,
        writing,
        render_ready,
        rendering
    };

    struct frame_context
    {
        std::atomic<frame_state> State = frame_state::write_ready; 
    };

    struct irenderer_backend
    {
        virtual ~irenderer_backend() = default;

        virtual auto submit(const frame_context &Context) -> void = 0;
    };

    class renderer
    {
      public:
        static constexpr auto Frames = 3zu;

        renderer() = default;
        explicit renderer(api API, const os::window::handle_type &WindowHandle);
        
        ~renderer();

        renderer(const renderer &) = delete;
        auto operator=(const renderer &) -> renderer& = delete;

        renderer(renderer &&) noexcept = delete;
        auto operator=(renderer &&) noexcept -> renderer& = delete;

        template <typename func_type>
        requires std::is_invocable_r_v<void, func_type, frame_context&>
        auto submit(func_type &&Func) -> void
            pre(m_Backend != nullptr)
        {
            auto Context = get_frame_context(frame_state::write_ready, frame_state::writing);
            if (!Context.has_value()) { // no submission frames available? drop the frame!
                return;
            }

            std::forward<func_type>(Func)(*Context);

            Context->State.store(frame_state::render_ready, std::memory_order_release);
            m_FramesReady.release();
        }

      private:
        auto get_frame_context(const frame_state OldState, const frame_state NewState) -> std::optional<frame_context&>
            post(R: !R.has_value() || (R.has_value() && R->State == NewState));

        auto renderer_worker(std::stop_token StopToken) -> void;

        std::counting_semaphore<Frames>    m_FramesReady;
        std::array<frame_context, Frames>  m_SubmissionFrames;
        std::unique_ptr<irenderer_backend> m_Backend;
        os::thread                         m_RendererThread;
    };
} // namespace ink::gfx 