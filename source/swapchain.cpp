#include "swapchain.h"

SwapChain::SwapChain(
    const PLogicalDevice & device, 
    const VkSwapchainKHR & swapChain,
    const VkSwapchainCreateInfoKHR & createInfo) {
    mDevice = device;
    mSwapChain = swapChain;
    uint32_t count;
    vkGetSwapchainImagesKHR(*mDevice, mSwapChain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(*mDevice, mSwapChain, &count, images.data());
    auto imageInfo = SwapChain::GetImageCreateInfo(createInfo);
    for (const auto & image : images) {
        mImages.emplace_back(Image(mDevice, image, imageInfo));
    }
}

Image::ImageCreateInfo SwapChain::GetImageCreateInfo(
    const VkSwapchainCreateInfoKHR & createInfo) {
    Image::ImageCreateInfo result;
    result.imageType = VK_IMAGE_TYPE_2D;
    result.usage = createInfo.imageUsage;
    result.tiling = VK_IMAGE_TILING_OPTIMAL;
    result.sharingMode = createInfo.imageSharingMode;
    result.samples = VK_SAMPLE_COUNT_1_BIT;
    uint32_t count = createInfo.queueFamilyIndexCount;
    for (int i = 0; i < count; i++) {
        result.queues.push_back(*(createInfo.pQueueFamilyIndices + i));
    }
    result.mipLevels = 1;
    result.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    result.format = createInfo.imageFormat;
    result.extent.width = createInfo.imageExtent.width;
    result.extent.height = createInfo.imageExtent.height;
    result.extent.depth = 1;
    result.arrayLayers = 1;
    return result;
}

VkSwapchainCreateInfoKHR SwapChain::DefaultCreateInfo( 
    const PSurface & surface,
    const CreateParams & params) {
    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = *surface;
    createInfo.flags = 0;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (params.queueFamilies.size() > 1) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = params.queueFamilies.size();
        createInfo.pQueueFamilyIndices = params.queueFamilies.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.minImageCount = params.imageCount;
    createInfo.imageFormat = params.format.format;
    createInfo.imageColorSpace = params.format.colorSpace;
    createInfo.imageExtent = params.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = params.transform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = params.presentMode;
    createInfo.clipped = VK_TRUE;
    return createInfo;
}

PSwapChain SwapChain::CreateSwapChain(
    const PLogicalDevice & device, 
    const PSurface & surface,
    const VkSwapchainCreateInfoKHR & createInfo) {
    VkSwapchainKHR swapChain;
    auto result = vkCreateSwapchainKHR(
        *device, &createInfo, nullptr, &swapChain);
    if (result != VK_SUCCESS) { return VK_NULL_HANDLE; }
    return PSwapChain(new SwapChain(device, swapChain, createInfo));
}

SwapChain::~SwapChain() {
    if (mDevice != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(*mDevice, mSwapChain, nullptr);
    }
};