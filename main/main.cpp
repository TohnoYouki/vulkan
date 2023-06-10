#include <map>
#include <limits>
#include <memory>
#include <iostream>
#include <optional> 
#include <stdexcept>
#include <algorithm>
#include "head.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Application {
public:
    void Run() {
        Init();
        MainLoop();
        Cleanup();
    }
private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    GLFWwindow* mWindow;

    uint32_t currentFrame = 0;
    bool frameBufferResized = false;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    PInstance mInstance = nullptr;
    PDebugMessenger mDebugMessenger = nullptr;
    PSurface mSurface = nullptr;
    const PhysicalDevice* mPhysicalDevice = nullptr;
    uint32_t mQueueFamily = 0;
    PLogicalDevice mLogicalDevice = nullptr;
    PSwapChain mSwapChain = nullptr;
    std::vector<PImageView> mSwapChainImageViews;
    PRenderPass mRenderPass = nullptr;
    PPipelineLayout mPipelineLayout = nullptr;
    PGraphicsPipeline mGraphicsPipeline = nullptr;
    PBuffer mVertexBuffer = nullptr;
    PDeviceMemory mVertexMemory = nullptr;
    PBuffer mIndexBuffer = nullptr;
    PDeviceMemory mIndexMemory = nullptr;
    std::vector<PFrameBuffer> mFramebuffers;
    PCommandPool mCommandPool = nullptr;
    std::vector<PCommandBuffer> mCommandBuffers;
    std::vector<PSemaphore> mImageAvailableSemaphores;
    std::vector<PSemaphore> mRenderFinishedSemaphores;
    std::vector<PFence> mInFlightFences;

    const bool mEnableValidationLaryers = true;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        application->frameBufferResized = true;
    }

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
    }

    void CreateInstance() {
        auto appInfo = Instance::DefaultApplicationCreateInfo(
            "Vulkan GPU Driven Pipeline");
        auto extensions = Environment::GlfwRequiredInstanceExtensions();
        auto debugUtils = mEnableValidationLaryers ?
            DebugModes::GetInstance() : nullptr;
        mInstance = Instance::CreateInstance(appInfo, extensions, debugUtils);
        if (mInstance == nullptr) {
            throw std::runtime_error("failed to create an Instance!");
        }
        if (mEnableValidationLaryers) {
            auto debugInfo = debugUtils->DebugMessengerCreateInfo();
            mDebugMessenger = DebugMessenger::CreateDebugMessenger(
                mInstance.get(), debugInfo);
            if (mDebugMessenger == nullptr) {
                throw std::runtime_error("failed to create a debugMessenger!");
            }
        }
    }

    void CreateSurface() {
        mSurface = Surface::CreateSurface(
            mInstance.get(), mWindow, Environment::GlfwSurfaceCreateFn);
        if (mSurface == nullptr) {
            throw std::runtime_error("failed to create a surface!");
        }
    }

    std::optional<uint32_t> FindGraphicQueue(const PhysicalDevice* device) {
        auto bit = VK_QUEUE_GRAPHICS_BIT;
        for (uint32_t i = 0; i < device->mQueuesProperties.size(); i++) {
            const auto& x = device->mQueuesProperties[i];
            if ((x.queueFlags & bit) && x.queueCount > 0)
                return std::optional<uint32_t>(i);
        }
        return std::optional<uint32_t>();
    }

    std::optional<uint32_t> FindPresentQueue(const PhysicalDevice* device) {
        for (uint32_t i = 0; i < device->mQueuesProperties.size(); i++) {
            const auto& x = device->mQueuesProperties[i];
            VkBool32 support = false;
            if (x.queueCount <= 0) { continue; }
            if (mSurface->PhysicalDeviceSupport(device, i)) {
                return std::optional<uint32_t>(i);
            }
        }
        return std::optional<uint32_t>();
    }

    const std::vector<const char*> mDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    bool CheckPhysicalDeviceSupport(const PhysicalDevice* device) {
        if (!device->mFeatures.geometryShader) { return false; }
        auto graphic = FindGraphicQueue(device);
        auto present = FindPresentQueue(device);
        if (!graphic.has_value() or !present.has_value()) { return false; }
        if (graphic.value() != present.value()) { return false; }
        mQueueFamily = graphic.value();
        if (!device->CheckExtensionsSupport(mDeviceExtensions)) { return false; }
        auto support = mSurface->GetSwapChainSupportDetails(device);
        if (support.formats.empty() or support.presentModes.empty()) { return false; }
        return true;
    }

    int PhysicalDeviceScore(const PhysicalDevice* device) {
        int score = 0;
        if (device->mProperties.deviceType
            == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        score += device->mProperties.limits.maxImageDimension2D;
        return score;
    }

    void PickPhysicalDevice() {
        auto devices = mInstance->EnumeratePhysicalDevices();
        if (devices.empty()) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::multimap<int, const PhysicalDevice*> rank;
        for (const auto& device : devices) {
            if (!CheckPhysicalDeviceSupport(device)) { continue; }
            rank.insert(std::make_pair(
                PhysicalDeviceScore(device), device));
        }
        if (rank.begin()->first > 0) {
            mPhysicalDevice = rank.begin()->second;
        }
        else {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void CreateLogicalDevice() {
        LogicalDevice::QueueCreateInfo queues{
            .families = { mQueueFamily }, .priorities = { 1.0 } };
        mLogicalDevice = LogicalDevice::CreateLogicalDevice(
            mPhysicalDevice, queues, mDeviceExtensions,
            DebugModes::GetInstance()->GetRequiredLayers(),
            VkPhysicalDeviceFeatures{});
        if (mLogicalDevice == nullptr) {
            throw std::runtime_error("failed to create a logical device!");
        }
    }

    VkSurfaceFormatKHR SelectSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats) {
        VkSurfaceFormatKHR result = formats[0];
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                result = format;
            }
        }
        return result;
    }

    VkPresentModeKHR SelectPresentMode(
        const std::vector<VkPresentModeKHR>& modes) {
        VkPresentModeKHR result = modes[0];
        for (const auto& mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                result = mode;
                break;
            }
            else if (mode == VK_PRESENT_MODE_FIFO_KHR) {
                result = mode;
            }
        }
        return result;
    }

    VkExtent2D ChooseSwapChainExtent(
        const VkSurfaceCapabilitiesKHR capabilities) {
        VkExtent2D result;
        if (capabilities.currentExtent.width
            != std::numeric_limits<uint32_t>::max()) {
            result = capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);
            result.height = std::clamp(
                static_cast<uint32_t>(height),
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);
            result.width = std::clamp(
                static_cast<uint32_t>(width),
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width);
        }
        return result;
    }

    void CreateSwapChain() {
        SwapChain::SwapChainCreateParams params;
        auto details = mSurface->GetSwapChainSupportDetails(mPhysicalDevice);
        params.format = SelectSurfaceFormat(details.formats);
        params.presentMode = SelectPresentMode(details.presentModes);
        params.extent = ChooseSwapChainExtent(details.capabilities);
        params.imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 &&
            params.imageCount > details.capabilities.maxImageCount) {
            params.imageCount = details.capabilities.maxImageCount;
        }
        params.transform = details.capabilities.currentTransform;
        params.queueFamilies.push_back(mQueueFamily);
        auto createInfo = SwapChain::DefaultCreateInfo(mSurface.get(), params);
        mSwapChain = SwapChain::CreateSwapChain(
            mLogicalDevice.get(), mSurface.get(), createInfo);
        if (mSwapChain == nullptr) {
            throw std::runtime_error("failed to create a swap chain!");
        }
    }

    void CreateImageViews() {
        const auto& images = mSwapChain->GetImages();
        for (const auto& image : images) {
            Image::ImageViewCreateInfo createInfo{
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_UNDEFINED,
                .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .a = VK_COMPONENT_SWIZZLE_IDENTITY },
                .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                      .baseMipLevel = 0, .levelCount = 1,
                                      .baseArrayLayer = 0, .layerCount = 1 } };
            PImageView view = image->CreateImageView(createInfo);
            if (view == nullptr) {
                throw std::runtime_error("failed to create an image view!");
            }
            mSwapChainImageViews.push_back(std::move(view));
        }
    }

    void CreateRenderPass() {
        std::vector<VkAttachmentDescription> attachments{
            VkAttachmentDescription {
                .flags = 0,
                .format = mSwapChainImageViews[0]->ImageViewInfo().format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        } };
        std::vector<VkAttachmentReference> refs{
            VkAttachmentReference {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        } };
        std::vector<VkSubpassDescription> subpasses{
            VkSubpassDescription {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0, .pInputAttachments = nullptr,
                .colorAttachmentCount = 1, .pColorAttachments = refs.data(),
                .pResolveAttachments = nullptr, .pDepthStencilAttachment = nullptr,
                .preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
        } };  
        
        std::vector<VkSubpassDependency> dependencies {
            VkSubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0
            }
        };
        mRenderPass = RenderPass::CreateRenderPass(
            mLogicalDevice.get(), attachments, subpasses, dependencies);
        if (mRenderPass == nullptr) {
            throw std::runtime_error("failed to create a render pass!");
        }
    }

    void CreatePipelineLayout() {
        mPipelineLayout = PipelineLayout::CreatePipelineLayout(mLogicalDevice.get());
        if (mPipelineLayout == nullptr) {
            throw std::runtime_error("failed to create a pipeline layout");
        }
    }

    struct VertexData {
        glm::vec2 position;
        glm::vec3 color;
    };

    const std::vector<VertexData> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    void CreateGraphicPipeline() {
        auto vertex = ShaderModule::CreateShaderModule(
            mLogicalDevice.get(), "./spv/shader_vert.spv");
        auto fragment = ShaderModule::CreateShaderModule(
            mLogicalDevice.get(), "./spv/shader_frag.spv");
        if (vertex == nullptr || fragment == nullptr) {
            throw std::runtime_error("failed to load shader module!");
        }
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        { vertex->GetStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
          fragment->GetStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT) };

        std::vector<VkVertexInputBindingDescription> inputBindings{
            VkVertexInputBindingDescription {
                .binding = 0, .stride = sizeof(VertexData),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            }
        };

        std::vector<VkVertexInputAttributeDescription> inputAttributes{
            VkVertexInputAttributeDescription {
                .location = 0, .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(VertexData, position)
            },
            VkVertexInputAttributeDescription {
                .location = 1, .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexData, color)
            }
        };

        VkPipelineVertexInputStateCreateInfo vertexInput{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(inputBindings.size()),
            .pVertexBindingDescriptions = inputBindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributes.size()),
            .pVertexAttributeDescriptions = inputAttributes.data()
        };
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };
        std::vector<VkViewport> viewports{
            VkViewport {
                .x = 0.0f, .y = 0.0f,
                .width = 0.0f, .height = 0.0f,
                .minDepth = 0.0f, .maxDepth = 1.0f
            }
        };
        std::vector<VkRect2D> scissors{
            VkRect2D {
                .offset = {0, 0},
                .extent = VkExtent2D {0, 0}
        } };
        VkPipelineViewportStateCreateInfo viewState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .viewportCount = static_cast<uint32_t>(viewports.size()),
            .pViewports = viewports.data(),
            .scissorCount = static_cast<uint32_t>(scissors.size()),
            .pScissors = scissors.data()
        };
        VkPipelineRasterizationStateCreateInfo rasterization{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = false,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };
        VkPipelineMultisampleStateCreateInfo multisample{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };
        std::vector<VkPipelineColorBlendAttachmentState> attachmentBlends{
            VkPipelineColorBlendAttachmentState {
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                  VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT |
                                  VK_COLOR_COMPONENT_A_BIT
            }
        };
        VkPipelineColorBlendStateCreateInfo colorblend{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = static_cast<uint32_t>(attachmentBlends.size()),
            .pAttachments = attachmentBlends.data(),
            .blendConstants = {0.0, 0.0, 0.0, 0.0}
        };
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };
        VkGraphicsPipelineCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pTessellationState = nullptr,
            .pViewportState = &viewState,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisample,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorblend,
            .pDynamicState = &dynamicState,
            .layout = *mPipelineLayout,
            .renderPass = *mRenderPass, .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
        };
        mGraphicsPipeline = GraphicsPipeline::CreateGraphicsPipeline(
            mLogicalDevice.get(), info);
        if (mGraphicsPipeline == nullptr) {
            throw std::runtime_error("failed to create a graphics pipeline!");
        }
    }

    void CreateFrameBuffers() {
        for (int i = 0; i < mSwapChainImageViews.size(); i++) {
            auto framebuffer = FrameBuffer::CreateFrameBuffer(
                mRenderPass.get(), mSwapChainImageViews[i].get());
            if (framebuffer == nullptr) {
                throw std::runtime_error("failed to create a frame buffer!");
            }
            mFramebuffers.push_back(std::move(framebuffer));
        }
    }

    void CreateCommandPoolAndBuffer() {
        mCommandPool = CommandPool::CreateCommandPool(mLogicalDevice.get(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, mQueueFamily);
        if (mCommandPool == nullptr) {
            throw std::runtime_error("failed to create a command pool!");
        }
        mCommandBuffers = mCommandPool->AllocateCommandBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT);
        if (mCommandBuffers.size() == 0) {
            throw std::runtime_error("failed to create command buffers!");
        }
    }

    int FindMemoryType(VkMemoryPropertyFlags properties, uint32_t memorybits) {
        auto memories = mPhysicalDevice->GetMemoryProperties();
        for (int i = 0; i < memories.memoryTypeCount; i++) {
            if ((memories.memoryTypes[i].propertyFlags & properties) != properties) { continue; }
            if (!(memorybits & (1 << i))) { continue; }
            return i;
        }
        return -1;
    }

    bool CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      PBuffer* pbuffer, PDeviceMemory* pmemory) {
        PBuffer buffer = Buffer::CreateBuffer(
            mLogicalDevice.get(), size, usage, std::vector<uint32_t>(mQueueFamily));
        if (buffer == nullptr) { return false; }
        auto requirements = buffer->GetMemoryRequirements();
        size = requirements.size;
        int index = FindMemoryType(properties, requirements.memoryTypeBits);
        if (index == -1) { return false; }
        PDeviceMemory memory = DeviceMemory::AllocateDeviceMemory(mLogicalDevice.get(), size, index);
        if (memory == nullptr) { return false; }
        vkBindBufferMemory(*mLogicalDevice, *buffer, *memory, 0);
        *pbuffer = std::move(buffer);
        *pmemory = std::move(memory);
        return true;
    }

    bool CopyBuffer(Buffer* source, Buffer* destination, uint32_t size) {
        auto commandBuffers = mCommandPool->AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        if (commandBuffers.size() <= 0) { return false; }
        commandBuffers[0]->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkBufferCopy region {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
        vkCmdCopyBuffer(*(commandBuffers[0]), *source, *destination, 1, &region);
        if (!commandBuffers[0]->End()) { return false; }
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0, .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr, .commandBufferCount = 1,
            .pCommandBuffers = &(commandBuffers[0]->operator const VkCommandBuffer & ()),
            .signalSemaphoreCount = 0, .pSignalSemaphores = nullptr
        };
        if (vkQueueSubmit(mLogicalDevice->GetQueues()[0], 1, &submitInfo, nullptr) != VK_SUCCESS) {
            return false;
        }
        vkQueueWaitIdle(mLogicalDevice->GetQueues()[0]);
        return true;
    }

    void CreateIndexBuffers() {
        uint32_t size = sizeof(uint16_t) * indices.size();
        PBuffer stagingBuffer;
        PDeviceMemory stagingMemory;
        if (!CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | 
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
                          &stagingBuffer, &stagingMemory)) {
            throw std::runtime_error("failed to create staging buffer!");
        }
        void* data;
        vkMapMemory(*mLogicalDevice, *stagingMemory, 0, size, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(size));
        vkUnmapMemory(*mLogicalDevice, *stagingMemory);   
        if (!CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                          &mIndexBuffer, &mIndexMemory)) {
            throw std::runtime_error("failed to create ");
        }
        if (!CopyBuffer(stagingBuffer.get(), mIndexBuffer.get(), size)) {
            throw std::runtime_error("failed to copy buffer!");
        }   
    }

    void CreateVertexBuffers() {
        uint32_t size = sizeof(VertexData) * vertices.size();
        PBuffer stagingBuffer;
        PDeviceMemory stagingMemory;
        if (!CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | 
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
                          &stagingBuffer, &stagingMemory)) {
            throw std::runtime_error("failed to create staging buffer!");
        }
        void* data;
        vkMapMemory(*mLogicalDevice, *stagingMemory, 0, size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(size));
        vkUnmapMemory(*mLogicalDevice, *stagingMemory);
        if (!CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                          &mVertexBuffer, &mVertexMemory)) {
            throw std::runtime_error("failed to create ");
        }
        if (!CopyBuffer(stagingBuffer.get(), mVertexBuffer.get(), size)) {
            throw std::runtime_error("failed to copy buffer!");
        }
    }

    void CreateSyncObjects() {
        mImageAvailableSemaphores.clear();
        mRenderFinishedSemaphores.clear();
        mInFlightFences.clear();
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            mImageAvailableSemaphores.push_back(Semaphore::CreateSemaphore(mLogicalDevice.get()));
            mRenderFinishedSemaphores.push_back(Semaphore::CreateSemaphore(mLogicalDevice.get()));
            mInFlightFences.push_back(Fence::CreateFence(mLogicalDevice.get(), VK_FENCE_CREATE_SIGNALED_BIT));
            if (mImageAvailableSemaphores.back() == nullptr ||
                mRenderFinishedSemaphores.back() == nullptr ||
                mInFlightFences.back() == nullptr) {
                throw std::runtime_error("failed to create sync objects!");
            }
        }
    }

    void RecordCommandBuffer(int index) {
        mCommandBuffers[currentFrame]->Reset();
        if (!mCommandBuffers[currentFrame]->Begin(0)) {
            throw std::runtime_error("failed to begin a command buffer!");
        }
        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        VkRenderPassBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = *mRenderPass,
            .framebuffer = *(mFramebuffers[index]),
            .renderArea = { {0, 0}, VkExtent2D {
                .width = mSwapChain->ImageInfo().extent.width,
                .height = mSwapChain->ImageInfo().extent.height } },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };
        vkCmdBeginRenderPass(*mCommandBuffers[currentFrame], &info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(*mCommandBuffers[currentFrame], 
                          VK_PIPELINE_BIND_POINT_GRAPHICS, *mGraphicsPipeline);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(*mCommandBuffers[currentFrame], 0, 1, 
            &(mVertexBuffer->operator const VkBuffer & ()), offsets);
        vkCmdBindIndexBuffer(*mCommandBuffers[currentFrame], 
            *mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
      
        std::vector<VkViewport> viewports{
            VkViewport {
                .x = 0.0f, .y = 0.0f,
                .width = static_cast<float>(mSwapChain->ImageInfo().extent.width),
                .height = static_cast<float>(mSwapChain->ImageInfo().extent.height),
                .minDepth = 0.0f, .maxDepth = 1.0f
            }
        };
        std::vector<VkRect2D> scissors{
            VkRect2D {
                .offset = {0, 0},
                .extent = VkExtent2D{
                    .width = mSwapChain->ImageInfo().extent.width,
                    .height = mSwapChain->ImageInfo().extent.height
        }} };
        vkCmdSetViewport(*mCommandBuffers[currentFrame], 0,
            static_cast<uint32_t>(viewports.size()), viewports.data());
        vkCmdSetScissor(*mCommandBuffers[currentFrame], 0,
            static_cast<uint32_t>(scissors.size()), scissors.data());
        vkCmdDrawIndexed(*mCommandBuffers[currentFrame], 
            static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(*mCommandBuffers[currentFrame]);
        if (!mCommandBuffers[currentFrame]->End()) {
            throw std::runtime_error("failed to end a command buffer!");
        }
    }

    void RecreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(mWindow, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(mWindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(*mLogicalDevice);
        mFramebuffers.clear();
        mSwapChainImageViews.clear();
        mSwapChain = nullptr;
        CreateSwapChain();
        CreateImageViews();
        CreateFrameBuffers();
        CreateSyncObjects();
    }

    void Init() {
        InitWindow();
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreatePipelineLayout();
        CreateGraphicPipeline();
        CreateFrameBuffers();
        CreateCommandPoolAndBuffer();
        CreateVertexBuffers();
        CreateIndexBuffers();
        CreateSyncObjects();
    }

    void DrawFrame() {
        const VkFence* fence = &(mInFlightFences[currentFrame]->operator const VkFence & ());
        vkWaitForFences(*mLogicalDevice, 1, fence, true, UINT64_MAX);
        uint32_t index;
        auto aresult = vkAcquireNextImageKHR(*mLogicalDevice, *mSwapChain, UINT64_MAX, 
            *mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &index);
        if (aresult == VK_ERROR_OUT_OF_DATE_KHR || aresult == VK_SUBOPTIMAL_KHR || frameBufferResized) {
            frameBufferResized = false;
            RecreateSwapChain();
            return;
        } else if (aresult != VK_SUCCESS) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(*mLogicalDevice, 1, fence);
        RecordCommandBuffer(index);
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &(mImageAvailableSemaphores[currentFrame]->operator const VkSemaphore & ()),
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &(mCommandBuffers[currentFrame]->operator const VkCommandBuffer & ()),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &(mRenderFinishedSemaphores[currentFrame]->operator const VkSemaphore & ())
        };
        if (vkQueueSubmit(mLogicalDevice->GetQueues()[0], 1,
                          &submitInfo, *mInFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &(mRenderFinishedSemaphores[currentFrame]->operator const VkSemaphore & ()),
            .swapchainCount = 1,
            .pSwapchains = &(mSwapChain->operator const VkSwapchainKHR &()),
            .pImageIndices = &index,
            .pResults = nullptr
        };
        if (vkQueuePresentKHR(mLogicalDevice->GetQueues()[0], &presentInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
            DrawFrame();
        }
        vkDeviceWaitIdle(*mLogicalDevice);
    }

    void Cleanup() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
};

int main()
{
    Application app;
    try {
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}