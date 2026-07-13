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

        float R = 0.0f;
        float G = 0.0f;
        float B = 0.0f;
        float A = 0.0f;
    };

    struct irenderer_backend
    {
        virtual ~irenderer_backend() = default;

        virtual auto begin_frame(std::uint32_t Width, std::uint32_t Height) -> void = 0;
        virtual auto end_frame(const frame_context &Frame) -> void = 0;

        virtual auto cancel_frame() -> void = 0;
    };

    class renderer
    {
      public:
        static constexpr auto Frames = 2zu;

        struct config
        {
            os::window::handle_type WindowHandle;

            std::uint32_t Width;
            std::uint32_t Height;

            bool UseVSync;
        };

        renderer() = default;
        explicit renderer(api API, const renderer::config &Config);
        
        ~renderer();

        renderer(const renderer &) = delete;
        auto operator=(const renderer &) -> renderer& = delete;

        renderer(renderer &&) noexcept = delete;
        auto operator=(renderer &&) noexcept -> renderer& = delete;

        auto resize(std::uint32_t Width, std::uint32_t Height) -> void;

        template <typename func_type>
        requires std::is_invocable_r_v<void, func_type, frame_context&>
        auto submit(func_type &&Func) -> void
            pre(m_Backend != nullptr)
        {
            auto Frame = get_frame_context(frame_state::write_ready, frame_state::writing);
            if (!Frame.has_value()) { // no submission frames available? drop the frame!
                return;
            }

            std::forward<func_type>(Func)(*Frame);

            Frame->State.store(frame_state::render_ready, std::memory_order_release);
            m_FramesReady.release();
        }

      private:
        auto get_frame_context(const frame_state OldState, const frame_state NewState) -> std::optional<frame_context&>
            post(R: !R.has_value() || (R.has_value() && R->State == NewState));

        auto renderer_worker(std::stop_token StopToken) -> void;

        // We do +1 here so that the renderer can always wake up the renderer thread even if the queue is full
        std::counting_semaphore<Frames + 1zu> m_FramesReady;
        std::array<frame_context, Frames>     m_SubmissionFrames;

        std::unique_ptr<irenderer_backend> m_Backend;
        os::thread                         m_RendererThread;

        std::atomic_uint64_t m_Extent;
    };
} // namespace ink::gfx 