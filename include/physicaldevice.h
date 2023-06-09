#pragma once
#include "core.h"
#include <vector>

class PhysicalDevice {
private:
    const Instance* mInstance = nullptr;
    VkPhysicalDevice mDevice = VK_NULL_HANDLE;

    PhysicalDevice(const Instance* instance, VkPhysicalDevice device);
    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;
    PhysicalDevice(PhysicalDevice&&) = delete;
    PhysicalDevice& operator=(PhysicalDevice&&) = delete;
public:
    friend class Instance;

    VkPhysicalDeviceProperties mProperties;
    VkPhysicalDeviceFeatures mFeatures;
    std::vector<VkQueueFamilyProperties> mQueuesProperties;
    std::vector<VkExtensionProperties> mExtensions;

    inline operator const VkPhysicalDevice& () const { return mDevice; }

    VkPhysicalDeviceMemoryProperties GetMemoryProperties() const;

    bool CheckExtensionsSupport(
        const std::vector<const char*>& extensions) const;
};