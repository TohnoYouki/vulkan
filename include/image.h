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
        VkExtent2D extent;
        VkImageViewType viewType;
        VkFormat format;
        VkComponentMapping components;
        VkImageSubresourceRange subresourceRange;
    };
    friend class SwapChain;
private:
    VkImage mImage = VK_NULL_HANDLE;
    const LogicalDevice* mDevice = nullptr;
    ImageCreateInfo mInfo;

    Image(const LogicalDevice* device, VkImage image, const ImageCreateInfo& info);
public:
    inline operator const VkImage& () const { return mImage; }
    PImageView CreateImageView(const ImageViewCreateInfo& info) const;

    ~Image();
};

class ImageView {
private:
    const LogicalDevice* mDevice = nullptr;
    VkImageView mImageView;
    Image::ImageViewCreateInfo mInfo;
    friend class Image;

    ImageView(const LogicalDevice* device, VkImageView view,
        const Image::ImageViewCreateInfo& info);
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;
    ImageView(ImageView&&) = delete;
    ImageView& operator=(ImageView&&) = delete;
public:
    inline operator const VkImageView& () const { return mImageView; }

    const LogicalDevice* GetDevice() const;
    const Image::ImageViewCreateInfo& ImageViewInfo() const;

    ~ImageView();
};