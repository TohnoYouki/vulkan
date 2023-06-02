#pragma once
#include "core.h"
#include <vector>
#include <vulkan/vulkan.h>

class Image {
public:
    struct ImageCreateInfo {
        VkImageType imageType;
        VkFormat format;
        VkImageUsageFlags usage;
        VkExtent3D extent;
        VkImageTiling tiling;
        VkSampleCountFlagBits samples;
        VkImageLayout initialLayout;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        VkSharingMode sharingMode;
        std::vector<uint32_t> queues;
    };
    struct ImageViewCreateInfo {
        VkImageViewType viewType;
        VkFormat format;
        VkComponentMapping components;
        VkImageSubresourceRange subresourceRange;
    };
    friend class SwapChain;
private:
    VkImage mImage = VK_NULL_HANDLE;
    const LogicalDevice * mDevice = nullptr;
    ImageCreateInfo mInfo;

    Image(const LogicalDevice * device, VkImage image, const ImageCreateInfo& info);
public:
    PImageView CreateImageView(const ImageViewCreateInfo& info) const;

    ~Image();
};

class ImageView {
private:
    const LogicalDevice * mDevice = nullptr;
    VkImageView mImageView;
    friend class Image;

    ImageView(const LogicalDevice * device, VkImageView view);
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;
    ImageView(ImageView&&) = delete;
    ImageView& operator=(ImageView&&) = delete;
public:
    ~ImageView();
};