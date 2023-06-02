#pragma once
#include <memory>
#include <vulkan/vulkan.h>

class Instance;
typedef std::unique_ptr<Instance> PInstance;

class DebugMessenger;
typedef std::unique_ptr<DebugMessenger> PDebugMessenger;

class Surface;
typedef std::unique_ptr<Surface> PSurface;
typedef VkResult SurfaceCreateFn(
    VkInstance instance, void* window, VkSurfaceKHR* surface);

class PhysicalDevice;
typedef std::unique_ptr<PhysicalDevice> PPhysicalDevice;

class LogicalDevice;
typedef std::unique_ptr<LogicalDevice> PLogicalDevice;

class SwapChain;
typedef std::unique_ptr<SwapChain> PSwapChain;

class Image;
typedef std::unique_ptr<Image> PImage;
class ImageView;
typedef std::unique_ptr<ImageView> PImageView;