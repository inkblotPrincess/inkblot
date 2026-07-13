#include "vulkan_renderer.hpp"

namespace ink::gfx::vk
{
    vulkan_renderer::vulkan_renderer(const os::window::handle_type &WindowHandle)
        : m_Allocator{nullptr}
        , m_Context{WindowHandle, m_Allocator}
    {
    }

    auto vulkan_renderer::submit([[maybe_unused]] const frame_context &Context) -> void
    {
        
    }
} // namespace ink::gfx::vk