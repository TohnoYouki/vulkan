#pragma once
#include <vector>
#include "device.h"
#include "surface.h"

class SwapChain {
private:
    VkExtent2D mImageExtent;
    VkFormat mImageFormat;

    VkSwapchainKHR mSwapChain;
    std::vector<VkImage> mImages;
    PLogicalDevice mDevice = VK_NULL_HANDLE;
    
    SwapChain(const PLogicalDevice & device, 
              const VkSwapchainKHR & swapChain,
              const VkSwapchainCreateInfoKHR & createInfo);
    SwapChain(const SwapChain &) = delete;
    SwapChain & operator=(const SwapChain &) = delete;
    SwapChain(SwapChain &&) = delete;
    SwapChain & operator=(SwapChain &&) = delete;

public:
    struct CreateParams {
        uint32_t imageCount;
        VkExtent2D extent;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR format;
        VkSurfaceTransformFlagBitsKHR transform;
        std::vector<uint32_t> queueFamilies;
    };

    static VkSwapchainCreateInfoKHR DefaultCreateInfo(
        const PSurface & surface, 
        const CreateParams & params);

    static PSwapChain CreateSwapChain(
        const PLogicalDevice & device, 
        const PSurface & surface,
        const VkSwapchainCreateInfoKHR & createInfo);
    
    ~SwapChain();
};