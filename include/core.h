#pragma once
#include <memory>
#include <vulkan/vulkan.h>

class DebugModes;
class Environment;

class Instance;
typedef std::shared_ptr<Instance> PInstance;

class Surface;
typedef std::shared_ptr<Surface> PSurface;
typedef VkResult SurfaceCreateFn(
    VkInstance instance, void * window, VkSurfaceKHR * surface);

class PhysicalDevice;
typedef std::shared_ptr<PhysicalDevice> PPhysicalDevice;

class LogicalDevice;
typedef std::shared_ptr<LogicalDevice> PLogicalDevice;

class SwapChain;
typedef std::shared_ptr<SwapChain> PSwapChain;