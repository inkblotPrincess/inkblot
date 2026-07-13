#pragma once

#include <inkblot/os/window.hpp>

#include "vulkan.hpp"

#include <vector>

namespace ink::gfx::vk
{
    auto get_platform_extensions() -> std::vector<const char*>;

    auto create_surface(::VkInstance Instance, const os::window::handle_type &WindowHandle, const ::VkAllocationCallbacks *Allocator) -> ::VkSurfaceKHR
        pre(Instance != VK_NULL_HANDLE)
        pre(WindowHandle != nullptr)
        post(R: R != VK_NULL_HANDLE);
} // namespace ink::gfx::vk