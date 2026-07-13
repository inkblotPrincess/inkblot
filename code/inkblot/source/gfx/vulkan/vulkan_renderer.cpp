#include "vulkan_renderer.hpp"

#include <limits>

namespace ink::gfx::vk
{
    [[nodiscard]] auto create_command_pool(::VkDevice Device, std::uint32_t QueueFamilyIndex, const ::VkAllocationCallbacks *Allocator) -> ::VkCommandPool
        pre(Device != VK_NULL_HANDLE)
    {
        const auto CreateInfo = ::VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = QueueFamilyIndex
        };

        auto CommandPool = ::VkCommandPool{VK_NULL_HANDLE};
        ensure_vk(::vkCreateCommandPool(Device, &CreateInfo, Allocator, &CommandPool));

        return CommandPool;
    }

    [[nodiscard]] auto allocate_command_buffer(::VkDevice Device, ::VkCommandPool CommandPool) -> ::VkCommandBuffer
        pre(Device != VK_NULL_HANDLE)
        pre(CommandPool != VK_NULL_HANDLE)
    {
        const auto AllocateInfo = ::VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,

            .commandPool        = CommandPool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1u
        };

        auto CommandBuffer = ::VkCommandBuffer{VK_NULL_HANDLE};
        ensure_vk(::vkAllocateCommandBuffers(Device, &AllocateInfo, &CommandBuffer));

        return CommandBuffer;
    }

    [[nodiscard]] auto create_semaphore(::VkDevice Device, const ::VkAllocationCallbacks *Allocator) -> ::VkSemaphore
        pre(Device != VK_NULL_HANDLE)
    {
        const auto CreateInfo = ::VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0u
        };

        auto Semaphore = ::VkSemaphore{VK_NULL_HANDLE};
        ensure_vk(::vkCreateSemaphore(Device, &CreateInfo, Allocator, &Semaphore));

        return Semaphore;
    }

    [[nodiscard]] auto create_fence(::VkDevice Device, const ::VkAllocationCallbacks *Allocator) -> ::VkFence
        pre(Device != VK_NULL_HANDLE)
    {
        const auto CreateInfo = ::VkFenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        auto Fence = ::VkFence{VK_NULL_HANDLE};
        ensure_vk(::vkCreateFence(Device, &CreateInfo, Allocator, &Fence));

        return Fence;
    }

    [[nodiscard]] auto create_frame_resources(const vulkan_context &Context, const ::VkAllocationCallbacks *Allocator) -> frame_resources
    {
        const auto Device = Context.logical_device();
        auto Frame = frame_resources{};

        try {
            Frame.CommandPool    = create_command_pool(Device, Context.device_queues().Graphics.FamilyIndex, Allocator);
            Frame.CommandBuffer  = allocate_command_buffer(Device, Frame.CommandPool);
            Frame.ImageAvailable = create_semaphore(Device, Allocator);
            Frame.RenderFence    = create_fence(Device, Allocator);
        } catch (...) {
            if (Frame.RenderFence != VK_NULL_HANDLE) {
                ::vkDestroyFence(Device, Frame.RenderFence, Allocator);
            }

            if (Frame.ImageAvailable != VK_NULL_HANDLE) {
                ::vkDestroySemaphore(Device, Frame.ImageAvailable, Allocator);
            }

            if (Frame.CommandPool  != VK_NULL_HANDLE) {
                ::vkDestroyCommandPool(Device, Frame.CommandPool, Allocator);
            }

            throw;
        }

        return Frame;
    }

    auto transition_image(
        ::VkCommandBuffer CommandBuffer, 
        ::VkImage Image, 
        ::VkImageLayout OldLayout, 
        ::VkImageLayout NewLayout, 
        ::VkPipelineStageFlags2 SourceStage, 
        ::VkAccessFlags2 SourceAccess, 
        ::VkPipelineStageFlags2 DestinationStage, 
        ::VkAccessFlags2 DestinationAccess) noexcept -> void
        pre(CommandBuffer != VK_NULL_HANDLE)
        pre(Image != VK_NULL_HANDLE)
    {
        const auto Barrier = ::VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext         = nullptr,
            .srcStageMask  = SourceStage,
            .srcAccessMask = SourceAccess,
            .dstStageMask  = DestinationStage,
            .dstAccessMask = DestinationAccess,
            .oldLayout     = OldLayout,
            .newLayout     = NewLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = Image,
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0u,
                .levelCount     = 1u,
                .baseArrayLayer = 0u,
                .layerCount     = 1u
            }
        };

        const auto DependencyInfo = ::VkDependencyInfo{
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext                    = nullptr,
            .dependencyFlags          = 0u,
            .memoryBarrierCount       = 0u,
            .pMemoryBarriers          = nullptr,
            .bufferMemoryBarrierCount = 0u,
            .pBufferMemoryBarriers    = nullptr,
            .imageMemoryBarrierCount  = 1u,
            .pImageMemoryBarriers     = &Barrier
        };

        ::vkCmdPipelineBarrier2(CommandBuffer, &DependencyInfo);
    }

    vulkan_renderer::vulkan_renderer(const renderer::config &Config)
        : m_Allocator{nullptr}
        , m_Context{Config.WindowHandle, m_Allocator}
        , m_Swapchain{{.Width = Config.Width, .Height = Config.Height, .VSync = Config.UseVSync}, m_Context, m_Allocator}
    {
        try {
            for (auto &Frame : m_Frames) {
                Frame = create_frame_resources(m_Context, m_Allocator);
            }

            m_RenderFinished.resize(m_Swapchain.image_count());
            for (auto &Semaphore : m_RenderFinished) {
                Semaphore = create_semaphore(m_Context.logical_device(), m_Allocator);
            }
        } catch (...) {
            destroy();
            throw;
        }
    }

    vulkan_renderer::~vulkan_renderer()
    {
        destroy();
    }

    auto vulkan_renderer::begin_frame(std::uint32_t Width, std::uint32_t Height) -> void
    {
        const auto Device = m_Context.logical_device();
        auto &Frame       = m_Frames[m_CurrentFrame];

        ensure_vk(::vkWaitForFences(Device, 1u, &Frame.RenderFence, VK_TRUE, std::numeric_limits<std::uint64_t>::max()));

        if (m_SwapchainNeedsRecreation) {
            recreate_swapchain(Width, Height);
            m_SwapchainNeedsRecreation = false;
        }

        while (true) {
            const auto AcquireResult = ::vkAcquireNextImageKHR(Device, m_Swapchain.handle(), std::numeric_limits<std::uint64_t>::max(), Frame.ImageAvailable, VK_NULL_HANDLE, &m_CurrentImage);
            if (AcquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
                recreate_swapchain(Width, Height);
                continue;
            }

            if (AcquireResult == VK_SUBOPTIMAL_KHR) {
                m_SwapchainNeedsRecreation = true;
            } else {
                ensure_vk(AcquireResult);
            }

            break;
        }

        ensure_vk(::vkResetCommandPool(Device, Frame.CommandPool, 0u));

        const auto BeginInfo = ::VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
        };

        ensure_vk(::vkBeginCommandBuffer(Frame.CommandBuffer, &BeginInfo));
        m_FrameStarted = true;
    }

    auto vulkan_renderer::end_frame(const frame_context &FrameContext) -> void
    {
        const auto Device  = m_Context.logical_device();
        const auto &Queues = m_Context.device_queues();

        auto &Frame = m_Frames[m_CurrentFrame];

        const auto SwapchainImage = m_Swapchain.image(m_CurrentImage);

        const auto ClearColour = ::VkClearColorValue{
            .float32 = {FrameContext.ClearColour.X, FrameContext.ClearColour.Y, FrameContext.ClearColour.Z, FrameContext.ClearColour.W}
        };

        transition_image(
            Frame.CommandBuffer,
            SwapchainImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE,
            VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT
        );

        const auto SubresourceRange = ::VkImageSubresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0u,
            .levelCount     = 1u,
            .baseArrayLayer = 0u,
            .layerCount     = 1u
        };

        ::vkCmdClearColorImage(Frame.CommandBuffer, SwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColour, 1u, &SubresourceRange);

        transition_image(
            Frame.CommandBuffer,
            SwapchainImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE,
            VK_ACCESS_2_NONE
        );

        ensure_vk(::vkEndCommandBuffer(Frame.CommandBuffer));
        ensure_vk(::vkResetFences(Device, 1u, &Frame.RenderFence));

        const auto WaitStage  = ::VkPipelineStageFlags{VK_PIPELINE_STAGE_TRANSFER_BIT};
        const auto SubmitInfo = ::VkSubmitInfo{
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext                = nullptr,
            .waitSemaphoreCount   = 1u,
            .pWaitSemaphores      = &Frame.ImageAvailable,
            .pWaitDstStageMask    = &WaitStage,
            .commandBufferCount   = 1u,
            .pCommandBuffers      = &Frame.CommandBuffer,
            .signalSemaphoreCount = 1u,
            .pSignalSemaphores    = &m_RenderFinished[m_CurrentImage]
        };

        ensure_vk(::vkQueueSubmit(Queues.Graphics.Handle, 1u, &SubmitInfo, Frame.RenderFence));

        const auto SwapchainHandle = m_Swapchain.handle();
        const auto PresentInfo = ::VkPresentInfoKHR{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = 1u,
            .pWaitSemaphores    = &m_RenderFinished[m_CurrentImage],
            .swapchainCount     = 1u,
            .pSwapchains        = &SwapchainHandle,
            .pImageIndices      = &m_CurrentImage,
            .pResults           = nullptr
        };

        const auto PresentResult = ::vkQueuePresentKHR(Queues.Present.Handle,&PresentInfo);

        if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR || PresentResult == VK_SUBOPTIMAL_KHR) {
            m_SwapchainNeedsRecreation = true;
        } else {
            ensure_vk(PresentResult);
        }

        m_CurrentFrame = (m_CurrentFrame + 1zu) % m_Frames.size();
        m_FrameStarted = false;
    }

    auto vulkan_renderer::recreate_swapchain(std::uint32_t Width, std::uint32_t Height) -> void
    {
        log::info("Recreating swapchain ({}, {})", Width, Height);
        
        ensure_vk(::vkDeviceWaitIdle(m_Context.logical_device()));
        m_Swapchain.recreate({
            .Width  = Width,
            .Height = Height,
            .VSync  = true // @TEMP: hardcoded for now
        });

        for (auto Semaphore : m_RenderFinished) {
            ::vkDestroySemaphore(m_Context.logical_device(), Semaphore, m_Allocator);
        }

        m_RenderFinished.clear();
        m_RenderFinished.resize(m_Swapchain.image_count());

        try {
            for (auto &Semaphore : m_RenderFinished) {
                Semaphore = create_semaphore(m_Context.logical_device(), m_Allocator);
            }
        } catch (...) {
            for (auto &Semaphore : m_RenderFinished) {
                if (Semaphore != VK_NULL_HANDLE) {
                    ::vkDestroySemaphore(m_Context.logical_device(), Semaphore, m_Allocator);
                    Semaphore = VK_NULL_HANDLE;
                }
            }

            m_RenderFinished.clear();
            throw;
        }
    }

    auto vulkan_renderer::destroy() noexcept -> void
    {
        const auto Device = m_Context.logical_device();
        if (Device == VK_NULL_HANDLE) {
            return;
        }

        ::vkDeviceWaitIdle(Device);

        for (auto &Semaphore : m_RenderFinished) {
            if (Semaphore != VK_NULL_HANDLE) {
                ::vkDestroySemaphore(Device, Semaphore, m_Allocator);
                Semaphore = VK_NULL_HANDLE;
            }
        }

        m_RenderFinished.clear();

        for (auto &Frame : m_Frames) {
            if (Frame.RenderFence != VK_NULL_HANDLE) {
                ::vkDestroyFence(Device, Frame.RenderFence, m_Allocator);
                Frame.RenderFence = VK_NULL_HANDLE;
            }

            if (Frame.ImageAvailable != VK_NULL_HANDLE) {
                ::vkDestroySemaphore(Device, Frame.ImageAvailable, m_Allocator);
                Frame.ImageAvailable = VK_NULL_HANDLE;
            }

            if (Frame.CommandPool  != VK_NULL_HANDLE) {
                ::vkDestroyCommandPool(Device, Frame.CommandPool, m_Allocator);
                Frame.CommandBuffer = VK_NULL_HANDLE;
                Frame.CommandPool   = VK_NULL_HANDLE;
            }
        }
    }
} // namespace ink::gfx::vk