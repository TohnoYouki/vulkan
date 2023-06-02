#include "surface.h"
#include "instance.h"
#include "physicaldevice.h"

Surface::Surface(const Instance* instance, VkSurfaceKHR surface) :
    mInstance(instance), mSurface(surface) {}

bool Surface::PhysicalDeviceSupport(
    const PhysicalDevice* device, int family) const {
    VkBool32 result;
    vkGetPhysicalDeviceSurfaceSupportKHR(*device, family, mSurface, &result);
    return static_cast<bool>(result);
}

Surface::SwapChainSupportDetails 
Surface::GetSwapChainSupportDetails(const PhysicalDevice* device) const {
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

PSurface Surface::CreateSurface(
    const Instance* instance, void* window, SurfaceCreateFn fn) {
    VkSurfaceKHR surface;
    auto result = fn(*instance, window, &surface);
    if (result != VK_SUCCESS) { nullptr; }
    return PSurface(new Surface(instance, surface));
}

Surface::~Surface() {
    if (mSurface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(*mInstance, mSurface, nullptr);
    }
}