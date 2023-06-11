#pragma once
#include "core.h"
#include <vector>

class DescriptorSetLayout {
private:
    const LogicalDevice* mDevice = nullptr;
    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;

    DescriptorSetLayout(
        const LogicalDevice* device, VkDescriptorSetLayout layout);
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;
public:
    inline operator const VkDescriptorSetLayout& () const { return mLayout; }

    static PDescriptorSetLayout CreateDescriptorSetLayout(
        const LogicalDevice* device, 
        const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    
    ~DescriptorSetLayout();
};

class PipelineLayout {
private:
    const LogicalDevice* mDevice = nullptr;
    VkPipelineLayout mLayout = VK_NULL_HANDLE;

    PipelineLayout(const LogicalDevice* device, VkPipelineLayout layout);
    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout& operator=(const PipelineLayout&) = delete;
    PipelineLayout(PipelineLayout&&) = delete;
    PipelineLayout& operator=(PipelineLayout&&) = delete;
public:
    inline operator const VkPipelineLayout& () const { return mLayout; }

    static PPipelineLayout CreatePipelineLayout(
        const LogicalDevice* device, 
        const std::vector<VkDescriptorSetLayout>& setLayouts);

    ~PipelineLayout();
};