#include "instance.h"
#include "physicaldevice.h"

VkApplicationInfo Instance::DefaultApplicationCreateInfo(const char* name) {
    return VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = name,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0 };
}

Instance::Instance(VkInstance instance, const std::vector<const char*>& layers) {
    mInstance = instance;
    mSupportLayers = layers;
}

std::vector<const PhysicalDevice*> Instance::EnumeratePhysicalDevices() {
    uint32_t count;
    vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
    std::vector<VkPhysicalDevice> vkdevices(count);
    vkEnumeratePhysicalDevices(mInstance, &count, vkdevices.data());
    std::vector<PPhysicalDevice> devices;
    std::vector<const PhysicalDevice*> result;
    for (const auto& vkdevice : vkdevices) {
        auto findRes = std::find_if(
            mPhysicalDevices.begin(), mPhysicalDevices.end(),
            [&vkdevice](const PPhysicalDevice& x) {
                return x->mDevice == vkdevice; });
        if (findRes == mPhysicalDevices.end()) {
            devices.emplace_back(new PhysicalDevice(this, vkdevice));
        }
        else {
            devices.push_back(std::move(*findRes));
        }
        result.push_back(devices.back().get());
    }
    mPhysicalDevices = std::move(devices);
    return result;
}

PInstance Instance::CreateInstance(
    const VkApplicationInfo& applicationInfo,
    std::vector<const char*>& extensions,
    DebugModes* debugUtils) {

    std::vector<const char*> layers;
    if (debugUtils != nullptr) {
        layers = debugUtils->GetRequiredLayers();
        auto debugExtensions = debugUtils->GetRequiredExtensions();
        extensions.insert(extensions.end(),
            debugExtensions.begin(), debugExtensions.end());
    }

    if (!Environment::CheckAvaliableLayers(layers) ||
        !Environment::CheckAvailableExtensions(extensions)) {
        return nullptr;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data() };

    if (debugUtils != nullptr) {
        debugCreateInfo = debugUtils->DebugMessengerCreateInfo();
        createInfo.pNext = &debugCreateInfo;
    }

    VkInstance instance;
    auto result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) { return nullptr; }
    return PInstance(new Instance(instance, layers));
}

Instance::~Instance() {
    if (mInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(mInstance, nullptr);
    }
}