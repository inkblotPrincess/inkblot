#include <inkblot/basic/error.hpp>
#include <inkblot/basic/logging.hpp>

#include <inkblot/gfx/renderer.hpp>

#include "vulkan/vulkan_renderer.hpp"

namespace ink::gfx
{
    struct extent
    {
        std::uint32_t Width;
        std::uint32_t Height;
    };

    auto create_renderer_backend(api API, const renderer::config &Config) -> std::unique_ptr<irenderer_backend>
    {
        switch (API) {
            case api::vulkan: return std::make_unique<vk::vulkan_renderer>(Config);
            case api::dx12:   throw exception{"dx12 unimplemented"};
        }

        std::unreachable();
    }

    [[nodiscard]] inline auto pack_extent(std::uint32_t Width, std::uint32_t Height) noexcept -> std::uint64_t
    {
        return static_cast<std::uint64_t>(Width) | (static_cast<std::uint64_t>(Height) << 32u);
    }

    [[nodiscard]] inline auto unpack_extent(std::uint64_t Extent) noexcept -> extent
    {
        return {.Width = static_cast<std::uint32_t>(Extent), .Height = static_cast<std::uint32_t>(Extent >> 32u)};
    }

    renderer::renderer(api API, const renderer::config &Config)
        : m_FramesReady{0}
        , m_SubmissionFrames{}
        , m_Backend{create_renderer_backend(API, Config)}
        , m_RendererThread{[this](std::stop_token StopToken) { renderer_worker(StopToken); }}
        , m_Extent{pack_extent(Config.Width, Config.Height)}
    {
    }

    auto renderer::resize(std::uint32_t Width, std::uint32_t Height) -> void
    {
        m_Extent.store(pack_extent(Width, Height), std::memory_order_relaxed);
    }

    auto renderer::shutdown() -> void
    {
        m_RendererThread.request_stop();
        m_FramesReady.release();

        m_RendererThread.join();
        if (auto Exception = m_RendererThread.exception(); Exception != nullptr) {
            std::rethrow_exception(Exception);
        }
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
            const auto [Width, Height] = unpack_extent(m_Extent.load(std::memory_order_relaxed));
            m_Backend->begin_frame(Width, Height);
            
            m_FramesReady.acquire();
            if (StopToken.stop_requested()) { // stop might have been requested while sleeping
                break;
            }
            
            auto Frame = get_frame_context(frame_state::render_ready, frame_state::rendering);
            contract_assert(Frame.has_value());
            
            m_Backend->end_frame(*Frame);

            Frame->State.store(frame_state::write_ready, std::memory_order_release);
        }
    }
} // namespace ink::gfx