#pragma once
#include <vector>
#include <cstring>
#include <algorithm>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Environment {
public:
    static std::vector<VkExtensionProperties> ExtensionProperties() {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(
            nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(
            nullptr, &count, extensions.data());
        return extensions;
    }

    static std::vector<VkLayerProperties> LayerProperties() {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> layers(count);
        vkEnumerateInstanceLayerProperties(&count, layers.data());
        return layers;
    }

    static std::vector<const char*> GlfwRequiredInstanceExtensions() {
        uint32_t count = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> result(extensions, extensions + count);
        return result;
    }

    static bool CheckAvailableExtensions(
        const std::vector<const char*>& extensions) {
        auto avaliables = ExtensionProperties();
        for (const char* name : extensions) {
            auto fn = [name](const VkExtensionProperties& x) {
                return strcmp(name, x.extensionName) == 0; };
            auto res = std::find_if(avaliables.begin(), avaliables.end(), fn);
            if (res == avaliables.end()) return false;
        }
        return true;
    }

    static bool CheckAvaliableLayers(
        const std::vector<const char*>& layers) {
        auto avaliables = LayerProperties();
        for (const char* name : layers) {
            auto fn = [name](const VkLayerProperties& x) {
                return strcmp(name, x.layerName) == 0; };
            auto res = std::find_if(avaliables.begin(), avaliables.end(), fn);
            if (res == avaliables.end()) return false;
        }
        return true;
    }

    static VkResult GlfwSurfaceCreateFn(
        VkInstance instance, void* window, VkSurfaceKHR* surface) {
        return glfwCreateWindowSurface(
            instance, (GLFWwindow*)window, nullptr, surface);
    }
};