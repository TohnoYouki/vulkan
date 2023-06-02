#include "debug.h"
#include "instance.h"

DebugModes* DebugModes::mInstance = nullptr;

DebugModes* DebugModes::GetInstance() {
    if (mInstance == nullptr) {
        static DebugModes instance;
        mInstance = &instance;
    }
    return mInstance;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

std::vector<const char*> DebugModes::GetRequiredLayers() const {
    return std::vector<const char*>({ "VK_LAYER_KHRONOS_validation" });
};

std::vector<const char*> DebugModes::GetRequiredExtensions() const {
    return std::vector<const char*>({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME });
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugModes::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "validation layer: "
            << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT DebugModes::DebugMessengerCreateInfo(
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) const {
    return VkDebugUtilsMessengerCreateInfoEXT {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr, .flags = 0, 
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr,
    };
}

PDebugMessenger DebugMessenger::CreateDebugMessenger(
    const Instance* instance,
    const VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    VkDebugUtilsMessengerEXT debugMessenger;
    if (CreateDebugUtilsMessengerEXT(
        *instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        return nullptr;
    }
    return PDebugMessenger(new DebugMessenger(instance, debugMessenger));
}

DebugMessenger::~DebugMessenger() {
    if (mDebugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(*mInstance, mDebugMessenger, nullptr);
    }
}