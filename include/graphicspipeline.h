#pragma once
#include "core.h"

class GraphicsPipeline {
private:
    const LogicalDevice* mDevice = nullptr;
    VkPipeline mPipeline = VK_NULL_HANDLE;

    GraphicsPipeline(const LogicalDevice* device, VkPipeline pipeline);
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    GraphicsPipeline(GraphicsPipeline&&) = delete;
    GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;
public:
    inline operator const VkPipeline& () const { return mPipeline; }

    static PGraphicsPipeline CreateGraphicsPipeline(
        const LogicalDevice* device, const VkGraphicsPipelineCreateInfo& info);

    ~GraphicsPipeline();
};