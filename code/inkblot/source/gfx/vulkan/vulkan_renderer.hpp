#pragma once

#include <inkblot/gfx/renderer.hpp>

#include "vulkan.hpp"
#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"

#include <array>
#include <vector>

namespace ink::gfx::vk
{
    struct frame_resources
    {
        ::VkCommandPool CommandPool     = VK_NULL_HANDLE;
        ::VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

        ::VkSemaphore ImageAvailable = VK_NULL_HANDLE;

        ::VkFence RenderFence = VK_NULL_HANDLE;
    };

    class vulkan_renderer : public irenderer_backend
    {
      public:
        explicit vulkan_renderer(const renderer::config &Config);
        
        ~vulkan_renderer();

        auto begin_frame(std::uint32_t Width, std::uint32_t Height) -> void override;
        
        auto end_frame(const frame_context &Frame) -> void override;

        auto cancel_frame() -> void override;

      private:
        static constexpr auto FramesInFlight = 2zu;

        auto recreate_swapchain(std::uint32_t Width, std::uint32_t Height) -> void;
        auto destroy() noexcept -> void;

        ::VkAllocationCallbacks *m_Allocator;
        vulkan_context           m_Context;
        vulkan_swapchain         m_Swapchain;

        std::array<frame_resources, FramesInFlight> m_Frames;
        std::size_t m_CurrentFrame = 0zu;
        std::uint32_t m_CurrentImage = 0u;

        std::vector<::VkSemaphore> m_RenderFinished;

        bool m_SwapchainNeedsRecreation = false;
    };
} // namespace ink::gfx::vk