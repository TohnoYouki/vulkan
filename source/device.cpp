#include "device.h"

QueueManager::QueueManager(const std::vector<QueueCreateInfo> & infos) {
    for (uint32_t i = 0; i < infos.size(); i++) {
        auto index = infos[i].familyIndex;
        if (mFamilies.find(index) == mFamilies.end())
            mFamilies[index] = std::shared_ptr<std::vector<float>>(
                new std::vector<float>());
        auto priorities = mFamilies[index];
        auto offset = static_cast<uint32_t>(priorities->size());
        Location location{index, offset, infos[i].count};
        mLocations[infos[i].name] = location;
        priorities->insert(
            priorities->end(),
            infos[i].priorities.begin(),
            infos[i].priorities.begin() + infos[i].count);
    }
}

const std::vector<VkDeviceQueueCreateInfo> 
    QueueManager::GetQueueCreateInfo() const{
    std::vector<VkDeviceQueueCreateInfo> result;
    for (const auto & item: mFamilies) {
        VkDeviceQueueCreateInfo createInfo;        
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueCount = item.second->size();
        createInfo.queueFamilyIndex = item.first;
        createInfo.pQueuePriorities = item.second->data();
        result.push_back(createInfo);
    }
    return result;
}

QueueManager::Location QueueManager::GetQueueLocation(const char * name) const {
    auto res = mLocations.find(name);
    if (res == mLocations.end())
        return Location{0, 0, 0};
    return res->second;
}

LogicalDevice::LogicalDevice(const PPhysicalDevice & physicalDevice, 
    const VkDevice & device, const QueueManager & manager): mManager(manager) {
    mDevice = device;
    mPhysicalDevice = physicalDevice;
}

VkQueue LogicalDevice::GetSingleQueue(const char * name, uint32_t index) const {
    auto location = mManager.GetQueueLocation(name);
    auto offset = location.offset;
    auto count = location.count;
    if (index >= count) { return VK_NULL_HANDLE; }
    VkQueue queue;
    vkGetDeviceQueue(mDevice, location.family, offset + index, &queue);
    return queue;
}

QueueManager::Location 
LogicalDevice::GetQueueLocation(const char * name) const {
    return mManager.GetQueueLocation(name);
};

LogicalDevice::~LogicalDevice() {
    vkDestroyDevice(mDevice, nullptr);
}

PLogicalDevice LogicalDevice::CreateLogicalDevice(
    const PPhysicalDevice & physicalDevice,
    const std::vector<QueueCreateInfo> & queues,
    const std::vector<const char *> & extensions,
    const std::vector<const char *> & layers,
    const VkPhysicalDeviceFeatures & features) {
    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.pEnabledFeatures = &features;
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();

    QueueManager queueManager(queues);
    auto queueCreateInfos = queueManager.GetQueueCreateInfo();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    VkDevice device;
    auto res = vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
    if (res != VK_SUCCESS) { return VK_NULL_HANDLE; }
    return PLogicalDevice(new LogicalDevice(
           physicalDevice, device, queueManager));
}

PhysicalDevice::PhysicalDevice(
    const PInstance & instance, const VkPhysicalDevice & device) {
    mDevice = device;
    mInstance = instance;
    vkGetPhysicalDeviceFeatures(mDevice, &mFeatures);
    vkGetPhysicalDeviceProperties(mDevice, &mProperties);
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(mDevice, &count, nullptr);
    mQueues.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(mDevice, &count, mQueues.data());
    vkEnumerateDeviceExtensionProperties(mDevice, nullptr, &count, nullptr);
    mExtensions.resize(count);
    vkEnumerateDeviceExtensionProperties(
        mDevice, nullptr, &count, mExtensions.data());
}

std::vector<PPhysicalDevice>
    PhysicalDevice::CreatePhsicalDevices(const PInstance & instance) {
    std::vector<PPhysicalDevice> result;
    auto devices = instance->PhysicDevices();
    for (const auto & device : devices) {
        result.push_back(
            PPhysicalDevice(new PhysicalDevice(instance, device)));
    }
    return result;
}

bool PhysicalDevice::CheckExtensionsSupport(
    const std::vector<const char *> names) const {
    std::set<std::string> required(names.begin(), names.end());
    for (const auto & extension : mExtensions) {
        required.erase(extension.extensionName);
    }
    return required.empty();
}