#include <map>
#include "logicaldevice.h"
#include "physicaldevice.h"

LogicalDevice::LogicalDevice(const PhysicalDevice* physical, 
	VkDevice device, const QueueCreateInfo& queues) :
	mPhysical(physical), mDevice(device) {
	std::map<uint32_t, uint32_t> indices;
	for (int i = 0; i < queues.families.size(); i++) {
		uint32_t family = queues.families[i];
		if (indices.find(family) == indices.end()) {
			indices.insert(std::make_pair(family, 0));
		}
		uint32_t index = indices[family];
		indices[family] += 1;
		VkQueue queue;
		vkGetDeviceQueue(mDevice, family, index, &queue);
		mQueues.push_back(queue);
	}
}

PLogicalDevice LogicalDevice::CreateLogicalDevice(
	const PhysicalDevice* physical,
	const QueueCreateInfo& queues,
	const std::vector<const char*>& extensions,
	const std::vector<const char*>& layers,
	const VkPhysicalDeviceFeatures& features) {

	std::map<uint32_t, std::vector<float>> queueInfos;
	for (int i = 0; i < queues.families.size(); i++) {
		uint32_t family = queues.families[i];
		if (queueInfos.find(family) == queueInfos.end()) {
			queueInfos.insert(std::make_pair(family, std::vector<float>()));
		}
		queueInfos[family].push_back(queues.priorities[i]);
	}
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (const auto& pair : queueInfos) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = nullptr, .flags = 0,
			.queueFamilyIndex = pair.first,
			.queueCount = static_cast<uint32_t>(pair.second.size()),
			.pQueuePriorities = pair.second.data()
			});
	}

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
		.pEnabledFeatures = &features
	};

	VkDevice device;
	auto res = vkCreateDevice(*physical, &createInfo, nullptr, &device);
	if (res != VK_SUCCESS) { return nullptr; }
	return PLogicalDevice(new LogicalDevice(physical, device, queues));
}

LogicalDevice::~LogicalDevice() {
	if (mDevice != VK_NULL_HANDLE) {
		vkDestroyDevice(mDevice, nullptr);
	}
}