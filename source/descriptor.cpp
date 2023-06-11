#include "descriptor.h"
#include "logicaldevice.h"

DescriptorPool::DescriptorPool(
    const LogicalDevice* device, VkDescriptorPool pool) :
    mDevice(device), mPool(pool) {}

PDescriptorPool DescriptorPool::CreateDescriptorPool(
    const LogicalDevice* device, uint32_t layoutSize, 
    const std::vector<VkDescriptorPoolSize>& poolSizes) {

    VkDescriptorPoolCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
        .maxSets = layoutSize,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    VkDescriptorPool pool;
    auto result = vkCreateDescriptorPool(*device, &info, nullptr, &pool);
    if (result != VK_SUCCESS) { return nullptr; }
    return PDescriptorPool(new DescriptorPool(device, pool));
}

std::vector<PDescriptorSet> DescriptorPool::AllocateDescriptorSets(
    const std::vector<VkDescriptorSetLayout>& layouts) {
    uint32_t count = static_cast<uint32_t>(layouts.size());
    VkDescriptorSetAllocateInfo info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = mPool,
        .descriptorSetCount = count,
        .pSetLayouts = layouts.data()  
    };
    std::vector<VkDescriptorSet> sets(count);
    auto result = vkAllocateDescriptorSets(*mDevice, &info, sets.data());
    std::vector<PDescriptorSet> psets;
    if (result != VK_SUCCESS) { return psets; }
    for (int i = 0; i < count; i++) {
        psets.emplace_back(new DescriptorSet(this, sets[i]));
    }
    return psets;
}

DescriptorPool::~DescriptorPool() {
    if (mPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(*mDevice, mPool, nullptr);
    }
}