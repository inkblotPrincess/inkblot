#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/rvo.hpp>
#include <inkblot/gfx/renderer.hpp>

namespace ink::gfx
{
    // @TEMP
    struct dummy_backend : irenderer_backend
    {
        static auto make([[maybe_unused]] const os::window_handle &WindowHandle) -> std::pair<std::unique_ptr<dummy_backend>, bool>
            post(R: (R.first == nullptr && !R.second) || (R.first != nullptr && R.second))
        {
            return MAKE_PAIR(std::make_unique<dummy_backend>(), true);
        }

        auto submit_frame([[maybe_unused]] const frame_context &Context) -> void override 
        {
        }
    };

    renderer::renderer(std::unique_ptr<irenderer_backend> Backend) noexcept
        : m_Backend{std::move(Backend)}
    {
    }

    auto renderer::make(api API, const os::window_handle &WindowHandle) noexcept -> std::pair<renderer, bool>
    {
        auto [Backend, BackendSuccess] = [API, &WindowHandle] noexcept {
            switch (API) {
                case api::vulkan: return dummy_backend::make(WindowHandle); // @TEMP
                case api::dx12:   contract_assert(false); // @TODO
            }

            std::unreachable();
        }();

        if (!BackendSuccess) {
            log::error("Failed to initialise renderer backend!");
            return MAKE_PAIR(renderer{nullptr}, false);
        }

        return MAKE_PAIR(renderer{std::move(Backend)}, true);
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