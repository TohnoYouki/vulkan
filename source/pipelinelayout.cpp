#include "logicaldevice.h"
#include "pipelinelayout.h"

DescriptorSetLayout::DescriptorSetLayout(
    const LogicalDevice* device, VkDescriptorSetLayout layout) :
    mDevice(device), mLayout(layout) {}

PDescriptorSetLayout DescriptorSetLayout::CreateDescriptorSetLayout(
    const LogicalDevice* device, 
    const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayoutCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    VkDescriptorSetLayout layout;
    auto result = vkCreateDescriptorSetLayout(*device, &info, nullptr, &layout);
    if (result != VK_SUCCESS) { return nullptr; }
    return PDescriptorSetLayout(new DescriptorSetLayout(device, layout));
}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (mLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(*mDevice, mLayout, nullptr);
    }
}

PipelineLayout::PipelineLayout(const LogicalDevice* device, VkPipelineLayout layout) :
    mDevice(device), mLayout(layout) {}

PPipelineLayout PipelineLayout::CreatePipelineLayout(
    const LogicalDevice* device, 
    const std::vector<VkDescriptorSetLayout>& setLayouts) {
    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(setLayouts.size()), 
        .pSetLayouts = setLayouts.data(),
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