#include <map>
#include <limits>
#include <memory>
#include "debug.h"
#include <iostream>
#include <optional> 
#include "device.h"
#include <stdexcept>
#include <algorithm>
#include "surface.h"
#include "instance.h"
#include "swapchain.h"
#include "environment.h"
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
    GLFWwindow * mWindow;
    
    PSurface mSurface;
    PInstance mInstance;
    PPhysicalDevice mPhysicalDevice;
    PLogicalDevice mLogicalDevice;
    VkQueue mQueueHandle;
    PSwapChain mSwapChain;
    std::vector<PImageView> mSwapChainImageViews;
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
    }

    void CreateSurface() {
        mSurface = Surface::CreateSurface(mInstance, mWindow,
            [](VkInstance instance, void * window, VkSurfaceKHR * surface) {
            return glfwCreateWindowSurface(
                instance, (GLFWwindow *)window, nullptr, surface);});
        if (mSurface == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    std::optional<uint32_t> FindGraphicQueue(PPhysicalDevice device) {
        auto bit = VK_QUEUE_GRAPHICS_BIT;
        for (uint32_t i = 0; i < device->mQueues.size(); i++) {
            const auto & x = device->mQueues[i];
            if ((x.queueFlags & bit) && x.queueCount > 0)
                return std::optional<uint32_t>(i);
        }
        return std::optional<uint32_t>();
    }

    std::optional<uint32_t> FindPresentQueue(PPhysicalDevice device) {
        for (uint32_t i = 0; i < device->mQueues.size(); i++) {
            const auto & x = device->mQueues[i];
            VkBool32 support = false;
            if (x.queueCount <= 0) { continue; }
            if (mSurface->GetPhysicalDeviceSupport(device, i)) {
                return std::optional<uint32_t>(i);
            }
        }
        return std::optional<uint32_t>();
    }

    const std::vector<const char *> mDeviceExtensions = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    bool CheckPhysicalDeviceSupport(const PPhysicalDevice & device) {
        if (!device->mFeatures.geometryShader) { return false; }
        auto graphic = FindGraphicQueue(device);
        auto present = FindPresentQueue(device);
        if (!graphic.has_value() or !present.has_value()) { return false; }
        if (graphic.value() != present.value()) { return false; }
        if (!device->CheckExtensionsSupport(mDeviceExtensions)) { return false; }
        auto support = mSurface->GetSwapChainSupportDetails(device);
        if (support.formats.empty() or support.presentModes.empty()) { return false; }
        return true;
    }

    int PhysicalDeviceScore(const PPhysicalDevice & device) {
        int score = 0;
        if (device->mProperties.deviceType 
            == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { score += 1000; }
        score += device->mProperties.limits.maxImageDimension2D;
        return score;
    }

    void PickPhysicalDevices() {
        auto devices = PhysicalDevice::CreatePhsicalDevices(mInstance);
        if (devices.empty()) {
            throw std::runtime_error(
                "failed to find GPUs with Vulkan support!");
        }
        std::multimap<int, const PPhysicalDevice *> rank;
        for (const auto & device: devices) {
            if (!CheckPhysicalDeviceSupport(device)) { continue; }
            rank.insert(std::make_pair(
                PhysicalDeviceScore(device), &device));
        }
        if (rank.begin()->first > 0) {
            mPhysicalDevice = *(rank.begin()->second);
        } else {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void CreateLogicalDevice() {
        auto graphicIndex = FindGraphicQueue(mPhysicalDevice);
        auto presentIndex = FindPresentQueue(mPhysicalDevice);

        std::vector<QueueCreateInfo> queues;
        queues.emplace_back(QueueCreateInfo(
            {"present", 1, {1.0}, graphicIndex.value()}));

        std::vector<const char *> extensions = mDeviceExtensions;
        VkPhysicalDeviceFeatures features{};
        mLogicalDevice = LogicalDevice::CreateLogicalDevice(
            mPhysicalDevice, queues, extensions, 
            mInstance->GetSupportLayers(), features);

        if (mLogicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to create logical device!");
        }
        mQueueHandle = mLogicalDevice->GetSingleQueue("present", 0);
    }

    VkSurfaceFormatKHR SelectSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> & formats) {
        auto result = formats[0];
        for (const auto & format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && 
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                result = format;
            }
        }
        return result;
    }

    VkPresentModeKHR SelectPresentMode(
        const std::vector<VkPresentModeKHR> & modes) {
        auto result = modes[0];
        for (const auto & mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                result = mode;
                break;
            } else if (mode == VK_PRESENT_MODE_FIFO_KHR) {
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
        } else {
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
        SwapChain::CreateParams params;
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
        auto presentIndex = mLogicalDevice->GetQueueLocation("present").family;
        auto graphicIndex = mLogicalDevice->GetQueueLocation("graphic").family;
        params.queueFamilies.push_back(presentIndex);
        if (graphicIndex != presentIndex) {
            params.queueFamilies.push_back(graphicIndex);
        } 
        auto createInfo = SwapChain::DefaultCreateInfo(mSurface, params);
        mSwapChain = SwapChain::CreateSwapChain(
            mLogicalDevice, mSurface, createInfo);
    }

    void CreateImageView() {
        const auto & images = mSwapChain->GetImages();
        for (const auto & image : images) {
            Image::ImageViewCreateInfo createInfo;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = VK_FORMAT_UNDEFINED;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            mSwapChainImageViews.push_back(image.CreateImageView(createInfo));
        }
    }

    void Init() {
        InitWindow();
        CreateInstance();
        CreateSurface();
        PickPhysicalDevices();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageView();
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
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}