#pragma once

#include <inkblot/os/window.hpp>

#include <array>
#include <atomic>
#include <concepts>
#include <memory>

namespace ink::gfx
{
    enum class api
    {
        dx12,
        vulkan
    };

    enum class frame_state
    {
        available,
        writing,
        rendering
    };

    struct frame_context
    {
        std::atomic<frame_state> State = frame_state::available; 
    };

    struct irenderer_backend
    {
        virtual ~irenderer_backend() = default;

        virtual auto submit_frame(const frame_context &Context) -> void = 0;
    };

    class renderer
    {
      public:
        renderer() = delete;
        explicit renderer(api API, const os::window::handle_type &WindowHandle);
        
        ~renderer() = default;

        renderer(const renderer &) = delete;
        auto operator=(const renderer &) -> renderer& = delete;

        renderer(renderer &&) noexcept = default;
        auto operator=(renderer &&) noexcept -> renderer& = default;

        template <typename func_type>
        requires std::is_invocable_r_v<void, func_type, frame_context&>
        auto submit(func_type &&Func) -> void
            pre(m_Backend != nullptr)
        {
            auto &Context = get_frame_context();
            std::forward<func_type>(Func)(Context);

            Context.State.store(frame_state::rendering, std::memory_order_release);
        }

      private:
        auto get_frame_context() -> frame_context&
            post(R: R.State == frame_state::writing);

        std::array<frame_context, 3>       m_FramesInFlight;
        std::unique_ptr<irenderer_backend> m_Backend;
    };
} // namespace ink::gfx 