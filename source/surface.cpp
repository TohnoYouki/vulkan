#include "surface.h"

Surface::Surface(
    const PInstance & instance, const VkSurfaceKHR & surface) {
    mInstance = instance;
    mSurface = surface;
}

PSurface Surface::CreateSurface(
    const PInstance & instance, void * window, SurfaceCreateFn fn) {
    VkSurfaceKHR surface;
    auto result = fn(*instance, window, &surface);
    if (result != VK_SUCCESS) { return VK_NULL_HANDLE; }
    return PSurface(new Surface(instance, surface));
}

bool Surface::GetPhysicalDeviceSupport(
    const PPhysicalDevice & device, uint32_t family) const {
    VkBool32 support;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(
        *device, family, mSurface, &support) != VK_SUCCESS)
        support = false;
    return support;
}

Surface::SwapChainSupportDetails Surface::GetSwapChainSupportDetails(
    const PPhysicalDevice & device) const {
    SwapChainSupportDetails result;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        *device, mSurface, &result.capabilities);
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, mSurface, &count, nullptr);
    result.formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        *device, mSurface, &count, result.formats.data());
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, mSurface, &count, nullptr);
    result.presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *device, mSurface, &count, result.presentModes.data());
    return result;
}

Surface::~Surface() {
    if (mInstance != nullptr && mSurface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(*mInstance, mSurface, nullptr); 
    }
}