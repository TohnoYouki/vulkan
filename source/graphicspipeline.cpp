#include "logicaldevice.h"
#include "graphicspipeline.h"

GraphicsPipeline::GraphicsPipeline(
    const LogicalDevice* device, VkPipeline pipeline) :
    mDevice(device), mPipeline(pipeline) {}

PGraphicsPipeline GraphicsPipeline::CreateGraphicsPipeline(
    const LogicalDevice* device, const VkGraphicsPipelineCreateInfo& info) {
    VkPipeline pipeline;
    auto result = vkCreateGraphicsPipelines(
        *device, nullptr, 1, &info, nullptr, &pipeline);
    if (result != VK_SUCCESS) { return nullptr; }
    return PGraphicsPipeline(new GraphicsPipeline(device, pipeline));
}

GraphicsPipeline::~GraphicsPipeline() {
    if (mPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(*mDevice, mPipeline, nullptr);
    }
}