#include <inkblot/basic/error.hpp>
#include <inkblot/basic/logging.hpp>

#include <inkblot/gfx/renderer.hpp>

#include "vulkan/vulkan_renderer.hpp"

namespace ink::gfx
{
    auto create_renderer_backend(api API, const renderer::config &Config) -> std::unique_ptr<irenderer_backend>
    {
        switch (API) {
            case api::vulkan: return std::make_unique<vk::vulkan_renderer>(Config);
            case api::dx12:   throw exception{"dx12 unimplemented"};
        }

        std::unreachable();
    }

    renderer::renderer(api API, const renderer::config &Config)
        : m_FramesReady{0}
        , m_SubmissionFrames{}
        , m_Backend{create_renderer_backend(API, Config)}
        , m_RendererThread{[this](std::stop_token StopToken) { renderer_worker(StopToken); }}
    {
    }

    renderer::~renderer()
    {
        m_RendererThread.request_stop();
        m_FramesReady.release();
    }

    auto renderer::get_frame_context(const frame_state OldState, const frame_state NewState) -> std::optional<frame_context&>
    {
        for (auto &Frame : m_SubmissionFrames)
        {
            auto Expected = OldState;
            if (Frame.State.compare_exchange_strong(Expected, NewState, std::memory_order_acquire)) {
                return Frame;
            }
        }

        return std::nullopt;
    }

    auto renderer::renderer_worker(std::stop_token StopToken) -> void
    {
        while (!StopToken.stop_requested())
        {
            m_FramesReady.acquire();
            if (StopToken.stop_requested()) { // stop might have been requested while sleeping
                break;
            }
            
            auto Context = get_frame_context(frame_state::render_ready, frame_state::rendering);
            contract_assert(Context.has_value());
            
            m_Backend->submit(*Context);

            Context->State.store(frame_state::write_ready, std::memory_order_release);
        }
    }
} // namespace ink::gfx