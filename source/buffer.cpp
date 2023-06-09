#include "buffer.h"
#include "logicaldevice.h"

Buffer::Buffer(const LogicalDevice* device, VkBuffer buffer) :
	mDevice(device), mBuffer(buffer) {}

PBuffer Buffer::CreateBuffer(const LogicalDevice* device, uint32_t size,
	VkBufferUsageFlags usage, std::vector<uint32_t> queues) {
	VkBufferCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr, .flags = 0,
		.size = size, .usage = usage,
		.sharingMode = queues.size() > 1 ?
			VK_SHARING_MODE_CONCURRENT :
			VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(queues.size()),
		.pQueueFamilyIndices = queues.data()
	};
	VkBuffer buffer;
	auto result = vkCreateBuffer(*device, &info, nullptr, &buffer);
	if (result != VK_SUCCESS) { return nullptr; }
	return PBuffer(new Buffer(device, buffer));
}

VkMemoryRequirements Buffer::GetMemoryRequirements() const {
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(*mDevice, mBuffer, &requirements);
	return requirements;
}

Buffer::~Buffer() {
	if (mBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(*mDevice, mBuffer, nullptr);
	}
}