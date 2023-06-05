#include "image.h"
#include "logicaldevice.h"

Image::Image(const LogicalDevice* device, VkImage image, const ImageCreateInfo& info) :
	mImage(image), mDevice(device), mInfo(info) {};

PImageView Image::CreateImageView(const ImageViewCreateInfo& info) const {
    VkFormat format = info.format == VK_FORMAT_UNDEFINED ? mInfo.format : info.format;
    VkImageViewCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .image = mImage, 
        .viewType = info.viewType,
        .format = format,
        .components = info.components, 
        .subresourceRange = info.subresourceRange };

    VkImageView view;
    auto res = vkCreateImageView(*mDevice, &createInfo, nullptr, &view);
    if (res != VK_SUCCESS) { return nullptr; }
    auto pointer = PImageView(new ImageView(mDevice, view, info));
    pointer->mInfo.format = format;
    return pointer;
}

Image::~Image() {}

ImageView::ImageView(const LogicalDevice* device, VkImageView view,
    const Image::ImageViewCreateInfo& info) :
    mDevice(device), mImageView(view), mInfo(info) {}

const Image::ImageViewCreateInfo& ImageView::ImageViewInfo() const { 
    return mInfo; 
}

ImageView::~ImageView() {
    if (mDevice != nullptr) {
        vkDestroyImageView(*mDevice, mImageView, nullptr);
    }
}