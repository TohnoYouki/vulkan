#include "renderpass.h"
#include "logicaldevice.h"

RenderPass::RenderPass(const LogicalDevice* device, VkRenderPass renderpass) :
    mDevice(device), mRenderPass(renderpass) {};

PRenderPass RenderPass::CreateRenderPass(
    const LogicalDevice* device,
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkSubpassDescription>& subpasses,
    const std::vector<VkSubpassDependency>& dependencies) {
    VkRenderPassCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = static_cast<uint32_t>(subpasses.size()),
        .pSubpasses = subpasses.data(),
        .dependencyCount = static_cast<uint32_t>(dependencies.size()), 
        .pDependencies = dependencies.data()
    };
    VkRenderPass renderpass;
    auto result = vkCreateRenderPass(*device, &info, nullptr, &renderpass);
    if (result != VK_SUCCESS) { return nullptr; }
    return PRenderPass(new RenderPass(device, renderpass));
};

RenderPass::~RenderPass() {
    if (mRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(*mDevice, mRenderPass, nullptr);
    }
};