#include "command.h"
#include "logicaldevice.h"

CommandBuffer::CommandBuffer(const CommandPool* pool, VkCommandBuffer buffer) :
	mCommandPool(pool), mCommandBuffer(buffer) {}

CommandPool::CommandPool(const LogicalDevice* device, uint32_t family, VkCommandPool pool) :
	mDevice(device), mFamily(family), mCommandPool(pool) {}

void CommandBuffer::Reset() {
	vkResetCommandBuffer(mCommandBuffer, 0);
}

bool CommandBuffer::Begin() {
	VkCommandBufferBeginInfo info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr, .flags = 0,
		.pInheritanceInfo = nullptr
	};
	return vkBeginCommandBuffer(mCommandBuffer, &info) == VK_SUCCESS;
}

bool CommandBuffer::End() {
	return vkEndCommandBuffer(mCommandBuffer) == VK_SUCCESS;
}

PCommandPool CommandPool::CreateCommandPool(
	const LogicalDevice* device, VkCommandPoolCreateFlags flags, uint32_t family) {
	VkCommandPoolCreateInfo info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags, .queueFamilyIndex = family
	};
	VkCommandPool pool;
	auto result = vkCreateCommandPool(*device, &info, nullptr, &pool);
	if (result != VK_SUCCESS) { return nullptr; }
	return PCommandPool(new CommandPool(device, family, pool));
}

std::vector<PCommandBuffer> CommandPool::AllocateCommandBuffer(
	VkCommandBufferLevel level, uint32_t number) {
	VkCommandBufferAllocateInfo info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = this->operator const VkCommandPool & (),
		.level = level,
		.commandBufferCount = number
	};
	std::vector<VkCommandBuffer> buffers(number);
	std::vector<PCommandBuffer> pbuffers;
	auto result = vkAllocateCommandBuffers(*mDevice, &info, buffers.data());
	if (result != VK_SUCCESS) { return pbuffers; }
	for (int i = 0; i < number; i++) {
		pbuffers.emplace_back(new CommandBuffer(this, buffers[i]));
	}
	return pbuffers;
}

CommandPool::~CommandPool() {
	if (mCommandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(*mDevice, mCommandPool, nullptr);
	}
}