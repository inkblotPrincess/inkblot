#pragma once

#include "vulkan.hpp"
#include "vulkan_context.hpp"

#include <cstdint>
#include <vector>

namespace ink::gfx::vk
{
    class vulkan_swapchain
    {
      public:
        struct config
        {
            std::uint32_t Width  = 0u;
            std::uint32_t Height = 0u;

            bool VSync = true;
        };

        explicit vulkan_swapchain(const vulkan_swapchain::config &Config, const vulkan_context &Context, const ::VkAllocationCallbacks *Allocator);

        ~vulkan_swapchain();

        vulkan_swapchain(const vulkan_swapchain &) = delete;
        auto operator=(const vulkan_swapchain &) -> vulkan_swapchain& = delete;

        vulkan_swapchain(vulkan_swapchain &&From) noexcept;
        auto operator=(vulkan_swapchain &&From) noexcept -> vulkan_swapchain&;

        auto handle() const noexcept -> ::VkSwapchainKHR
            post(R: R != VK_NULL_HANDLE);
        
        auto image(std::size_t Index) const noexcept -> ::VkImage
            pre(Index < m_Images.size());
        
        auto image_count() const noexcept -> std::uint32_t;

        auto recreate(const vulkan_swapchain::config &Config) -> void
            pre(m_Context != nullptr);

      private:
        auto destroy() noexcept -> void;

        const ::VkAllocationCallbacks *m_Allocator = nullptr;
        const vulkan_context          *m_Context   = nullptr;

        ::VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

        std::vector<::VkImage>     m_Images;
        std::vector<::VkImageView> m_ImageViews;

        ::VkSurfaceFormatKHR m_SurfaceFormat;
        ::VkPresentModeKHR   m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        ::VkExtent2D         m_Extent;
    };
} // namespace ink::gfx::vk