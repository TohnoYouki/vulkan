#pragma once
#include <vector>
#include "core.h"
#include "debug.h"
#include <stdexcept>
#include "environment.h"

class Instance {
private:
    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    std::vector<const char *> mSupportLayers;
    
    Instance(const VkApplicationInfo & applicationInfo,
        std::vector<const char *> & extensions,
        DebugModes * debugUtils = nullptr);

    Instance(const Instance &) = delete;
    Instance & operator=(const Instance &) = delete;
    Instance(Instance &&) = delete;
    Instance & operator=(Instance &&) = delete;

public:
    operator const VkInstance & () const { return mInstance; }

    std::vector<VkPhysicalDevice> PhysicDevices() const;

    inline const std::vector<const char *> & GetSupportLayers() const {
        return mSupportLayers;
    }

    static inline PInstance CreateInstance(
        const VkApplicationInfo & applicationInfo,
        std::vector<const char *> & extensions, 
        DebugModes * debugUtils = nullptr) {
        return PInstance(new Instance(
            applicationInfo, extensions, debugUtils));
    }

    static VkApplicationInfo DefaultApplicationCreateInfo(const char * name);

    ~Instance();
};