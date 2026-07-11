#include <inkblot/basic/error.hpp>
#include <inkblot/basic/logging.hpp>
#include <inkblot/gfx/renderer.hpp>

namespace ink::gfx
{
    // @TEMP
    struct dummy_backend : irenderer_backend
    {
        dummy_backend([[maybe_unused]] const os::window::handle_type &WindowHandle) 
        {
        }

        auto submit_frame([[maybe_unused]] const frame_context &Context) -> void override 
        {
        }
    };

    auto create_renderer_backend(api API, const os::window::handle_type &WindowHandle) -> std::unique_ptr<irenderer_backend>
    {
        switch (API) {
            case api::vulkan: return std::make_unique<dummy_backend>(WindowHandle);
            case api::dx12:   throw exception{"dx12 unimplemented"};
        }

        std::unreachable();
    }

    renderer::renderer(api API, const os::window::handle_type &WindowHandle)
        : m_FramesInFlight{}
        , m_Backend{create_renderer_backend(API, WindowHandle)}
    {
    }

    auto renderer::get_frame_context() -> frame_context&
    {
        for (auto &Frame : m_FramesInFlight)
        {
            auto Expected = frame_state::available;
            if (Frame.State.compare_exchange_strong(Expected, frame_state::writing, std::memory_order_acquire)) {
                return Frame;
            }
        }

        std::unreachable();
    }
} // namespace ink::gfx