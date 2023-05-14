#pragma once
#include "core.h"
#include <vector>
#include "device.h"
#include <vulkan/vulkan.h>

class ImageView {
private:
    PLogicalDevice mDevice = nullptr;
    VkImageView mImageView;
    friend class Image;
    
    ImageView(const PLogicalDevice & device, const VkImageView & view);
    ImageView(const ImageView &) = delete;
    ImageView & operator=(const ImageView &) = delete;
    ImageView(ImageView &&) = delete;
    ImageView & operator=(ImageView &&) = delete;
public: 
    ~ImageView();
};

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

    PImageView CreateImageView(const ImageViewCreateInfo & info) const;
private:
    VkImage mImage;
    PLogicalDevice mDevice;
    ImageCreateInfo mInfo;

    Image(const PLogicalDevice & device, 
          const VkImage & image, const ImageCreateInfo & info);
};