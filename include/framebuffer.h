#pragma once
#include "core.h"

class FrameBuffer {
private:
    const LogicalDevice * mDevice = nullptr;
    VkFramebuffer mFrameBuffer = VK_NULL_HANDLE;

    FrameBuffer(const LogicalDevice* device, VkFramebuffer framebuffer);
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&) = delete;
    FrameBuffer& operator=(FrameBuffer&&) = delete;
public:
    inline operator const VkFramebuffer& () const { return mFrameBuffer; }

    static PFrameBuffer CreateFrameBuffer(
        RenderPass * renderpass, ImageView * view);

    ~FrameBuffer();
};