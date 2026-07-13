#include "vulkan_platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ink::gfx::vk
{
    auto get_platform_extensions() -> std::vector<const char*>
    {
        return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    }

    auto create_surface(::VkInstance Instance, const os::window::handle_type &WindowHandle, const ::VkAllocationCallbacks *Allocator) -> ::VkSurfaceKHR
    {
        const auto CreateInfo = ::VkWin32SurfaceCreateInfoKHR{
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext     = nullptr,
            .flags     = 0,
            .hinstance = ::GetModuleHandleW(nullptr),
            .hwnd      = static_cast<::HWND>(WindowHandle)
        };

        auto Surface = ::VkSurfaceKHR{VK_NULL_HANDLE};
        ensure_vk(::vkCreateWin32SurfaceKHR(Instance, &CreateInfo, Allocator, &Surface));

        return Surface;
    }
} // namespace ink::gfx::vk