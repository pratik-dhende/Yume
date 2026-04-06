#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include "RenderPassManager.h"
#include "Scene/CullingSystem.h"

namespace Yume {

class Renderer {

public:
    Renderer(const bool enableValidationLayer, GLFWwindow* window);

    void Init();

    // No need for explicit destructor with RAII objects

    void SetCamera(Camera* camera) {
        m_cullingSystem.SetCamera(camera);
    }

    void Render(const std::vector<Entity*>& entities);

private:
    static const std::vector<char const*> s_validationLayers;
    static const std::vector<const char*> s_requiredDeviceExtensions;
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

private:
    void InitVulkan();
    void CreateInstance();
    std::vector<const char*> GetRequiredInstanceExtensions();
    void SetupDebugMessenger();

    void SelectPhysicalDevice();
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device);
    void CreateLogicalDevice();
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    void SetupRenderPasses() {
        // Create geometry pass

        // Create lighting pass

        // Create post-process pass

        // Add post-process effects
        // ...
    }

private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;

    vk::raii::Device m_device = nullptr;
    vk::Queue m_tmpGraphicsQueue;
    vk::raii::CommandPool m_commandPool = nullptr;

    RenderPassManager m_renderPassManager;
    CullingSystem m_cullingSystem;

    // Current frame resources
    vk::raii::CommandBuffer m_commandBuffer = nullptr;
    vk::raii::Fence m_fence = nullptr;
    vk::raii::Semaphore m_imageAvailableSemaphore = nullptr;
    vk::raii::Semaphore m_renderFinishedSemaphore = nullptr;

    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_logicalDevice = nullptr;
    vk::raii::Queue m_graphicsQueue = nullptr;

    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::SwapchainKHR m_swapChain = nullptr;
    
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;

    vk::PhysicalDeviceFeatures m_deviceFeatures;

    bool m_enableValidationLayers = false;

    GLFWwindow* m_window = nullptr;
};
}