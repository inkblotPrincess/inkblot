#include <inkblot/basic/logging.hpp>

#include "vulkan_context.hpp"
#include "vulkan_platform.hpp"

#include <inplace_vector>
#include <utility>

namespace ink::gfx::vk
{
    struct queue_family_indices
    {
        static constexpr auto Invalid = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t Graphics = Invalid;
        std::uint32_t Present  = Invalid;
        std::uint32_t Compute  = Invalid;
        std::uint32_t Transfer = Invalid;

        [[nodiscard]] constexpr auto complete() const noexcept -> bool
        {
            return Graphics != Invalid && Present  != Invalid && Compute  != Invalid && Transfer != Invalid;
        }
    };

    struct physical_device_selection
    {
        ::VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
        
        ::VkPhysicalDeviceProperties2 DeviceProperties;
        ::VkPhysicalDeviceFeatures2   DeviceFeatures;
        
        queue_family_indices QueueFamilies;

        std::uint64_t Score = 0u;
    };

    [[nodiscard]] auto create_instance(const ::VkAllocationCallbacks *Allocator) -> ::VkInstance
        post(R: R != VK_NULL_HANDLE)
    {
        const auto AppInfo = ::VkApplicationInfo{
            .sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "ink_vk_renderer",
            .apiVersion       = VK_API_VERSION_1_3
        };

        const auto InstanceExtensions = get_platform_extensions();

        const auto InstanceCreateInfo = ::VkInstanceCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo        = &AppInfo,
            .enabledExtensionCount   = static_cast<std::uint32_t>(InstanceExtensions.size()),
            .ppEnabledExtensionNames = InstanceExtensions.data(),
        };

        auto Instance = ::VkInstance{VK_NULL_HANDLE};
        ensure_vk(::vkCreateInstance(&InstanceCreateInfo, Allocator, &Instance));

        return Instance;
    }

    [[nodiscard]] auto select_physical_device(::VkInstance Instance, ::VkSurfaceKHR Surface) -> physical_device_selection
        pre(Instance != VK_NULL_HANDLE)
    {
        auto DeviceCount = std::uint32_t{};
        ensure_vk(::vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr));

        auto Devices = std::vector<::VkPhysicalDevice>{DeviceCount};
        ensure_vk(::vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data()));

        auto BestSelection = physical_device_selection{};

        for (const auto &Device : Devices) {
            auto Properties = ::VkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            ::vkGetPhysicalDeviceProperties2(Device, &Properties);

            auto Features = ::VkPhysicalDeviceFeatures2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            ::vkGetPhysicalDeviceFeatures2(Device, &Features);

            auto QueueFamilyCount = std::uint32_t{};
            ::vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

            auto QueueFamilyProperties = std::vector<::VkQueueFamilyProperties>{QueueFamilyCount};
            ::vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilyProperties.data());

            auto QueueFamilies = queue_family_indices{};

            // pass 1 -> prioritse dedicate queues
            for (auto QueueFamiliesIndex = 0u; QueueFamiliesIndex < QueueFamilyCount; ++QueueFamiliesIndex) {
                const auto &QueueFamily = QueueFamilyProperties[QueueFamiliesIndex];
                if (
                    QueueFamilies.Compute == queue_family_indices::Invalid && 
                    (QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u &&
                    (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u) {
                    QueueFamilies.Compute = QueueFamiliesIndex;
                }

                if (QueueFamilies.Transfer == queue_family_indices::Invalid &&
                    (QueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u &&
                    (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u &&
                    (QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0u) {
                    QueueFamilies.Transfer = QueueFamiliesIndex;
                }
            }

            // pass 2 -> fill in other queues
            for (auto QueueFamiliesIndex = 0u; QueueFamiliesIndex < QueueFamilyCount; ++QueueFamiliesIndex) {
                const auto &QueueFamily = QueueFamilyProperties[QueueFamiliesIndex];

                if (QueueFamilies.Graphics == queue_family_indices::Invalid && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
                    QueueFamilies.Graphics = QueueFamiliesIndex;
                }

                if (QueueFamilies.Compute == queue_family_indices::Invalid && (QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u) {
                    QueueFamilies.Compute = QueueFamiliesIndex;
                }

                if (QueueFamilies.Transfer == queue_family_indices::Invalid && (QueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u) {
                    QueueFamilies.Transfer = QueueFamiliesIndex;
                }

                if (QueueFamilies.Present == queue_family_indices::Invalid) {
                    auto PresentSupported = VkBool32{VK_FALSE};
                    ensure_vk(::vkGetPhysicalDeviceSurfaceSupportKHR(Device, QueueFamiliesIndex, Surface, &PresentSupported));

                    if (PresentSupported == VK_TRUE) {
                        QueueFamilies.Present = QueueFamiliesIndex;
                    }
                }

                if (QueueFamilies.complete()) {
                    continue;
                }
            }

            if (!QueueFamilies.complete()) {
                continue;
            }

            auto Score = 0u;

            switch (Properties.properties.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    Score += 10'000;
                    break;
                
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    Score += 5'000;
                    break;
                
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    Score += 1'000;
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    Score += 100;
                    break;

                default:
                    break;
            }

            if (QueueFamilies.Compute != QueueFamilies.Graphics) {
                Score += 500;
            }

            if (QueueFamilies.Transfer != QueueFamilies.Compute && QueueFamilies.Transfer != QueueFamilies.Graphics) {
                Score += 500;
            }

            if (BestSelection.PhysicalDevice == VK_NULL_HANDLE || Score > BestSelection.Score) {
                BestSelection = {
                    .PhysicalDevice   = Device,
                    .DeviceProperties = Properties,
                    .DeviceFeatures   = Features,
                    .QueueFamilies    = QueueFamilies,
                    .Score            = Score
                };
            }
        }

        ensure(BestSelection.PhysicalDevice != VK_NULL_HANDLE, "Failed to find suitable device");
        return BestSelection;
    }

    [[nodiscard]] auto create_logical_device(::VkPhysicalDevice PhysicalDevice, const queue_family_indices &QueueFamilies, const ::VkAllocationCallbacks *Allocator) -> ::VkDevice
        pre(PhysicalDevice != nullptr)
    {
        constexpr auto QueuePriority = 1.0f;
        
        auto QueueCreateInfos = std::inplace_vector<::VkDeviceQueueCreateInfo, 4>{};

        const auto AddQueueFamily = [&QueueCreateInfos, &QueuePriority](std::uint32_t FamilyIndex) {
            if (std::ranges::contains(QueueCreateInfos, FamilyIndex, &::VkDeviceQueueCreateInfo::queueFamilyIndex)) {
                return;
            }

            QueueCreateInfos.emplace_back(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0u, FamilyIndex, 1u, &QueuePriority);
        };

        AddQueueFamily(QueueFamilies.Graphics);
        AddQueueFamily(QueueFamilies.Present);
        AddQueueFamily(QueueFamilies.Compute);
        AddQueueFamily(QueueFamilies.Transfer);

        constexpr auto DeviceExtensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        const auto Features = ::VkPhysicalDeviceFeatures{};

        const auto CreateInfo = ::VkDeviceCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0u,
            .queueCreateInfoCount    = static_cast<std::uint32_t>(QueueCreateInfos.size()),
            .pQueueCreateInfos       = QueueCreateInfos.data(),
            .enabledLayerCount       = 0u,
            .ppEnabledLayerNames     = nullptr,
            .enabledExtensionCount   = static_cast<std::uint32_t>(DeviceExtensions.size()),
            .ppEnabledExtensionNames = DeviceExtensions.data(),
            .pEnabledFeatures        = &Features
        };

        auto LogicalDevice = ::VkDevice{VK_NULL_HANDLE};
        ensure_vk(::vkCreateDevice(PhysicalDevice, &CreateInfo, Allocator, &LogicalDevice));

        return LogicalDevice;
    }

    [[nodiscard]] auto create_queues(::VkDevice LogicalDevice, const queue_family_indices &QueueFamilies) -> vulkan_context::queues
        pre(LogicalDevice != VK_NULL_HANDLE)
    {
        const auto GetQueue = [&LogicalDevice](std::uint32_t FamilyIndex) {
            auto Queue = ::VkQueue{VK_NULL_HANDLE};

            ::vkGetDeviceQueue(LogicalDevice, FamilyIndex, 0u, &Queue);
            ensure(Queue != VK_NULL_HANDLE, "Failed to create device queue");

            return Queue;
        };

        return {
            .Graphics = {
                .Handle      = GetQueue(QueueFamilies.Graphics),
                .FamilyIndex = QueueFamilies.Graphics,
                .QueueIndex  = 0u
            },
            .Present = {
                .Handle      = GetQueue(QueueFamilies.Present),
                .FamilyIndex = QueueFamilies.Present,
                .QueueIndex  = 0u
            },
            .Compute = {
                .Handle      = GetQueue(QueueFamilies.Compute),
                .FamilyIndex = QueueFamilies.Compute,
                .QueueIndex  = 0u
            },
            .Transfer = {
                .Handle      = GetQueue(QueueFamilies.Transfer),
                .FamilyIndex = QueueFamilies.Transfer,
                .QueueIndex  = 0u
            }
        };
    }

    vulkan_context::vulkan_context(const os::window::handle_type &WindowHandle, ::VkAllocationCallbacks *Allocator)
        : m_Allocator{Allocator}
    {
        try {
            m_Instance = create_instance(m_Allocator);
            m_Surface  = create_surface(m_Instance, WindowHandle, m_Allocator);

            const auto SelectedPhysicalDevice = select_physical_device(m_Instance, m_Surface);
            m_PhysicalDevice = SelectedPhysicalDevice.PhysicalDevice;

            const auto &Properties = SelectedPhysicalDevice.DeviceProperties.properties;
            log::info("Selected GPU: {}", Properties.deviceName);
            log::info(" -> Driver version: {}.{}.{}", 
                VK_VERSION_MAJOR(Properties.driverVersion),
                VK_VERSION_MINOR(Properties.driverVersion),
                VK_VERSION_PATCH(Properties.driverVersion));
            log::info(" -> Vulkan API version: {}.{}.{}",
                VK_VERSION_MAJOR(Properties.apiVersion),
                VK_VERSION_MINOR(Properties.apiVersion),
                VK_VERSION_PATCH(Properties.apiVersion));
            
            m_LogicalDevice = create_logical_device(m_PhysicalDevice, SelectedPhysicalDevice.QueueFamilies, m_Allocator);
            m_Queues        = create_queues(m_LogicalDevice, SelectedPhysicalDevice.QueueFamilies);
        } catch (...) {
            destroy();
            throw;
        }
    }

    vulkan_context::~vulkan_context()
    {
        destroy();
    }

    vulkan_context::vulkan_context(vulkan_context &&From) noexcept
        : m_Allocator{std::exchange(From.m_Allocator, nullptr)}
        , m_Instance{std::exchange(From.m_Instance, VK_NULL_HANDLE)}
        , m_Surface{std::exchange(From.m_Surface, VK_NULL_HANDLE)}
        , m_PhysicalDevice{std::exchange(From.m_PhysicalDevice, VK_NULL_HANDLE)}
        , m_LogicalDevice{std::exchange(From.m_LogicalDevice, VK_NULL_HANDLE)}
    {
    }

    auto vulkan_context::operator=(vulkan_context &&From) noexcept -> vulkan_context&
    {
        if (this != &From) {
            destroy();

            m_Allocator = std::exchange(From.m_Allocator, nullptr);

            m_Instance       = std::exchange(From.m_Instance, VK_NULL_HANDLE);
            m_Surface        = std::exchange(From.m_Surface, VK_NULL_HANDLE);
            m_PhysicalDevice = std::exchange(From.m_PhysicalDevice, VK_NULL_HANDLE);
            m_LogicalDevice  = std::exchange(From.m_LogicalDevice, VK_NULL_HANDLE);
        }

        return *this;
    }

    auto vulkan_context::instance() const noexcept -> ::VkInstance
    {
        return m_Instance;
    }
    
    auto vulkan_context::surface() const noexcept -> ::VkSurfaceKHR
    {
        return m_Surface;
    }

    auto vulkan_context::physical_device() const noexcept -> ::VkPhysicalDevice
    {
        return m_PhysicalDevice;
    }

    auto vulkan_context::logical_device() const noexcept -> ::VkDevice
    {
        return m_LogicalDevice;
    }

    auto vulkan_context::queue_families() const noexcept -> const queues&
    {
        return m_Queues;
    }

    auto vulkan_context::destroy() noexcept -> void
    {
        if (m_LogicalDevice != VK_NULL_HANDLE) {
            ::vkDestroyDevice(m_LogicalDevice, m_Allocator);
            m_LogicalDevice = VK_NULL_HANDLE;
        }

        m_PhysicalDevice = VK_NULL_HANDLE;
        m_Queues = {};

        if (m_Surface != VK_NULL_HANDLE) {
            ::vkDestroySurfaceKHR(m_Instance, m_Surface, m_Allocator);
            m_Surface = VK_NULL_HANDLE;
        }

        if (m_Instance != VK_NULL_HANDLE) {
            ::vkDestroyInstance(m_Instance, m_Allocator);
            m_Instance = VK_NULL_HANDLE;
        }
    }
} // namespace ink::gfx::vk