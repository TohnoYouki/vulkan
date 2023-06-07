#pragma once
#include "core.h"
#include <vector>

class RenderPass {
private:
    const LogicalDevice* mDevice = nullptr;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;

    RenderPass(const LogicalDevice* device, VkRenderPass renderpass);
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass(RenderPass&&) = delete;
    RenderPass& operator=(RenderPass&&) = delete;
public:
    inline operator const VkRenderPass& () const { return mRenderPass; }

    static PRenderPass CreateRenderPass(
        const LogicalDevice* device,
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkSubpassDescription>& subpasses,
        const std::vector<VkSubpassDependency>& dependencies);

    ~RenderPass();
};