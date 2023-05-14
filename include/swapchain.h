#pragma once
#include <vector>
#include "image.h"
#include "device.h"
#include "surface.h"
#include <vulkan/vulkan.h>

class SwapChain {
private:
    VkSwapchainKHR mSwapChain;
    std::vector<Image> mImages;
    PLogicalDevice mDevice = VK_NULL_HANDLE;
    
    SwapChain(const PLogicalDevice & device, 
              const VkSwapchainKHR & swapChain,
              const VkSwapchainCreateInfoKHR & createInfo);
    SwapChain(const SwapChain &) = delete;
    SwapChain & operator=(const SwapChain &) = delete;
    SwapChain(SwapChain &&) = delete;
    SwapChain & operator=(SwapChain &&) = delete;

    static Image::ImageCreateInfo GetImageCreateInfo(
        const VkSwapchainCreateInfoKHR & createInfo);

public:
    inline const std::vector<Image> & GetImages() const { 
        return mImages; }

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