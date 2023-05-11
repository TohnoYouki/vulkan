#include "instance.h"

VkApplicationInfo Instance::DefaultApplicationCreateInfo(const char * name) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;  
    return appInfo;
}

std::vector<VkPhysicalDevice> Instance::PhysicDevices() const {
    uint32_t count;
    vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(mInstance, &count, devices.data());
    return devices;
}

Instance::Instance(
    const VkApplicationInfo & applicationInfo,
    std::vector<const char *> & extensions,
    DebugModes * debugUtils) {

    std::vector<const char *> layers;
    if (debugUtils != nullptr) {
        layers = debugUtils->GetRequiredLayers();
        auto debugExtensions = debugUtils->GetRequiredExtensions();
        extensions.insert(extensions.end(), 
            debugExtensions.begin(), debugExtensions.end());
    }
        
    if (!Environment::CheckAvaliableLayers(layers)) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    if (!Environment::CheckAvailableExtensions(extensions)) {
        throw std::runtime_error("extension requested, but not available!");
    }
        
    VkInstanceCreateInfo createInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    mSupportLayers = layers;

    if (debugUtils != nullptr) {
        debugCreateInfo = debugUtils->DebugMessengerCreateInfo();
        createInfo.pNext = &debugCreateInfo;
    }

    auto result = vkCreateInstance(&createInfo, nullptr, &mInstance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    
    if (debugUtils != nullptr) {
        auto info = debugUtils->DebugMessengerCreateInfo();
        if (CreateDebugUtilsMessengerEXT(
            mInstance, &info, nullptr, &mDebugMessenger) != VK_SUCCESS) {
            vkDestroyInstance(mInstance, nullptr);
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
}

Instance::~Instance() {
    if (mDebugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(
            mInstance, mDebugMessenger, nullptr);
    }
    if (mInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(mInstance, nullptr);
    }
}