#pragma once

#include <inkblot/gfx/renderer.hpp>

#include "vulkan.hpp"
#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"

namespace ink::gfx::vk
{
    class vulkan_renderer : public irenderer_backend
    {
      public:
        explicit vulkan_renderer(const renderer::config &Config);
        
        ~vulkan_renderer() = default;

        auto submit(const frame_context &Context) -> void override;

      private:
        ::VkAllocationCallbacks *m_Allocator;
        vulkan_context           m_Context;
        vulkan_swapchain         m_Swapchain;
    };
} // namespace ink::gfx::vk