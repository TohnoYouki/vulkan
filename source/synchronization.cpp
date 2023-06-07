#include "logicaldevice.h"
#include "synchronization.h"

Semaphore::Semaphore(const LogicalDevice* device, VkSemaphore semaphore) :
	mDevice(device), mSemaphore(semaphore) {}

PSemaphore Semaphore::CreateSemaphore(const LogicalDevice* device) {
	VkSemaphoreCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		 .pNext = nullptr, .flags = 0
	};
	VkSemaphore semaphore;
	auto result = vkCreateSemaphore(*device, &info, nullptr, &semaphore);
	if (result != VK_SUCCESS) { return nullptr; }
	return PSemaphore(new Semaphore(device, semaphore));
}

Semaphore::~Semaphore() {
	if (mSemaphore != VK_NULL_HANDLE) {
		vkDestroySemaphore(*mDevice, mSemaphore, nullptr);
	}
}

Fence::Fence(const LogicalDevice* device, VkFence fence) :
	mDevice(device), mFence(fence) {}

PFence Fence::CreateFence(const LogicalDevice* device, VkFenceCreateFlags flag) {
	VkFenceCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr, .flags = flag
	};
	VkFence fence;
	auto result = vkCreateFence(*device, &info, nullptr, &fence);
	if (result != VK_SUCCESS) { return nullptr; }
	return PFence(new Fence(device, fence));
}

Fence::~Fence() {
	if (mFence != VK_NULL_HANDLE) {
		vkDestroyFence(*mDevice, mFence, nullptr);
	}
}