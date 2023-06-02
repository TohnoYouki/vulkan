#include "image.h"
#include "logicaldevice.h"

Image::Image(const LogicalDevice* device, VkImage image, const ImageCreateInfo& info) :
	mImage(image), mDevice(device), mInfo(info) {};

PImageView Image::CreateImageView(const ImageViewCreateInfo& info) const {
    VkImageViewCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .image = mImage, 
        .viewType = info.viewType,
        .format = info.format == VK_FORMAT_UNDEFINED ? mInfo.format : info.format,
        .components = info.components, 
        .subresourceRange = info.subresourceRange };

    VkImageView view;
    auto res = vkCreateImageView(*mDevice, &createInfo, nullptr, &view);
    if (res != VK_SUCCESS) { return nullptr; }
    return PImageView(new ImageView(mDevice, view));
}

Image::~Image() {}

ImageView::ImageView(const LogicalDevice* device, VkImageView view) :
    mDevice(device), mImageView(view) {}

ImageView::~ImageView() {
    if (mDevice != nullptr) {
        vkDestroyImageView(*mDevice, mImageView, nullptr);
    }
}