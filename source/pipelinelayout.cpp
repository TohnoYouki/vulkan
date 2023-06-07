#include "logicaldevice.h"
#include "pipelinelayout.h"

PipelineLayout::PipelineLayout(const LogicalDevice* device, VkPipelineLayout layout) :
    mDevice(device), mLayout(layout) {}

PPipelineLayout PipelineLayout::CreatePipelineLayout(const LogicalDevice* device) {
    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .setLayoutCount = 0, .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0, .pPushConstantRanges = nullptr
    };
    auto result = vkCreatePipelineLayout(*device, &info, nullptr, &layout);
    if (result != VK_SUCCESS) { return nullptr; }
    return PPipelineLayout(new PipelineLayout(device, layout));
}

PipelineLayout::~PipelineLayout() {
    if (mLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(*mDevice, mLayout, nullptr);
    }
}