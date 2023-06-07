#pragma once
#include "core.h"

class ShaderModule {
private:
    const LogicalDevice* mDevice = nullptr;
    VkShaderModule mShaderModule = VK_NULL_HANDLE;

    ShaderModule(const LogicalDevice* device, VkShaderModule module);
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&&) = delete;
    ShaderModule& operator=(ShaderModule&&) = delete;
public:
    inline operator const VkShaderModule& () const { return mShaderModule; }

    static PShaderModule CreateShaderModule(
        const LogicalDevice* device, const char* path);

    VkPipelineShaderStageCreateInfo GetStageCreateInfo(
        VkShaderStageFlagBits stage) const;

    ~ShaderModule();
};