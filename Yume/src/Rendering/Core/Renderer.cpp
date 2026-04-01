#include <iostream>

#include "Renderer.h"

namespace Yume {

const std::vector<char const*> Renderer::s_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

Renderer::Renderer(const bool enableValidationLayer) : m_enableValidationLayers(enableValidationLayer) {
    SetupRenderPasses();
}

void Renderer::Render(const std::vector<Entity*>& entities) {
    // Wait for previous frame to finish
    m_device.waitForFences(*m_fence, VK_TRUE, UINT64_MAX);
    m_device.resetFences(*m_fence);

    // Reset command buffer
    m_commandBuffer.reset();

    // Perform culling
    m_cullingSystem.CullScene(entities);

    // Record commands
    vk::CommandBufferBeginInfo beginInfo;
    m_commandBuffer.begin(beginInfo);

    // Execute render passes
    m_renderPassManager.Execute(m_commandBuffer);

    m_commandBuffer.end();

    // Submit command buffer
    vk::SubmitInfo submitInfo;

    // With vk::raii, we need to dereference the command buffer
    vk::CommandBuffer rawCommandBuffer = *m_commandBuffer;
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&rawCommandBuffer);

    // Set up wait and signal semaphores
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    // With vk::raii, we need to dereference the semaphores
    vk::Semaphore rawImageAvailableSemaphore = *m_imageAvailableSemaphore;
    vk::Semaphore rawRenderFinishedSemaphore = *m_renderFinishedSemaphore;

    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(&rawImageAvailableSemaphore);
    submitInfo.setPWaitDstStageMask(waitStages);
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(&rawRenderFinishedSemaphore);

    // With vk::raii, we need to dereference the fence
    vk::Fence rawFence = *m_fence;
    m_graphicsQueue.submit(1, &submitInfo, rawFence);
}

void Renderer::Init() {
    InitVulkan();
}

void Renderer::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
}

void Renderer::SetupDebugMessenger()
{
    if (!m_enableValidationLayers) return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                          .messageType     = messageTypeFlags,
                                                                          .pfnUserCallback = &DebugCallback};
    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

}

void Renderer::CreateInstance() {
    constexpr vk::ApplicationInfo appInfo{.pApplicationName   = "Hello Triangle",
                                          .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
                                          .pEngineName        = "No Engine",
                                          .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
                                          .apiVersion         = vk::ApiVersion14};

    // Get the required layers
    std::vector<char const*> requiredLayers;
    if (m_enableValidationLayers)
    {
        requiredLayers.assign(s_validationLayers.begin(), s_validationLayers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = m_context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                    [&layerProperties](auto const &requiredLayer) {
                                                        return std::ranges::none_of(layerProperties,
                                                                                    [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                    });
    if (unsupportedLayerIt != requiredLayers.end())
    {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    // Get the required extensions.
    auto requiredExtensions = GetRequiredInstanceExtensions();

    // Check if the required extensions are supported by the Vulkan implementation.
    auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
        std::ranges::find_if(requiredExtensions,
                                [&extensionProperties](auto const &requiredExtension) {
                                    return std::ranges::none_of(extensionProperties,
                                                                [requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
                                });
    if (unsupportedPropertyIt != requiredExtensions.end())
    {
        throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }

    vk::InstanceCreateInfo createInfo{.pApplicationInfo = &appInfo,
                                      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
                                      .ppEnabledLayerNames = requiredLayers.data(),
                                      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
                                      .ppEnabledExtensionNames = requiredExtensions.data()};

    m_instance = vk::raii::Instance(m_context, createInfo);
};

std::vector<const char*> Renderer::GetRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (m_enableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Renderer::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                                      void *                                         pUserData)
{
  std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

  return vk::False;
}

}