#include "image.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "logicaldevice.h"

FrameBuffer::FrameBuffer(
    const LogicalDevice* device, VkFramebuffer framebuffer) :
    mDevice(device), mFrameBuffer(framebuffer) {}

PFrameBuffer FrameBuffer::CreateFrameBuffer(
    RenderPass* renderpass, ImageView* view) {
    VkFramebufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .renderPass = *renderpass,
        .attachmentCount = 1,
        .pAttachments = &(view->operator const VkImageView & ()),
        .width = view->ImageViewInfo().extent.width,
        .height = view->ImageViewInfo().extent.height,
        .layers = 1
    };
    VkFramebuffer framebuffer;
    const LogicalDevice* device = view->GetDevice();
    auto result = vkCreateFramebuffer(*device, &info, nullptr, &framebuffer);
    if (result != VK_SUCCESS) { return nullptr; }
    return PFrameBuffer(new FrameBuffer(device, framebuffer));
}

FrameBuffer::~FrameBuffer() {
    if (mFrameBuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(*mDevice, mFrameBuffer, nullptr);
    }
}