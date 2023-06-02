#pragma once
#include "core.h"
#include <vector>

class Surface {
private:
    const Instance* mInstance = nullptr;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    Surface(const Instance* instance, VkSurfaceKHR surface);
    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(Surface&&) = delete;
public:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
      
    inline operator const VkSurfaceKHR& () const { return mSurface; }

    bool PhysicalDeviceSupport(const PhysicalDevice* device, int family) const;
    SwapChainSupportDetails GetSwapChainSupportDetails(const PhysicalDevice* device) const;

    static PSurface CreateSurface(
        const Instance* instance, void* window, SurfaceCreateFn fn);

    ~Surface();
};