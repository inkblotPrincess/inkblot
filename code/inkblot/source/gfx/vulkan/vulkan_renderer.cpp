#include "vulkan_renderer.hpp"

namespace ink::gfx::vk
{
    vulkan_renderer::vulkan_renderer(const renderer::config &Config)
        : m_Allocator{nullptr}
        , m_Context{Config.WindowHandle, m_Allocator}
        , m_Swapchain{{.Width = Config.Width, .Height = Config.Height, .VSync = Config.UseVSync}, m_Context, m_Allocator}
    {
    }

    auto vulkan_renderer::submit([[maybe_unused]] const frame_context &Context) -> void
    {
        
    }
} // namespace ink::gfx::vk