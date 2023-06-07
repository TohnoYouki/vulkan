#pragma once
#include "core.h"

class Semaphore {
private:
	const LogicalDevice* mDevice = nullptr;
	VkSemaphore mSemaphore = VK_NULL_HANDLE;

	Semaphore(const LogicalDevice* device, VkSemaphore semaphore);
	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;
	Semaphore(Semaphore&&) = delete;
	Semaphore& operator=(Semaphore&&) = delete;
public:
	inline operator const VkSemaphore& () const { return mSemaphore; }
	static PSemaphore CreateSemaphore(const LogicalDevice* device);
	~Semaphore();
};

class Fence {
private:
	const LogicalDevice* mDevice = nullptr;
	VkFence mFence = VK_NULL_HANDLE;

	Fence(const LogicalDevice* device, VkFence fence);
	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;
	Fence(Fence&&) = delete;
	Fence& operator=(Fence&&) = delete;
public:
	inline operator const VkFence& () const { return mFence; }
	static PFence CreateFence(const LogicalDevice* device, VkFenceCreateFlags flag);
	~Fence();
};