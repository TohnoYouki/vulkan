#include "image.h"

Image::Image(const PLogicalDevice & device, 
             const VkImage & image, const ImageCreateInfo & info) {
    mInfo = info;
    mImage = image;
    mDevice = device;
}

PImageView Image::CreateImageView(const ImageViewCreateInfo & info) const {
    VkImageViewCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.image = mImage;
    if (info.format == VK_FORMAT_UNDEFINED) {
        createInfo.format = mInfo.format;
    } else { createInfo.format = info.format; }
    createInfo.viewType = info.viewType;
    createInfo.components = info.components;
    createInfo.subresourceRange = info.subresourceRange;
    VkImageView view;
    auto res = vkCreateImageView(*mDevice, &createInfo, nullptr, &view);
    if (res != VK_SUCCESS) { return nullptr; }
    return PImageView(new ImageView(mDevice, view));
}

ImageView::ImageView(const PLogicalDevice & device, const VkImageView & view) {
    mDevice = device;
    mImageView = view;
}

ImageView::~ImageView() {
    if (mDevice != nullptr) {
        vkDestroyImageView(*mDevice, mImageView, nullptr);
    }
}