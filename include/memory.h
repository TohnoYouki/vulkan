#pragma once
#include "core.h"

class DeviceMemory {
private:
	const LogicalDevice* mDevice = nullptr;
	VkDeviceMemory mMemory = VK_NULL_HANDLE;

	DeviceMemory(const LogicalDevice* device, VkDeviceMemory memory);
	DeviceMemory(const DeviceMemory&) = delete;
	DeviceMemory& operator=(const DeviceMemory&) = delete;
	DeviceMemory(DeviceMemory&&) = delete;
	DeviceMemory& operator=(DeviceMemory&&) = delete;
public:
	inline operator const VkDeviceMemory& () const { return mMemory; }

	static PDeviceMemory AllocateDeviceMemory(
		const LogicalDevice* device, uint32_t size, uint32_t type);

	~DeviceMemory();
};