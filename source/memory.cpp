#include "memory.h"
#include "logicaldevice.h"

DeviceMemory::DeviceMemory(const LogicalDevice* device, VkDeviceMemory memory) :
	mDevice(device), mMemory(memory) {}

PDeviceMemory DeviceMemory::AllocateDeviceMemory(
	const LogicalDevice* device, uint32_t size, uint32_t type) {

    VkMemoryAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = size,
        .memoryTypeIndex = type
    };
    VkDeviceMemory memory;
    auto result = vkAllocateMemory(*device, &info, nullptr, &memory);
    if (result != VK_SUCCESS) { return nullptr; }
    return PDeviceMemory(new DeviceMemory(device, memory));
}

DeviceMemory::~DeviceMemory() {
    if (mMemory != VK_NULL_HANDLE) {
        vkFreeMemory(*mDevice, mMemory, nullptr);
    }
}