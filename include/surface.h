#pragma once
#include "core.h"
#include "device.h"
#include "instance.h"

class Surface {
private:
    PInstance mInstance = nullptr;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    
    Surface(const PInstance & instance, const VkSurfaceKHR & surface);
    Surface(const Surface &) = delete;
    Surface & operator=(const Surface &) = delete;
    Surface(Surface &&) = delete;
    Surface & operator=(Surface &&) = delete;

public:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    inline operator const VkSurfaceKHR & () const { return mSurface; }

    static PSurface CreateSurface(
        const PInstance & instance, void * window, SurfaceCreateFn fn);

    bool GetPhysicalDeviceSupport(
        const PPhysicalDevice & device, uint32_t family) const;

    SwapChainSupportDetails GetSwapChainSupportDetails(
        const PPhysicalDevice & device) const;

    ~Surface();
};