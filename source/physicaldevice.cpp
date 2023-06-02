#include <set>
#include <string>
#include "physicaldevice.h"

PhysicalDevice::PhysicalDevice(
    const Instance* instance, VkPhysicalDevice device) :
    mInstance(instance), mDevice(device) {
    uint32_t count;
    vkGetPhysicalDeviceProperties(mDevice, &mProperties);
    vkGetPhysicalDeviceFeatures(mDevice, &mFeatures);
    vkGetPhysicalDeviceQueueFamilyProperties(mDevice, &count, nullptr);
    mQueuesProperties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        mDevice, &count, mQueuesProperties.data());
    vkEnumerateDeviceExtensionProperties(mDevice, nullptr, &count, nullptr);
    mExtensions.resize(count);
    vkEnumerateDeviceExtensionProperties(
        mDevice, nullptr, &count, mExtensions.data());
};

bool PhysicalDevice::CheckExtensionsSupport(
    const std::vector<const char*>& extensions) const {
    std::set<std::string> required(
        extensions.begin(), extensions.end());
    for (const auto& extension : mExtensions) {
        required.erase(extension.extensionName);
    }
    return required.empty();
}