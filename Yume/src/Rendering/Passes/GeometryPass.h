#pragma once

#include "../Core/RenderPass.h"
#include "../Core/RenderTarget.h"
#include "Scene/CullingSystem.h"

#include <vulakan/vulkan_raii.hpp>

namespace Yume {

class GeometryPass : public RenderPass {
private:
    CullingSystem* m_cullingSystem;
    RenderTarget* m_gBuffer;

public:
    GeometryPass(const std::string& name, CullingSystem* culling)
        : RenderPass(name), m_cullingSystem(culling) {
        // Create G-buffer render target
        m_gBuffer = new RenderTarget(1920, 1080); // Example resolution
        SetRenderTarget(m_gBuffer);
    }

    ~GeometryPass() override {
        delete m_gBuffer;
    }

protected:
    void BeginPass(vk::raii::CommandBuffer& commandBuffer) override {
        // Begin rendering with dynamic rendering
        vk::RenderingInfoKHR renderingInfo;

        // Set up color attachment
        vk::RenderingAttachmentInfoKHR colorAttachment;
        colorAttachment.setImageView(m_gBuffer->GetColorImageView())
                       .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                       .setLoadOp(vk::AttachmentLoadOp::eClear)
                       .setStoreOp(vk::AttachmentStoreOp::eStore)
                       .setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));

        // Set up depth attachment
        vk::RenderingAttachmentInfoKHR depthAttachment;
        depthAttachment.setImageView(m_gBuffer->GetDepthImageView())
                       .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                       .setLoadOp(vk::AttachmentLoadOp::eClear)
                       .setStoreOp(vk::AttachmentStoreOp::eStore)
                       .setClearValue(vk::ClearDepthStencilValue(1.0f, 0));

        // Configure rendering info
        renderingInfo.setRenderArea(vk::Rect2D({0, 0}, {m_gBuffer->GetWidth(), m_gBuffer->GetHeight()}))
                     .setLayerCount(1)
                     .setColorAttachmentCount(1)
                     .setPColorAttachments(&colorAttachment)
                     .setPDepthAttachment(&depthAttachment);

        // Begin dynamic rendering
        commandBuffer.beginRendering(renderingInfo);
    }

    void Render(vk::raii::CommandBuffer& commandBuffer) override {
        // Get visible entities
        const auto& visibleEntities = m_cullingSystem->GetVisibleEntities();

        // Render each entity to G-buffer
        for (auto entity : visibleEntities) {
            auto meshComponent = entity->GetComponent<MeshComponent>();
            auto transformComponent = entity->GetComponent<TransformComponent>();

            if (meshComponent && transformComponent) {
                // Bind pipeline for G-buffer rendering
                // ...

                // Set model matrix
                // ...

                // Draw mesh
                // ...
            }
        }
    }

    void EndPass(vk::raii::CommandBuffer& commandBuffer) override {
        // End dynamic rendering
        commandBuffer.endRendering();
    }
};


}