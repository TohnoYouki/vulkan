#pragma once
#include "core.h"
#include <vector>

class CommandBuffer {
private:
	const CommandPool* mCommandPool = nullptr;
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

	CommandBuffer(const CommandPool* pool, VkCommandBuffer buffer);
	CommandBuffer(const CommandBuffer&) = delete;
	CommandBuffer& operator=(const CommandBuffer&) = delete;
	CommandBuffer(CommandBuffer&&) = delete;
	CommandBuffer& operator=(CommandBuffer&&) = delete;
public:
	friend class CommandPool;
	inline operator const VkCommandBuffer& () const { return mCommandBuffer; }
	
	void Reset();
	bool Begin(VkCommandBufferUsageFlags flag);
	bool End();

	~CommandBuffer() {}
}; 

class CommandPool {
private:
	const LogicalDevice* mDevice = nullptr;
	uint32_t mFamily = 0;
	VkCommandPool mCommandPool = VK_NULL_HANDLE;

	CommandPool(const LogicalDevice* device, uint32_t family, VkCommandPool pool);
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	CommandPool(CommandPool&&) = delete;
	CommandPool& operator=(CommandPool&&) = delete;
public:
	inline operator const VkCommandPool& () const { return mCommandPool; }

	static PCommandPool CreateCommandPool(
		const LogicalDevice* device, VkCommandPoolCreateFlags flags, uint32_t family);

	std::vector<PCommandBuffer> AllocateCommandBuffer(
		VkCommandBufferLevel level, uint32_t number);

	~CommandPool();
};