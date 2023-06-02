#pragma once
#include <vector>
#include "core.h"
#include "debug.h"
#include <stdexcept>
#include "environment.h"

class Instance {
private:
    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mSupportLayers;
    std::vector<PPhysicalDevice> mPhysicalDevices;

    Instance(VkInstance instance, const std::vector<const char*>& layers);
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(Instance&&) = delete;

public:
    operator const VkInstance& () const { return mInstance; }

    inline const std::vector<const char*>& GetSupportLayers() const {
        return mSupportLayers;
    }

    std::vector<const PhysicalDevice*> EnumeratePhysicalDevices();
    static VkApplicationInfo DefaultApplicationCreateInfo(const char* name);

    static PInstance CreateInstance(
        const VkApplicationInfo& applicationInfo,
        std::vector<const char*>& extensions,
        DebugModes* debugUtils = nullptr);

    ~Instance();
};