#pragma once
#include <vector>
#include "core.h"
#include <vulkan/vulkan.h>

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator);

class DebugMessenger {
private:
    const Instance* mInstance = nullptr;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;

    DebugMessenger(const Instance* instance,
        VkDebugUtilsMessengerEXT messenger) :
        mInstance(instance), mDebugMessenger(messenger) {};
public:
    static PDebugMessenger CreateDebugMessenger(
        const Instance* instance,
        const VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    ~DebugMessenger();
};

class DebugModes {
private:
    static DebugModes* mInstance;
    DebugModes() {};
    DebugModes(const DebugModes&) = delete;
    DebugModes& operator=(const DebugModes&) = delete;
    DebugModes(DebugModes&&) = delete;
    DebugModes& operator=(DebugModes&&) = delete;
    ~DebugModes() {};
public:
    static DebugModes* GetInstance();
    std::vector<const char*> GetRequiredLayers() const;
    std::vector<const char*> GetRequiredExtensions() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(
        PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = DebugCallback) const;
};