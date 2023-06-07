#pragma once
#include "core.h"
#include <vector>

class LogicalDevice {
public:
	struct QueueCreateInfo {
		std::vector<uint32_t> families;
		std::vector<float> priorities;
	};
private:
	const PhysicalDevice* mPhysical = nullptr;
	VkDevice mDevice = VK_NULL_HANDLE;
	std::vector<VkQueue> mQueues;

	LogicalDevice(const PhysicalDevice* physical, 
				  VkDevice device, const QueueCreateInfo& queues);
	LogicalDevice(const LogicalDevice&) = delete;
	LogicalDevice& operator=(const LogicalDevice&) = delete;
	LogicalDevice(LogicalDevice&&) = delete;
	LogicalDevice& operator=(LogicalDevice&&) = delete;
public:
	inline operator const VkDevice& () const { return mDevice; }

	inline const std::vector<VkQueue>& GetQueues() { return mQueues; }

	static PLogicalDevice CreateLogicalDevice(
		const PhysicalDevice* physicalDevice,
		const QueueCreateInfo& queues,
		const std::vector<const char*>& extensions,
		const std::vector<const char*>& layers,
		const VkPhysicalDeviceFeatures& features
	);

	~LogicalDevice();
};