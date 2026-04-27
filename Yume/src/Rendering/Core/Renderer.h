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
#include "Services/EventSystem/EventBus.h"

#include <vector>
#include <glm/glm.hpp>

namespace Yume {

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = vk::VertexInputRate::eVertex;
        return binding;
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 3> attributes{};

        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = vk::Format::eR32G32B32Sfloat;
        attributes[0].offset = offsetof(Vertex, pos);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = vk::Format::eR32G32B32Sfloat;
        attributes[1].offset = offsetof(Vertex, color);

        attributes[2].binding = 0;
        attributes[2].location = 2;
        attributes[2].format = vk::Format::eR32G32Sfloat;
        attributes[2].offset = offsetof(Vertex, texCoord);

        return attributes;
    }
};

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
};

class Renderer : public EventListener {

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

    void OnEvent(const Event& event) override;

private:
    static const std::vector<char const*> s_validationLayers;
    static const std::vector<const char*> s_requiredDeviceExtensions;
    static constexpr int s_maxFramesInFlight = 2;
    static constexpr int s_particleCount = 1;
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
    void CreateSwapChainImageViews();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void TransitionImageLayout(
	    vk::Image               image,
	    vk::ImageLayout         oldLayout,
	    vk::ImageLayout         newLayout,
	    vk::AccessFlags2        srcAccessMask,
	    vk::AccessFlags2        dstAccessMask,
	    vk::Flags<vk::PipelineStageFlagBits2> srcStageMask,
	    vk::Flags<vk::PipelineStageFlagBits2> dstStageMask,
        vk::ImageAspectFlags    imageAspectFlags);
    void RecordCommandBuffer(const uint32_t imageIndex);
    void CreateSyncObjects();
    void HandleResize();
    void CleanupSwapChain();
    void CreateTextureImage();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
    void CopyBuffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & dstBuffer, vk::DeviceSize size);
    void CreateGraphicsDescriptorSetLayout();
    void CreateComputeDescriptorSetLayout();
    void CreateUniformBuffers();
    void UpdateUniformBuffer(const int frameIndex);
    void CreateGraphicsDescriptorPool();
    void CreateComputeDescriptorPool();
    void CreateGraphicsDescriptorSets();
    void CreateComputeDescriptorSets();
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& outImage, vk::raii::DeviceMemory& outImageMemory);
    vk::raii::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);
    void TransitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
    void CopyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);
    void CreateTextureImageView();
    vk::raii::ImageView CreateImageView(const vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags imageFlags, uint32_t mipLevels);
    void CreateTextureSampler();
    void CreateDepthResources();
    vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    vk::Format FindDepthFormat();
    bool HasStencilComponent(vk::Format format);
    void LoadModel();
    void GenerateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t width, int32_t height, uint32_t mipLevels);
    vk::SampleCountFlagBits GetMaxUsableSampleCount();
    void CreateColorResources();
    void CreateShaderStorageBuffers();
    void CreateComputePipeline();

    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

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

    void HandleWindowResize(const int width, const int height);

private:
    GLFWwindow* m_window = nullptr;

    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;

    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    uint32_t m_graphicsComputeQueueIndex = ~0;
    vk::raii::Device m_logicalDevice = nullptr;
    vk::raii::Queue m_graphicsComputeQueue = nullptr;
    vk::PhysicalDeviceFeatures m_deviceFeatures;

    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;

    vk::Format m_depthFormat;
    vk::raii::Image m_depthImage = nullptr;
    vk::raii::DeviceMemory m_depthImageMemory = nullptr;
    vk::raii::ImageView m_depthImageView = nullptr;

    vk::raii::DeviceMemory m_colorImageMemory = nullptr;
    vk::raii::Image m_colorImage = nullptr;
    vk::raii::ImageView m_colorImageView = nullptr;

    vk::raii::DescriptorSetLayout m_graphicsDescriptorSetLayout = nullptr;
    vk::raii::DescriptorSetLayout m_computeDescriptorSetLayout = nullptr;
    vk::raii::PipelineLayout m_graphicsPipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;
    vk::raii::PipelineLayout m_computePipelineLayout = nullptr;
    vk::raii::Pipeline m_computePipeline = nullptr;

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

    vk::raii::Buffer m_vertexBuffer = nullptr;
    vk::raii::DeviceMemory m_vertexBufferMemory = nullptr;

    vk::raii::Buffer m_indexBuffer = nullptr;
    vk::raii::DeviceMemory m_indexBufferMemory = nullptr;

    std::vector<vk::raii::Buffer> m_uniformBuffers;
    std::vector<vk::raii::DeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    std::vector<vk::raii::Buffer> m_shaderStorageBuffers;
    std::vector<vk::raii::DeviceMemory> m_shaderStorageBuffersMemory; 

    vk::raii::DescriptorPool m_descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> m_graphicsDescriptorSets;
    std::vector<vk::raii::DescriptorSet> m_computeDescriptorSets;

    uint32_t m_mipLevels;
    vk::raii::Image m_textureImage = nullptr;
    vk::raii::DeviceMemory m_textureImageMemory = nullptr;

    vk::raii::ImageView m_textureImageView = nullptr;
    vk::raii::Sampler m_textureSampler = nullptr;

    bool m_windowResized = false;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;

    int m_width;
    int m_height;
};
}