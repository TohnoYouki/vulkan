#include <map>
#include <limits>
#include <memory>
#include "debug.h"
#include <iostream>
#include <optional> 
#include <stdexcept>
#include <algorithm>
#include "shader.h"
#include "surface.h"
#include "instance.h"
#include "environment.h"
#include "physicaldevice.h"
#include "logicaldevice.h"
#include "swapchain.h"
#include "renderpass.h"
#include "pipelinelayout.h"
#include "pipeline.h"
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

    const bool mEnableValidationLaryers = true;

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
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
        } else {
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
            Image::ImageViewCreateInfo createInfo {
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_UNDEFINED,
                .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .a = VK_COMPONENT_SWIZZLE_IDENTITY },
                .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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
        std::vector<VkAttachmentDescription> attachments {
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
        }};
        std::vector<VkAttachmentReference> refs {
            VkAttachmentReference {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }};
        std::vector<VkSubpassDescription> subpasses {
            VkSubpassDescription {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0, .pInputAttachments = nullptr,
                .colorAttachmentCount = 1, .pColorAttachments = refs.data(),
                .pResolveAttachments = nullptr, .pDepthStencilAttachment = nullptr,
                .preserveAttachmentCount = 0, .pPreserveAttachments = nullptr 
        }};
        mRenderPass = RenderPass::CreateRenderPass(
            mLogicalDevice.get(), attachments, subpasses);
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

        VkPipelineVertexInputStateCreateInfo vertexInput {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr 
        };
        VkPipelineInputAssemblyStateCreateInfo inputAssembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };
        std::vector<VkViewport> viewports {
            VkViewport {
                .x = 0.0f, .y = 0.0f,
                .width = static_cast<float>(mSwapChain->ImageInfo().extent.width), 
                .height = static_cast<float>(mSwapChain->ImageInfo().extent.height),
                .minDepth = 0.0f, .maxDepth = 1.0f
            }
        };
        std::vector<VkRect2D> scissors {
            VkRect2D {
                .offset = {0, 0}, 
                .extent = VkExtent2D{
                    .width = mSwapChain->ImageInfo().extent.width,
                    .height = mSwapChain->ImageInfo().extent.height
        }}};
        VkPipelineViewportStateCreateInfo viewState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .viewportCount = static_cast<uint32_t>(viewports.size()), 
            .pViewports = viewports.data(),
            .scissorCount = static_cast<uint32_t>(scissors.size()), 
            .pScissors = scissors.data()
        };
        VkPipelineRasterizationStateCreateInfo rasterization {
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
        VkPipelineMultisampleStateCreateInfo multisample {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr, .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };
        std::vector<VkPipelineColorBlendAttachmentState> attachmentBlends {
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
        VkPipelineColorBlendStateCreateInfo colorblend {
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
        VkGraphicsPipelineCreateInfo info {
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
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
        }
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