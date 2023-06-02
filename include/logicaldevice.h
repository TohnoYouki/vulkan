#pragma once
#include "core.h"
#include <vector>

class LogicalDevice {
private:
	const PhysicalDevice* mPhysical = nullptr;
	VkDevice mDevice = VK_NULL_HANDLE;

	LogicalDevice(const PhysicalDevice* physical, VkDevice device);
	LogicalDevice(const LogicalDevice&) = delete;
	LogicalDevice& operator=(const LogicalDevice&) = delete;
	LogicalDevice(LogicalDevice&&) = delete;
	LogicalDevice& operator=(LogicalDevice&&) = delete;
public:
	struct QueueCreateInfo {
		std::vector<uint32_t> families;
		std::vector<float> priorities;
	};

	inline operator const VkDevice& () const { return mDevice; }

	static PLogicalDevice CreateLogicalDevice(
		const PhysicalDevice* physicalDevice,
		const QueueCreateInfo& queues,
		const std::vector<const char*>& extensions,
		const std::vector<const char*>& layers,
		const VkPhysicalDeviceFeatures& features
	);

	~LogicalDevice();
};