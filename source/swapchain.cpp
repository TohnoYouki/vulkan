#include "surface.h"
#include "swapchain.h"
#include "logicaldevice.h"

SwapChain::SwapChain(
    const LogicalDevice* device,
    VkSwapchainKHR swapChain,
    const VkSwapchainCreateInfoKHR& createInfo)
    : mDevice(device), mSwapChain(swapChain) {
    uint32_t count;
    vkGetSwapchainImagesKHR(*mDevice, mSwapChain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(*mDevice, mSwapChain, &count, images.data());
    auto imageInfo = SwapChain::GetImageCreateInfo(createInfo);
    mInfo = imageInfo;
    for (const auto& image : images) {
        mImages.emplace_back(new Image(mDevice, image, imageInfo));
    }
}

Image::ImageCreateInfo SwapChain::GetImageCreateInfo(
    const VkSwapchainCreateInfoKHR& info) {
    Image::ImageCreateInfo result {
        .imageType = VK_IMAGE_TYPE_2D,
        .format = info.imageFormat,
        .usage = info.imageUsage,
        .extent = { info.imageExtent.width,
                    info.imageExtent.height, 1 },
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .mipLevels = 1, .arrayLayers = 1,
        .sharingMode = info.imageSharingMode };
    uint32_t count = info.queueFamilyIndexCount;
    for (int i = 0; i < count; i++) {
        result.queues.push_back(*(info.pQueueFamilyIndices + i));
    }
    return result;
}

const Image::ImageCreateInfo& SwapChain::ImageInfo() const { return mInfo; }

VkSwapchainCreateInfoKHR SwapChain::DefaultCreateInfo(
    const Surface* surface,
    const SwapChainCreateParams& params) {
    return VkSwapchainCreateInfoKHR {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr, .flags = 0,
        .surface = *surface,
        .minImageCount = params.imageCount,
        .imageFormat = params.format.format,
        .imageColorSpace = params.format.colorSpace,
        .imageExtent = params.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = params.queueFamilies.size() > 1 ?
        VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = static_cast<uint32_t>(params.queueFamilies.size()),
        .pQueueFamilyIndices = params.queueFamilies.data(),
        .preTransform = params.transform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = params.presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
}

PSwapChain SwapChain::CreateSwapChain(
    const LogicalDevice* device,
    const Surface* surface,
    const VkSwapchainCreateInfoKHR& createInfo) {
    VkSwapchainKHR swapChain;
    auto result = vkCreateSwapchainKHR(
        *device, &createInfo, nullptr, &swapChain);
    if (result != VK_SUCCESS) { return nullptr; }
    return PSwapChain(new SwapChain(device, swapChain, createInfo));
}

SwapChain::~SwapChain() {
    if (mSwapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(*mDevice, mSwapChain, nullptr);
    }
};