#pragma once
#include <map>
#include <set>
#include "core.h"
#include <vector>
#include <algorithm>
#include "instance.h"

#define MAX_QUEUE_NAME_SIZE 20U

struct QueueCreateInfo {
    char name[MAX_QUEUE_NAME_SIZE];
    uint32_t count;
    std::vector<float> priorities;
    uint32_t familyIndex;
};

class QueueManager {
private:
    typedef std::shared_ptr<std::vector<float>> PPriorities;
public:
    struct Location {
        uint32_t family;
        uint32_t offset;
        uint32_t count;
    };
    struct LesserString {
        bool operator() (const char* lhs, const char* rhs) const {
            return strcmp(lhs, rhs) < 0;
        } 
    };

    QueueManager(const std::vector<QueueCreateInfo> & infos);
    const std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfo() const;
    inline Location GetQueueLocation(const char * name) const;
private:
    std::map<uint32_t, PPriorities> mFamilies;
    std::map<const char *, Location, LesserString> mLocations;
};

class LogicalDevice {
private:
    QueueManager mManager;
    VkDevice mDevice = VK_NULL_HANDLE;
    PPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

    LogicalDevice(const PPhysicalDevice & physicalDevice, 
        const VkDevice & device, const QueueManager & manager);
    LogicalDevice(const LogicalDevice &) = delete;
    LogicalDevice & operator=(const LogicalDevice &) = delete;
    LogicalDevice(LogicalDevice &&) = delete;
    LogicalDevice & operator=(LogicalDevice &&) =delete;

public:
    operator const VkDevice & () const { return mDevice; }

    QueueManager::Location GetQueueLocation(const char * name) const;

    VkQueue GetSingleQueue(const char * name, uint32_t index) const;

    static PLogicalDevice CreateLogicalDevice(
        const PPhysicalDevice & physicalDevice,
        const std::vector<QueueCreateInfo> & queues,
        const std::vector<const char *> & extensions,
        const std::vector<const char *> & layers,
        const VkPhysicalDeviceFeatures & features);

    ~LogicalDevice();
};

class PhysicalDevice {
private:
    PInstance mInstance = VK_NULL_HANDLE;
    VkPhysicalDevice mDevice = VK_NULL_HANDLE;

    PhysicalDevice(
        const PInstance & instance, const VkPhysicalDevice & device);

    PhysicalDevice(const PhysicalDevice &) = delete;
    PhysicalDevice & operator=(const PhysicalDevice &) = delete;
    PhysicalDevice(PhysicalDevice &&) = delete;
    PhysicalDevice & operator=(PhysicalDevice &&) = delete;

public:
    VkPhysicalDeviceProperties mProperties;
    VkPhysicalDeviceFeatures mFeatures;
    std::vector<VkQueueFamilyProperties> mQueues;
    std::vector<VkExtensionProperties> mExtensions;

    bool CheckExtensionsSupport(
        const std::vector<const char *> names) const;

    static std::vector<PPhysicalDevice> 
        CreatePhsicalDevices(const PInstance & instance);
        
    operator const VkPhysicalDevice & () const { return mDevice; }
};