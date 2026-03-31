#pragma once

#include "RenderPassManager.h"
#include "Scene/CullingSystem.h"

#include <vulkan/vulkan_raii.hpp>

namespace Yume {

class Renderer {
private:
    vk::raii::Device m_device = nullptr;
    vk::Queue m_graphicsQueue;
    vk::raii::CommandPool m_commandPool = nullptr;

    RenderPassManager m_renderPassManager;
    CullingSystem m_cullingSystem;

    // Current frame resources
    vk::raii::CommandBuffer m_commandBuffer = nullptr;
    vk::raii::Fence m_fence = nullptr;
    vk::raii::Semaphore m_imageAvailableSemaphore = nullptr;
    vk::raii::Semaphore m_renderFinishedSemaphore = nullptr;

public:
    Renderer(vk::raii::Device& dev, vk::Queue queue) : m_device(dev), m_graphicsQueue(queue) {
        // Create command pool
        // ...

        // Create synchronization objects
        // ...

        // Set up render passes
        SetupRenderPasses();
    }

    // No need for explicit destructor with RAII objects

    void SetCamera(Camera* camera) {
        m_cullingSystem.SetCamera(camera);
    }

    void Render(const std::vector<Entity*>& entities) {
        // Wait for previous frame to finish
        m_fence.wait(UINT64_MAX);
        m_fence.reset();

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

private:
    void SetupRenderPasses() {
        // Create geometry pass
        auto geometryPass = m_renderPassManager.AddRenderPass<GeometryPass>("GeometryPass", &m_cullingSystem);

        // Create lighting pass
        auto lightingPass = m_renderPassManager.AddRenderPass<LightingPass>("LightingPass", geometryPass);

        // Create post-process pass
        auto postProcessPass = m_renderPassManager.AddRenderPass<PostProcessPass>("PostProcessPass", lightingPass);

        // Add post-process effects
        // ...
    }
};
}