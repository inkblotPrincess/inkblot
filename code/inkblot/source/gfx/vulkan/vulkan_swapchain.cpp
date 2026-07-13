#include "vulkan_swapchain.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <ranges>
#include <span>
#include <vector>
#include <utility>

namespace ink::gfx::vk
{
    struct swapchain_support
    {
        ::VkSurfaceCapabilitiesKHR Capabilities;

        std::vector<::VkSurfaceFormatKHR> Formats;
        std::vector<::VkPresentModeKHR>   PresentModes;
    };

    [[nodiscard]] auto get_swapchain_support(::VkPhysicalDevice Device, ::VkSurfaceKHR Surface) -> swapchain_support
        pre(Device != VK_NULL_HANDLE)
        pre(Surface != VK_NULL_HANDLE)
    {
        auto Support = swapchain_support{};
        ensure_vk(::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Support.Capabilities));

        auto FormatCount = std::uint32_t{};
        ensure_vk(::vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr));

        Support.Formats.resize(FormatCount);
        ensure_vk(::vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Support.Formats.data()));

        auto PresentModeCount = std::uint32_t{};
        ensure_vk(::vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, nullptr));

        Support.PresentModes.resize(PresentModeCount);
        ensure_vk(::vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, Support.PresentModes.data()));

        return Support;
    }

    [[nodiscard]] auto choose_surface_format(std::span<const ::VkSurfaceFormatKHR> Formats) noexcept -> ::VkSurfaceFormatKHR
        pre(!Formats.empty())
    {
        const auto ChosenFormat = std::ranges::find_if(Formats, [](const auto &Format) noexcept {
            return Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        });

        if (ChosenFormat != Formats.end()) {
            return *ChosenFormat;
        }

        return Formats.front();
    }

    [[nodiscard]] auto choose_present_mode(std::span<const ::VkPresentModeKHR> PresentModes, bool VSync) noexcept -> ::VkPresentModeKHR
        pre(!PresentModes.empty())
    {
        if (VSync) {
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        if (std::ranges::contains(PresentModes, VK_PRESENT_MODE_MAILBOX_KHR)) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        if (std::ranges::contains(PresentModes, VK_PRESENT_MODE_IMMEDIATE_KHR)) {
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    [[nodiscard]] auto choose_extent(const ::VkSurfaceCapabilitiesKHR &Capabilities, std::uint32_t Width, std::uint32_t Height) noexcept -> ::VkExtent2D
    {
        if (Capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            return Capabilities.currentExtent;
        }

        return {
            .width  = std::clamp(Width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width),
            .height = std::clamp(Height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height),
        };
    }

    [[nodiscard]] auto choose_image_count(const ::VkSurfaceCapabilitiesKHR &Capabilities) noexcept -> std::uint32_t
    {
        auto ImageCount = Capabilities.minImageCount + 1u;
        if (Capabilities.maxImageCount != 0u && ImageCount > Capabilities.maxImageCount) {
            ImageCount = Capabilities.maxImageCount;
        }

        return ImageCount;
    }

    [[nodiscard]] auto create_image_view(::VkDevice LogicalDevice, ::VkImage Image, ::VkFormat Format, const ::VkAllocationCallbacks *Allocator) -> ::VkImageView
        pre(LogicalDevice != VK_NULL_HANDLE)
        pre(Image != VK_NULL_HANDLE)
    {
        const auto CreateInfo = ::VkImageViewCreateInfo{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0u,
            .image    = Image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = Format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0u,
                .levelCount     = 1u,
                .baseArrayLayer = 0u,
                .layerCount     = 1u
            }
        };

        auto ImageView = ::VkImageView(VK_NULL_HANDLE);
        ensure_vk(::vkCreateImageView(LogicalDevice, &CreateInfo, Allocator, &ImageView));

        return ImageView;
    }

    vulkan_swapchain::vulkan_swapchain(const vulkan_swapchain::config &Config, const vulkan_context &Context, const ::VkAllocationCallbacks *Allocator)
        : m_Allocator{Allocator}
        , m_Context{&Context}
    {
        recreate(Config);
    }

    vulkan_swapchain::~vulkan_swapchain()
    {
        destroy();
    }

    vulkan_swapchain::vulkan_swapchain(vulkan_swapchain &&From) noexcept
        : m_Allocator{std::exchange(From.m_Allocator, nullptr)}
        , m_Context{std::exchange(From.m_Context, nullptr)}
        , m_Swapchain{std::exchange(From.m_Swapchain, VK_NULL_HANDLE)}
        , m_Images{std::move(From.m_Images)}
        , m_ImageViews{std::move(From.m_ImageViews)}
        , m_SurfaceFormat{From.m_SurfaceFormat}
        , m_PresentMode{From.m_PresentMode}
        , m_Extent{From.m_Extent}
    {
    }

    auto vulkan_swapchain::operator=(vulkan_swapchain &&From) noexcept -> vulkan_swapchain&
    {
        if (this != &From) {
            destroy();

            m_Allocator = std::exchange(From.m_Allocator, nullptr);
            m_Context   = std::exchange(From.m_Context, nullptr);

            m_Swapchain = std::exchange(From.m_Swapchain, VK_NULL_HANDLE);

            m_Images     = std::move(From.m_Images);
            m_ImageViews = std::move(From.m_ImageViews);
            
            m_SurfaceFormat = From.m_SurfaceFormat;
            m_PresentMode   = From.m_PresentMode;
            m_Extent        = From.m_Extent;
        }

        return *this;
    }

    auto vulkan_swapchain::recreate(const vulkan_swapchain::config &Config) -> void
    {
        const auto PhysicalDevice = m_Context->physical_device();
        const auto LogicalDevice  = m_Context->logical_device();
        const auto Surface        = m_Context->surface();
        const auto &Queues        = m_Context->device_queues();

        const auto Support    = get_swapchain_support(PhysicalDevice, Surface);
        const auto Format     = choose_surface_format(Support.Formats);
        const auto Mode       = choose_present_mode(Support.PresentModes, Config.VSync);
        const auto Extent     = choose_extent(Support.Capabilities, Config.Width, Config.Height);
        const auto ImageCount = choose_image_count(Support.Capabilities);

        contract_assert(Extent.width > 0u);
        contract_assert(Extent.height > 0u);

        const auto QueueFamilyIndices = std::array{Queues.Graphics.FamilyIndex, Queues.Present.FamilyIndex};
        const auto SeparateFamilies   = QueueFamilyIndices[0] != QueueFamilyIndices[1];

        const auto CreateInfo = ::VkSwapchainCreateInfoKHR{
            .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext            = nullptr,
            .flags            = 0u,
            .surface          = Surface,
            .minImageCount    = ImageCount,
            .imageFormat      = Format.format,
            .imageColorSpace  = Format.colorSpace,
            .imageExtent      = Extent,
            .imageArrayLayers = 1u,
            .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

            .imageSharingMode      = SeparateFamilies ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = SeparateFamilies ? static_cast<std::uint32_t>(QueueFamilyIndices.size()) : 0u,
            .pQueueFamilyIndices   = SeparateFamilies ? QueueFamilyIndices.data() : nullptr,
            
            .preTransform   = Support.Capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode    = Mode,
            .clipped        = VK_TRUE,

            .oldSwapchain = m_Swapchain
        };

        auto NewSwapchain = ::VkSwapchainKHR(VK_NULL_HANDLE);
        ensure_vk(::vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, m_Allocator, &NewSwapchain));

        auto NewImages     = std::vector<::VkImage>{};
        auto NewImageViews = std::vector<::VkImageView>{};

        try {
            auto NewImageCount = std::uint32_t{};
            ensure_vk(::vkGetSwapchainImagesKHR(LogicalDevice, NewSwapchain, &NewImageCount, nullptr));

            NewImages.resize(NewImageCount);
            ensure_vk(::vkGetSwapchainImagesKHR(LogicalDevice, NewSwapchain, &NewImageCount, NewImages.data()));
            NewImages.resize(NewImageCount);

            NewImageViews.reserve(NewImages.size());
            for (auto Image : NewImages) {
                NewImageViews.emplace_back(create_image_view(LogicalDevice, Image, Format.format, m_Allocator));
            }
        } catch (...) {
            for (auto &ImageView : NewImageViews) {
                ::vkDestroyImageView(LogicalDevice, ImageView, m_Allocator);
            }

            ::vkDestroySwapchainKHR(LogicalDevice, NewSwapchain, m_Allocator);
            throw;
        }

        for (auto &ImageView : m_ImageViews) {
            ::vkDestroyImageView(LogicalDevice, ImageView, m_Allocator);
        }

        if (m_Swapchain != VK_NULL_HANDLE) {
            ::vkDestroySwapchainKHR(LogicalDevice, m_Swapchain, m_Allocator);
        }

        m_Swapchain     = NewSwapchain;
        m_Images        = std::move(NewImages);
        m_ImageViews    = std::move(NewImageViews);
        m_SurfaceFormat = Format;
        m_PresentMode   = Mode;
        m_Extent        = Extent;
    }

    auto vulkan_swapchain::destroy() noexcept -> void
    {
        if (m_Context == nullptr) {
            return;
        }

        const auto Device = m_Context->logical_device();
        for (auto &ImageView : m_ImageViews) {
            ::vkDestroyImageView(Device, ImageView, m_Allocator);
        }

        m_ImageViews.clear();
        m_Images.clear();

        if (m_Swapchain != VK_NULL_HANDLE) {
            ::vkDestroySwapchainKHR(Device, m_Swapchain, m_Allocator);
            m_Swapchain = VK_NULL_HANDLE;
        }
    }
} // namespace ink::gfx::vk