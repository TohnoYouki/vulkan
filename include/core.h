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

class ShaderModule;
typedef std::unique_ptr<ShaderModule> PShaderModule;

class RenderPass;
typedef std::unique_ptr<RenderPass> PRenderPass;

class PipelineLayout;
typedef std::unique_ptr<PipelineLayout> PPipelineLayout;

class GraphicsPipeline;
typedef std::unique_ptr<GraphicsPipeline> PGraphicsPipeline;

class FrameBuffer;
typedef std::unique_ptr<FrameBuffer> PFrameBuffer;

class CommandPool;
typedef std::unique_ptr<CommandPool> PCommandPool;

class CommandBuffer;
typedef std::unique_ptr<CommandBuffer> PCommandBuffer;

class Semaphore;
typedef std::unique_ptr<Semaphore> PSemaphore;

class Fence;
typedef std::unique_ptr<Fence> PFence;