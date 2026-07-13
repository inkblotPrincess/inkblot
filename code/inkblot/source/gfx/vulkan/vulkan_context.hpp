#pragma once

#include <inkblot/os/window.hpp>

#include "vulkan.hpp"

#include <cstdint>
#include <limits>

namespace ink::gfx::vk
{
    class vulkan_context
    {
      public:
        struct queue
        {
            static constexpr auto InvalidQueueFamily = std::numeric_limits<std::uint32_t>::max();

            ::VkQueue Handle = VK_NULL_HANDLE;

            std::uint32_t FamilyIndex = InvalidQueueFamily;
            std::uint32_t QueueIndex  = 0u;

            [[nodiscard]] constexpr explicit operator bool() const noexcept
            {
                return Handle != VK_NULL_HANDLE;
            }
        };

        struct queues
        {
            queue Graphics;
            queue Present;
            queue Compute;
            queue Transfer;
        };

        explicit vulkan_context(const os::window::handle_type &WindowHandle, ::VkAllocationCallbacks *Allocator = nullptr)
            pre(WindowHandle != nullptr);
        
        ~vulkan_context();

        vulkan_context(const vulkan_context &) = delete;
        auto operator=(const vulkan_context &) -> vulkan_context& = delete;

        vulkan_context(vulkan_context &&From) noexcept;
        auto operator=(vulkan_context &&From) noexcept -> vulkan_context&;

        [[nodiscard]] auto instance() const noexcept -> ::VkInstance;
        [[nodiscard]] auto surface() const noexcept -> ::VkSurfaceKHR;
        [[nodiscard]] auto physical_device() const noexcept -> ::VkPhysicalDevice;
        [[nodiscard]] auto logical_device() const noexcept -> ::VkDevice;
        [[nodiscard]] auto device_queues() const noexcept -> const queues&;

      private:
        auto destroy() noexcept -> void;

        ::VkAllocationCallbacks *m_Allocator = nullptr;

        ::VkInstance m_Instance = VK_NULL_HANDLE;
        ::VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        ::VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        ::VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        ::VkDevice m_LogicalDevice = VK_NULL_HANDLE; 
        queues m_Queues;
    };
} // namespace ink::gfx::vk