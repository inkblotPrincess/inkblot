#pragma once

#include <inkblot/gfx/renderer.hpp>

#include "vulkan.hpp"
#include "vulkan_context.hpp"

namespace ink::gfx::vk
{
    class vulkan_renderer : public irenderer_backend
    {
      public:
        explicit vulkan_renderer(const os::window::handle_type &WindowHandle);
        
        ~vulkan_renderer() = default;

        auto submit(const frame_context &Context) -> void override;

      private:
        ::VkAllocationCallbacks *m_Allocator;
        vulkan_context           m_Context;
    };
} // namespace ink::gfx::vk