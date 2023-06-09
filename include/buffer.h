#pragma once
#include "core.h"
#include <vector>

class Buffer {
private:
	const LogicalDevice* mDevice = nullptr;
	VkBuffer mBuffer = VK_NULL_HANDLE;

	Buffer(const LogicalDevice* device, VkBuffer buffer);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(Buffer&&) = delete;
public:
	inline operator const VkBuffer& () const { return mBuffer; }
	
	static PBuffer CreateBuffer(const LogicalDevice* device, uint32_t size, 
		VkBufferUsageFlags usage, std::vector<uint32_t> queues);

	VkMemoryRequirements GetMemoryRequirements() const;

	~Buffer();
};