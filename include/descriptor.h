#pragma once
#include "core.h"
#include <vector>

class DescriptorSet {
private:
    const DescriptorPool* mPool = nullptr;
    VkDescriptorSet mSet = VK_NULL_HANDLE;

    DescriptorSet(const DescriptorPool* pool, VkDescriptorSet set) :
        mPool(pool), mSet(set) {};
    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&&) = delete;
    DescriptorSet& operator=(DescriptorSet&&) = delete;
public:
    friend class DescriptorPool;
    inline operator const VkDescriptorSet& () const { return mSet; }
    ~DescriptorSet() {};        
};

class DescriptorPool {
private:
    const LogicalDevice* mDevice = nullptr;
    VkDescriptorPool mPool = VK_NULL_HANDLE;

    DescriptorPool(
        const LogicalDevice* device, VkDescriptorPool pool);
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
    DescriptorPool(DescriptorPool&&) = delete;
    DescriptorPool& operator=(DescriptorPool&&) = delete;
public:
    inline operator const VkDescriptorPool& () const { return mPool; }

    static PDescriptorPool CreateDescriptorPool(
        const LogicalDevice* device, uint32_t layoutSize, 
        const std::vector<VkDescriptorPoolSize>& poolSizes);

    std::vector<PDescriptorSet> AllocateDescriptorSets(
        const std::vector<VkDescriptorSetLayout>& layouts);
    
    ~DescriptorPool(); 
};