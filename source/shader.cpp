#include <vector>
#include <fstream>
#include "shader.h"
#include "logicaldevice.h"

static std::vector<char> ReadFile(const char* path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) { return std::vector<char>(); }
    size_t size = (size_t)file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

ShaderModule::ShaderModule(
    const LogicalDevice* device, VkShaderModule module) :
    mDevice(device), mShaderModule(module) {}

PShaderModule ShaderModule::CreateShaderModule(
    const LogicalDevice* device, const char* path) {
    std::vector<char> code = ReadFile(path);
    if (code.size() == 0) { return nullptr; }
    VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };
    VkShaderModule module;
    auto result = vkCreateShaderModule(*device, &info, nullptr, &module);
    if (result != VK_SUCCESS) { return nullptr; }
    return PShaderModule(new ShaderModule(device, module));
}

VkPipelineShaderStageCreateInfo ShaderModule::GetStageCreateInfo(
    VkShaderStageFlagBits stage) const {
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .stage = stage, .module = mShaderModule, .pName = "main",
        .pSpecializationInfo = nullptr
    };
}

ShaderModule::~ShaderModule() {
    if (mShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(*mDevice, mShaderModule, nullptr);
    }
}