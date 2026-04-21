#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include "RenderPassManager.h"
#include "Scene/CullingSystem.h"
#include "Services/ShaderCompiler.h"

#include <vector>

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

    void DrawFrame();

    void ShutDown();

private:
    static const std::vector<char const*> s_validationLayers;
    static const std::vector<const char*> s_requiredDeviceExtensions;
    static constexpr int s_maxFramesInFlight = 2;
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
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void TransitionImageLayout(
	    uint32_t                imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask);
    void RecordCommandBuffer(const uint32_t imageIndex);
    void CreateSyncObjects();

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    vk::raii::ShaderModule CreateShaderModule(ShaderBlob* bytecode);

    void SetupRenderPasses() {
        // Create geometry pass

        // Create lighting pass

        // Create post-process pass

        // Add post-process effects
        // ...
    }

private:
    GLFWwindow* m_window = nullptr;

    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;

    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_logicalDevice = nullptr;
    vk::raii::Queue m_graphicsQueue = nullptr;
    vk::PhysicalDeviceFeatures m_deviceFeatures;

    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;

    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    vk::raii::CommandPool m_commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;

    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;

    bool m_enableValidationLayers = false;

    uint32_t m_frameIndex = 0;

    // Unused
    vk::raii::Device m_device = nullptr;
    vk::Queue m_tmpGraphicsQueue;

    RenderPassManager m_renderPassManager;
    CullingSystem m_cullingSystem;

    vk::raii::Fence m_fence = nullptr;
    vk::raii::Semaphore m_imageAvailableSemaphore = nullptr;
};
}