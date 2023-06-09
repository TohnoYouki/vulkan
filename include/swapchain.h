#pragma once
#include "core.h"
#include "image.h"
#include <vector>
#include <vulkan/vulkan.h>

class SwapChain {
private:
    VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
    const LogicalDevice* mDevice = nullptr;
    std::vector<PImage> mImages;
    Image::ImageCreateInfo mInfo;

    SwapChain(const LogicalDevice* device,
        VkSwapchainKHR swapChain,
        const VkSwapchainCreateInfoKHR& createInfo);
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    Image::ImageCreateInfo GetImageCreateInfo(
        const VkSwapchainCreateInfoKHR& createInfo);
public:
    inline const std::vector<PImage>& GetImages() const { return mImages; }
    inline operator const VkSwapchainKHR& () const { return mSwapChain; }

    const Image::ImageCreateInfo& ImageInfo() const;

    struct SwapChainCreateParams {
        uint32_t imageCount;
        VkExtent2D extent;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR format;
        VkSurfaceTransformFlagBitsKHR transform;
        std::vector<uint32_t> queueFamilies;
    };

    static VkSwapchainCreateInfoKHR DefaultCreateInfo(
        const Surface* surface,
        const SwapChainCreateParams& params);

    static PSwapChain CreateSwapChain(
        const LogicalDevice* device,
        const Surface* surface,
        const VkSwapchainCreateInfoKHR& createInfo);

    ~SwapChain();
};