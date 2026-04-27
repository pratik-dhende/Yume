#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <stb_image.h>
#include <random>
#include <ctime>

#include "Renderer.h"
#include "Rendering/Resources/Shader.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/Resources/Mesh.h"
#include "Services/ResourceManager/ResourceManager.h"
#include "Services/ResourceManager/ResourceHandle.h"

namespace Yume {

const std::vector<char const*> Renderer::s_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> Renderer::s_requiredDeviceExtensions = { vk::KHRSwapchainExtensionName, vk::KHRShaderDrawParametersExtensionName, vk::KHRSynchronization2ExtensionName };

Renderer::Renderer(const bool enableValidationLayer, GLFWwindow* window) : m_enableValidationLayers(enableValidationLayer), m_window(window) {
    SetupRenderPasses();
}

void Renderer::ShutDown() {
    m_logicalDevice.waitIdle();

    m_textureSampler = nullptr;
    m_textureImageView = nullptr;

    m_textureImageMemory = nullptr;
    m_textureImage = nullptr;

    m_computeDescriptorSets.clear();
    m_graphicsDescriptorSets.clear();
    m_descriptorPool = nullptr;

    m_shaderStorageBuffers.clear();
    m_shaderStorageBuffersMemory.clear();

    m_uniformBuffers.clear();
    m_uniformBuffersMemory.clear();
    m_uniformBuffersMapped.clear();

    m_indexBuffer = nullptr;
    m_indexBufferMemory = nullptr;

    m_vertexBuffer = nullptr;
    m_vertexBufferMemory = nullptr;

    m_presentCompleteSemaphores.clear();
    m_inFlightFences.clear();
    m_renderFinishedSemaphores.clear();

    m_commandBuffers.clear();
    m_commandPool = nullptr;

    m_computePipeline = nullptr;
    m_computePipelineLayout = nullptr;
    m_computeDescriptorSetLayout = nullptr;

    m_graphicsPipeline = nullptr;
    m_graphicsPipelineLayout = nullptr;
    m_graphicsDescriptorSetLayout = nullptr;

    m_depthImageView = nullptr;
    m_depthImageMemory = nullptr;
    m_depthImage = nullptr;
    m_depthFormat = vk::Format{};
    
    m_colorImageView = nullptr;
    m_colorImage = nullptr;
    m_colorImageMemory = nullptr;

    CleanupSwapChain();

    m_graphicsComputeQueue = nullptr;
    m_logicalDevice = nullptr;
    m_physicalDevice = nullptr;

    m_surface = nullptr;
    m_debugMessenger = nullptr;
    m_instance = nullptr;

    m_window = nullptr;
}

void Renderer::Render(const std::vector<Entity*>& entities) {
    // // Wait for previous frame to finish
    // auto fenceWaitResult = m_device.waitForFences(*m_fence, VK_TRUE, UINT64_MAX);
    // m_device.resetFences(*m_fence);

    // // Reset command buffer
    // m_commandBuffer.reset();

    // // Perform culling
    // m_cullingSystem.CullScene(entities);

    // // Record commands
    // vk::CommandBufferBeginInfo beginInfo;
    // m_commandBuffer.begin(beginInfo);

    // // Execute render passes
    // m_renderPassManager.Execute(m_commandBuffer);

    // m_commandBuffer.end();

    // // Submit command buffer
    // vk::SubmitInfo submitInfo;

    // // With vk::raii, we need to dereference the command buffer
    // vk::CommandBuffer rawCommandBuffer = *m_commandBuffer;
    // submitInfo.setCommandBufferCount(1);
    // submitInfo.setPCommandBuffers(&rawCommandBuffer);

    // // Set up wait and signal semaphores
    // vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    // // With vk::raii, we need to dereference the semaphores
    // vk::Semaphore rawImageAvailableSemaphore = *m_imageAvailableSemaphore;
    // vk::Semaphore rawRenderFinishedSemaphore = nullptr;

    // submitInfo.setWaitSemaphoreCount(1);
    // submitInfo.setPWaitSemaphores(&rawImageAvailableSemaphore);
    // submitInfo.setPWaitDstStageMask(waitStages);
    // submitInfo.setSignalSemaphoreCount(1);
    // submitInfo.setPSignalSemaphores(&rawRenderFinishedSemaphore);

    // // With vk::raii, we need to dereference the fence
    // vk::Fence rawFence = *m_fence;
    // auto queueSubmitResult = m_tmpGraphicsQueue.submit(1, &submitInfo, rawFence);
}

void Renderer::HandleWindowResize(const int width, const int height) {
    m_windowResized = true;
}

void Renderer::OnEvent(const Event& event) {
    if (event.Is<WindowResizeEvent>()) {
        auto windowResizeEvent = static_cast<const WindowResizeEvent&>(event);
        HandleWindowResize(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());
    } 
}

void Renderer::Init() {
    ServiceLocator::GetService<EventBus>().AddListener(this, static_cast<int>(EventCategory::Window));
    InitVulkan();
}

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties))
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Renderer::CopyBuffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & dstBuffer, vk::DeviceSize size) {
    auto copyCommandBuffer = BeginSingleTimeCommands();
    copyCommandBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
    EndSingleTimeCommands(copyCommandBuffer);
}

void Renderer::CreateVertexBuffer() {
    vk::DeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, m_vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, m_vertexBuffer, m_vertexBufferMemory);

    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
}

void Renderer::CreateIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, m_indices.data(), (size_t) bufferSize);
    stagingBufferMemory.unmapMemory();

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, m_indexBuffer, m_indexBufferMemory);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
}

void Renderer::CreateUniformBuffers() {
    m_uniformBuffers.clear();
    m_uniformBuffersMemory.clear();
    m_uniformBuffersMapped.clear();

    for (size_t i = 0; i < s_maxFramesInFlight; i++) {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
        vk::raii::Buffer buffer({});
        vk::raii::DeviceMemory bufferMemory({});
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMemory);
        m_uniformBuffers.emplace_back(std::move(buffer));
        m_uniformBuffersMemory.emplace_back(std::move(bufferMemory));
        m_uniformBuffersMapped.emplace_back( m_uniformBuffersMemory[i].mapMemory(0, bufferSize));
    }
}

void Renderer::UpdateUniformBuffer(const int frameIndex) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::transpose(rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    ubo.view = glm::transpose(lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    ubo.proj = glm::transpose(glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height), 0.1f, 10.0f));
    ubo.proj[1][1] *= -1;

    memcpy(m_uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

void Renderer::CreateGraphicsDescriptorPool() {
    std::array poolSize {
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, s_maxFramesInFlight),
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, s_maxFramesInFlight),
    };

    vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = s_maxFramesInFlight, .poolSizeCount = static_cast<uint32_t>(poolSize.size()), .pPoolSizes = poolSize.data() };

    m_descriptorPool = vk::raii::DescriptorPool(m_logicalDevice, poolInfo);
    m_graphicsDescriptorSets.clear();
}

void Renderer::CreateGraphicsDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> graphicsLayouts(s_maxFramesInFlight, *m_graphicsDescriptorSetLayout);
    vk::DescriptorSetAllocateInfo graphicsAllocInfo{ .descriptorPool = m_descriptorPool, .descriptorSetCount = static_cast<uint32_t>(graphicsLayouts.size()), .pSetLayouts = graphicsLayouts.data() };

    m_graphicsDescriptorSets = m_logicalDevice.allocateDescriptorSets(graphicsAllocInfo);

    for (size_t i = 0; i < s_maxFramesInFlight; i++) {
        vk::DescriptorBufferInfo bufferInfo{ .buffer = m_uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject) };
        vk::DescriptorImageInfo imageInfo{ .sampler = m_textureSampler, .imageView = m_textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

        std::array desciptorWrites{
            vk::WriteDescriptorSet{ .dstSet = m_graphicsDescriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo },
            vk::WriteDescriptorSet{ .dstSet = m_graphicsDescriptorSets[i], .dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .pImageInfo = &imageInfo }
        };

        m_logicalDevice.updateDescriptorSets(desciptorWrites, {});
    }
}

void Renderer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo{.size        = size,
                                    .usage       = usage,
                                    .sharingMode = vk::SharingMode::eExclusive};

    buffer = vk::raii::Buffer(m_logicalDevice, bufferInfo);

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryAllocateInfo{
    .allocationSize  = memRequirements.size,
    .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)};

    bufferMemory = vk::raii::DeviceMemory(m_logicalDevice, memoryAllocateInfo);
    buffer.bindMemory(bufferMemory, 0 );
}

void Renderer::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    SelectPhysicalDevice();
    m_msaaSamples = GetMaxUsableSampleCount();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateGraphicsDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    LoadModel();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateShaderStorageBuffers();
    CreateGraphicsDescriptorPool();
    CreateGraphicsDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void Renderer::CreateComputeDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> computeLayouts(s_maxFramesInFlight, *m_computeDescriptorSetLayout);
    vk::DescriptorSetAllocateInfo computeAllocInfo{ .descriptorPool = m_descriptorPool, .descriptorSetCount = static_cast<uint32_t>(computeLayouts.size()), .pSetLayouts = computeLayouts.data() };

    m_computeDescriptorSets = m_logicalDevice.allocateDescriptorSets(computeAllocInfo);

    for (size_t i = 0; i < s_maxFramesInFlight; i++) {
        vk::DescriptorBufferInfo bufferInfo{ .buffer = m_uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject) };        
        vk::DescriptorBufferInfo storageBufferInfoLastFrame(m_shaderStorageBuffers[(i - 1) % s_maxFramesInFlight], 0, sizeof(Particle) * s_particleCount);
        vk::DescriptorBufferInfo storageBufferInfoCurrentFrame(m_shaderStorageBuffers[i], 0, sizeof(Particle) * s_particleCount);

        std::array desciptorWrites{
            vk::WriteDescriptorSet{ .dstSet = m_computeDescriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo },
            vk::WriteDescriptorSet{ .dstSet = m_computeDescriptorSets[i], .dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eStorageBuffer, .pBufferInfo = &storageBufferInfoLastFrame },
            vk::WriteDescriptorSet{ .dstSet = m_computeDescriptorSets[i], .dstBinding = 2, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eStorageBuffer, .pBufferInfo = &storageBufferInfoCurrentFrame },
        };

        m_logicalDevice.updateDescriptorSets(desciptorWrites, {});
    }   
}

void Renderer::CreateComputeDescriptorSetLayout() {
    std::array computeBindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr),
        vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr),
    };
    vk::DescriptorSetLayoutCreateInfo computeLayoutInfo{.bindingCount = static_cast<uint32_t>(computeBindings.size()), .pBindings = computeBindings.data()};
    m_computeDescriptorSetLayout = vk::raii::DescriptorSetLayout(m_logicalDevice, computeLayoutInfo);
}

void Renderer::CreateComputeDescriptorPool() {
   std::array poolSize {
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, s_maxFramesInFlight),
        vk::DescriptorPoolSize( vk::DescriptorType::eStorageBuffer, s_maxFramesInFlight * 2),
    };

    vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = s_maxFramesInFlight, .poolSizeCount = static_cast<uint32_t>(poolSize.size()), .pPoolSizes = poolSize.data() };

    m_descriptorPool = vk::raii::DescriptorPool(m_logicalDevice, poolInfo);
    m_computeDescriptorSets.clear();
}

void Renderer::CreateComputePipeline() {
    constexpr const char* computeShaderEntryPoint = "compMain";

    auto shaderHandle = ServiceLocator::GetService<ResourceManager>().Load<Shader>("shader.slang");
    auto shaderModule = CreateShaderModule(shaderHandle->GetShaderBytecode());

    vk::PipelineShaderStageCreateInfo computeShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eCompute, .module = shaderModule, .pName = computeShaderEntryPoint };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &*m_computeDescriptorSetLayout};
    m_computePipelineLayout = vk::raii::PipelineLayout(m_logicalDevice, pipelineLayoutInfo);

    vk::ComputePipelineCreateInfo pipelineInfo{.stage = computeShaderStageInfo, .layout = *m_computePipelineLayout};
    m_computePipeline = vk::raii::Pipeline(m_logicalDevice, nullptr, pipelineInfo);
}

void Renderer::CreateShaderStorageBuffers() {
    m_shaderStorageBuffers.clear();
    m_shaderStorageBuffersMemory.clear();

    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    // Initial particle positions on a circle
    std::vector<Particle> particles(s_particleCount);
    for (auto& particle : particles) {
        float r = 0.25f * sqrtf(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
        float x = r * cosf(theta) * m_height / m_width;
        float y = r * sinf(theta);
        particle.position = glm::vec2(x, y);
        particle.velocity = normalize(glm::vec2(x,y)) * 0.00025f;
        particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
    }

    vk::DeviceSize bufferSize = sizeof(Particle) * s_particleCount;

    // Create a staging buffer used to upload data to the gpu
    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, particles.data(), (size_t)bufferSize);
    stagingBufferMemory.unmapMemory();

    // Copy initial particle data to all storage buffers
    for (size_t i = 0; i < s_maxFramesInFlight; i++) {
        vk::raii::Buffer tempShaderStorageBuffer({});
        vk::raii::DeviceMemory tempShaderStorageBufferMemory({});

        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, tempShaderStorageBuffer, tempShaderStorageBufferMemory);
        CopyBuffer(stagingBuffer, tempShaderStorageBuffer, bufferSize);

        m_shaderStorageBuffers.emplace_back(std::move(tempShaderStorageBuffer));
        m_shaderStorageBuffersMemory.emplace_back(std::move(tempShaderStorageBufferMemory));
    }
}

void Renderer::CreateColorResources() {
    vk::Format colorFormat = m_swapChainSurfaceFormat.format;

    CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, 1, m_msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,  vk::MemoryPropertyFlagBits::eDeviceLocal, m_colorImage, m_colorImageMemory);
    m_colorImageView = CreateImageView(m_colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
}

vk::SampleCountFlagBits Renderer::GetMaxUsableSampleCount() {
    vk::PhysicalDeviceProperties physicalDeviceProperties = m_physicalDevice.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
    if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

    return vk::SampleCountFlagBits::e1;
}

void Renderer::GenerateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t width, int32_t height, uint32_t mipLevels) {
    // Check if image format supports linear blit-ing
    vk::FormatProperties formatProperties = m_physicalDevice.getFormatProperties(imageFormat);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    auto commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier {
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite, 
        .dstAccessMask = vk::AccessFlagBits::eTransferRead,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal, 
        .newLayout = vk::ImageLayout::eTransferSrcOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored, 
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored, 
        .image = image};

    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    auto mipWidth = width;
    auto mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

        vk::ArrayWrapper1D<vk::Offset3D, 2> offsets, dstOffsets;
        offsets[0] = vk::Offset3D(0, 0, 0);
        offsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
        dstOffsets[0] = vk::Offset3D(0, 0, 0);
        dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
        vk::ImageBlit blit = { .srcSubresource = {}, .srcOffsets = offsets,
                            .dstSubresource =  {}, .dstOffsets = dstOffsets };
        blit.srcSubresource = vk::ImageSubresourceLayers( vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
        blit.dstSubresource = vk::ImageSubresourceLayers( vk::ImageAspectFlagBits::eColor, i, 0, 1);

        commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::LoadModel() {
    auto modelHandle = ServiceLocator::GetService<ResourceManager>().Load<Mesh>("viking_room.obj");

    auto meshVertices = modelHandle->GetVertices();
    auto meshIndices = modelHandle->GetIndices();
    auto meshVertexCount = static_cast<int>(meshVertices.size());
    auto meshIndexCount = static_cast<int>(meshIndices.size());

    m_vertices.resize(meshVertexCount);
    for(int i = 0; i < meshVertexCount; ++i) {
        m_vertices[i].pos = meshVertices[i].position;
        m_vertices[i].texCoord= meshVertices[i].uv;
        m_vertices[i].color = glm::vec3(1.0, 1.0, 1.0);
    }

    m_indices = meshIndices;
}

bool Renderer::HasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::Format Renderer::FindDepthFormat() {
    return FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
}

vk::Format Renderer::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (const auto format : candidates) {
        vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
        throw std::runtime_error("failed to find supported format!");
    }
    return vk::Format{};
}

void Renderer::CreateDepthResources() {
    m_depthFormat = FindDepthFormat();
    CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, 1, m_msaaSamples, m_depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage, m_depthImageMemory);
    m_depthImageView = CreateImageView(m_depthImage, m_depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
}

void Renderer::CreateTextureSampler() {
    vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
    vk::SamplerCreateInfo        samplerInfo{
                .magFilter        = vk::Filter::eLinear,
                .minFilter        = vk::Filter::eLinear,
                .mipmapMode       = vk::SamplerMipmapMode::eLinear,
                .addressModeU     = vk::SamplerAddressMode::eRepeat,
                .addressModeV     = vk::SamplerAddressMode::eRepeat,
                .addressModeW     = vk::SamplerAddressMode::eRepeat,
                .mipLodBias       = 0.0f,
                .anisotropyEnable = vk::True,
                .maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
                .compareEnable    = vk::False,
                .compareOp        = vk::CompareOp::eAlways,
                .minLod = 0,
                .maxLod = vk::LodClampNone};
    m_textureSampler = vk::raii::Sampler(m_logicalDevice, samplerInfo);
}

vk::raii::ImageView Renderer::CreateImageView(const vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
    vk::ImageViewCreateInfo viewInfo{ .image = image, .viewType = vk::ImageViewType::e2D, .format = format, .subresourceRange = { aspectFlags, 0, mipLevels, 0, 1 } };
    return vk::raii::ImageView(m_logicalDevice, viewInfo );
}

void Renderer::CreateTextureImageView() {
    m_textureImageView = CreateImageView(m_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, m_mipLevels);
}

void Renderer::CopyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height) {
    vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferImageCopy region{ .bufferOffset = 0, .bufferRowLength = 0, .bufferImageHeight = 0,
    .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, .imageOffset = {0, 0, 0}, .imageExtent = {width, height, 1} };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::TransitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
    auto commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{ .oldLayout = oldLayout, .newLayout = newLayout, .image = image, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1 } };
    
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask =  vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask =  vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

    EndSingleTimeCommands(commandBuffer);
}

vk::raii::CommandBuffer Renderer::BeginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = m_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    vk::raii::CommandBuffer commandBuffer = std::move(m_logicalDevice.allocateCommandBuffers(allocInfo).front());

    vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Renderer::EndSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
    m_graphicsComputeQueue.submit(submitInfo, nullptr);
    m_graphicsComputeQueue.waitIdle();
}

void Renderer::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& outImage, vk::raii::DeviceMemory& outImageMemory) {
    vk::ImageCreateInfo imageInfo{ .imageType = vk::ImageType::e2D, .format = format,
        .extent = {width, height, 1}, .mipLevels = mipLevels, .arrayLayers = 1,
        .samples = numSamples, .tiling = tiling,
        .usage = usage, .sharingMode = vk::SharingMode::eExclusive };

    outImage = vk::raii::Image(m_logicalDevice, imageInfo);

    vk::MemoryRequirements memRequirements = outImage.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties) };
    outImageMemory = vk::raii::DeviceMemory(m_logicalDevice, allocInfo);
    outImage.bindMemory(outImageMemory, 0);
}

void Renderer::CreateTextureImage() {
    auto textureHandle = ServiceLocator::GetService<ResourceManager>().Load<Texture>("viking_room.png");

    auto width = static_cast<uint32_t>(textureHandle->GetWidth());
    auto height = static_cast<uint32_t>(textureHandle->GetHeight());

    m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    vk::DeviceSize imageSize = width * height * 4;

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});

    CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, textureHandle->GetPixels(), imageSize);
    stagingBufferMemory.unmapMemory();

    CreateImage(width, height, m_mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_textureImage, m_textureImageMemory);

    TransitionImageLayout(m_textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_mipLevels);
    CopyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    GenerateMipmaps(m_textureImage, vk::Format::eR8G8B8A8Srgb, width, height, m_mipLevels);
}

void Renderer::CreateGraphicsDescriptorSetLayout() {
    std::array graphicsBindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr),
    };
    vk::DescriptorSetLayoutCreateInfo graphicsLayoutInfo{.bindingCount = static_cast<uint32_t>(graphicsBindings.size()), .pBindings = graphicsBindings.data()};
    m_graphicsDescriptorSetLayout = vk::raii::DescriptorSetLayout(m_logicalDevice, graphicsLayoutInfo);
}

void Renderer::TransitionImageLayout(
	    vk::Image               image,
	    vk::ImageLayout         oldLayout,
	    vk::ImageLayout         newLayout,
	    vk::AccessFlags2        srcAccessMask,
	    vk::AccessFlags2        dstAccessMask,
	    vk::Flags<vk::PipelineStageFlagBits2> srcStageMask,
	    vk::Flags<vk::PipelineStageFlagBits2> dstStageMask,
        vk::ImageAspectFlags    imageAspectFlags)
{
		vk::ImageMemoryBarrier2 barrier = {
		    .srcStageMask        = srcStageMask,
		    .srcAccessMask       = srcAccessMask,
		    .dstStageMask        = dstStageMask,
		    .dstAccessMask       = dstAccessMask,
		    .oldLayout           = oldLayout,
		    .newLayout           = newLayout,
		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .image               = image,
		    .subresourceRange    = {
		           .aspectMask     = imageAspectFlags,
		           .baseMipLevel   = 0,
		           .levelCount     = 1,
		           .baseArrayLayer = 0,
		           .layerCount     = 1}};
		vk::DependencyInfo dependencyInfo = {
		    .dependencyFlags         = {},
		    .imageMemoryBarrierCount = 1,
		    .pImageMemoryBarriers    = &barrier};
    m_commandBuffers[m_frameIndex].pipelineBarrier2(dependencyInfo);
}

void Renderer::CreateSyncObjects() {
    assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

    for (size_t i = 0; i < static_cast<int>(m_swapChainImages.size()); i++)
    {
        m_renderFinishedSemaphores.emplace_back(m_logicalDevice, vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < s_maxFramesInFlight; i++)
    {
        m_presentCompleteSemaphores.emplace_back(m_logicalDevice, vk::SemaphoreCreateInfo());
        m_inFlightFences.emplace_back(m_logicalDevice, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void Renderer::CleanupSwapChain() {
    m_swapChainImages.clear();
    m_swapChainImageViews.clear();
    m_swapChain = nullptr;
}

void Renderer::HandleResize() {
    m_logicalDevice.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateColorResources();
    CreateDepthResources();
}

void Renderer::DrawFrame() {
    auto fenceResult = m_logicalDevice.waitForFences(*m_inFlightFences[m_frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence!");
    }

    auto [result, imageIndex] = m_swapChain.acquireNextImage(UINT64_MAX, *m_presentCompleteSemaphores[m_frameIndex], nullptr); // Signal to queue to start 
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        HandleResize();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    UpdateUniformBuffer(m_frameIndex);

    m_logicalDevice.resetFences(*m_inFlightFences[m_frameIndex]);

    m_commandBuffers[m_frameIndex].reset();
    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
    const vk::SubmitInfo submitInfo{.waitSemaphoreCount   = 1,
                                  .pWaitSemaphores      = &*m_presentCompleteSemaphores[m_frameIndex], // Wait for presentation engine to get the framebuffer
                                  .pWaitDstStageMask    = &waitDestinationStageMask,
                                  .commandBufferCount   = 1,
                                  .pCommandBuffers      = &*m_commandBuffers[m_frameIndex],
                                  .signalSemaphoreCount = 1,
                                  .pSignalSemaphores    = &*m_renderFinishedSemaphores[imageIndex]}; // Signal presentation engine for rendering completion

    m_graphicsComputeQueue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
                                            .pWaitSemaphores    = &*m_renderFinishedSemaphores[imageIndex],
                                            .swapchainCount     = 1,
                                            .pSwapchains        = &*m_swapChain,
                                            .pImageIndices      = &imageIndex};

    result = m_graphicsComputeQueue.presentKHR(presentInfoKHR);
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_windowResized)
    {
        HandleResize();
    }
    else
    {
        // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
        assert(result == vk::Result::eSuccess);
    }

    switch (result)
    {
        case vk::Result::eSuccess:
            break;
        case vk::Result::eSuboptimalKHR:
            std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
            break;
        default:
            break;        // an unexpected result is returned!
    }

    m_frameIndex = (m_frameIndex + 1) % s_maxFramesInFlight;
}

void Renderer::RecordCommandBuffer(const uint32_t imageIndex) {
    auto& commandBuffer = m_commandBuffers[m_frameIndex];

    commandBuffer.begin({});

    // Transition the image layout for rendering
    // Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
    TransitionImageLayout(
        m_swapChainImages[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},                                                        // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
        vk::ImageAspectFlagBits::eColor
    );

    TransitionImageLayout(
        *m_colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor);

    TransitionImageLayout(
        *m_depthImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth
    );

    // Set up the color attachment
    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderingAttachmentInfo colorAttachmentInfo = {
		    .imageView   = m_colorImageView,
		    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .resolveMode = vk::ResolveModeFlagBits::eAverage,
            .resolveImageView = m_swapChainImageViews[imageIndex],
            .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		    .loadOp      = vk::AttachmentLoadOp::eClear,
		    .storeOp     = vk::AttachmentStoreOp::eStore,
		    .clearValue  = clearColor};

    vk::RenderingAttachmentInfo depthAttachmentInfo = {
            .imageView   = m_depthImageView,
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eDontCare,
            .clearValue  = clearDepth};

    // Set up the rendering info
    vk::RenderingInfo renderingInfo = {
		    .renderArea           = {.offset = {0, 0}, .extent = m_swapChainExtent},
		    .layerCount           = 1,
		    .colorAttachmentCount = 1,
		    .pColorAttachments    = &colorAttachmentInfo,
            .pDepthAttachment     = &depthAttachmentInfo};

    // Begin rendering
    commandBuffer.beginRendering(renderingInfo);

    // Rendering commands will go here
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_swapChainExtent.width), static_cast<float>(m_swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swapChainExtent));

    commandBuffer.bindVertexBuffers(0, *m_vertexBuffer, {0});
    commandBuffer.bindIndexBuffer( *m_indexBuffer, 0, vk::IndexType::eUint32 );

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphicsPipelineLayout, 0, *m_graphicsDescriptorSets[m_frameIndex], nullptr);
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);;

    // End rendering
    commandBuffer.endRendering();

    // Transition the image layout for presentation
    TransitionImageLayout(
        m_swapChainImages[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    commandBuffer.end();

}

void Renderer::CreateCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = m_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = s_maxFramesInFlight };

    m_commandBuffers = std::move(vk::raii::CommandBuffers(m_logicalDevice, allocInfo));
}

void Renderer::CreateCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                       .queueFamilyIndex = m_graphicsComputeQueueIndex};

    m_commandPool = vk::raii::CommandPool(m_logicalDevice, poolInfo);
}

void Renderer::CreateGraphicsPipeline() {
    // Programmable Stages
    constexpr const char* vertexShaderEntryPoint = "vertMain";
    constexpr const char* fragmentShaderEntryPoint = "fragMain";

    auto shaderHandle = ServiceLocator::GetService<ResourceManager>().Load<Shader>("shader.slang");
    auto shaderModule = CreateShaderModule(shaderHandle->GetShaderBytecode());

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule,  .pName = vertexShaderEntryPoint };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = fragmentShaderEntryPoint };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Fixed functions
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{.vertexBindingDescriptionCount   = 1,
                                                           .pVertexBindingDescriptions      = &bindingDescription,
                                                           .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                                                           .pVertexAttributeDescriptions    = attributeDescriptions.data()};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
    vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

    vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable        = vk::False,
                                                        .rasterizerDiscardEnable = vk::False,
                                                        .polygonMode             = vk::PolygonMode::eFill,
                                                        .cullMode                = vk::CullModeFlagBits::eBack,
                                                        .frontFace               = vk::FrontFace::eCounterClockwise,
                                                        .depthBiasEnable         = vk::False,
                                                        .lineWidth               = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = m_msaaSamples, .sampleShadingEnable = vk::True, .minSampleShading = 0.2f}; // min fraction for sample shading; closer to one is smoother


    vk::PipelineColorBlendAttachmentState colorBlendAttachment{ .blendEnable = vk::False,
                                                                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &*m_graphicsDescriptorSetLayout, .pushConstantRangeCount = 0};
    m_graphicsPipelineLayout = vk::raii::PipelineLayout(m_logicalDevice, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &m_swapChainSurfaceFormat.format, .depthAttachmentFormat = m_depthFormat };

    vk::PipelineDepthStencilStateCreateInfo depthStencil{
    .depthTestEnable       = vk::True,
    .depthWriteEnable      = vk::True,
    .depthCompareOp        = vk::CompareOp::eLess,
    .depthBoundsTestEnable = vk::False,
    .stencilTestEnable     = vk::False};

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
    {   .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = m_graphicsPipelineLayout,
        .renderPass          = nullptr,},
        pipelineRenderingCreateInfo};

    m_graphicsPipeline = vk::raii::Pipeline(m_logicalDevice, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());   
}

vk::raii::ShaderModule Renderer::CreateShaderModule(ShaderBlob* bytecode) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = bytecode->getBufferSize();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode->getBufferPointer());
    return m_logicalDevice.createShaderModule(createInfo);
}

void Renderer::CreateSurface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*m_instance, m_window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
}

void Renderer::CreateSwapChainImageViews() {
    assert(m_swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = m_swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    for (auto &image : m_swapChainImages)
    {
        imageViewCreateInfo.image = image;
        m_swapChainImageViews.emplace_back(m_logicalDevice, imageViewCreateInfo);
    }
}

void Renderer::CreateSwapChain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);
    m_swapChainExtent = ChooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> availableFormats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
    m_swapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR(*m_surface);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface          = *m_surface,
                                                   .minImageCount    = minImageCount,
                                                   .imageFormat      = m_swapChainSurfaceFormat.format,
                                                   .imageColorSpace  = m_swapChainSurfaceFormat.colorSpace,
                                                   .imageExtent      = m_swapChainExtent,
                                                   .imageArrayLayers = 1,
                                                   .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
                                                   .imageSharingMode = vk::SharingMode::eExclusive,
                                                   .preTransform     = surfaceCapabilities.currentTransform,
                                                   .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                   .presentMode      = presentMode,
                                                   .clipped          = true};

    swapChainCreateInfo.oldSwapchain = nullptr;

    m_swapChain       = vk::raii::SwapchainKHR( m_logicalDevice, swapChainCreateInfo );
    m_swapChainImages = m_swapChain.getImages();
};

uint32_t Renderer::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
    {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}

vk::SurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR Renderer::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes) {
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
               vk::PresentModeKHR::eMailbox :
               vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    glfwGetFramebufferSize(m_window, &m_width, &m_height);

    return {
        std::clamp<uint32_t>(m_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(m_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

void Renderer::CreateLogicalDevice() {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports both graphics and present
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eCompute)) && 
            m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface))
        {
            // found a queue family that supports both graphics and present
            m_graphicsComputeQueueIndex = qfpIndex;
            break;
        }
    }
    if (m_graphicsComputeQueueIndex == ~0)
    {
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }
    
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo { .queueFamilyIndex = m_graphicsComputeQueueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };

    // Create a chain of feature structures
    vk::StructureChain<vk::PhysicalDeviceFeatures2, 
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features, 
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> 
    featureChain = {
        {.features = { .sampleRateShading = true, .samplerAnisotropy = true} },                 // vk::PhysicalDeviceFeatures2
        {.shaderDrawParameters = true},                              // vk::PhysicalDeviceVulkan11Features
        {.synchronization2 = true, .dynamicRendering = true},        // vk::PhysicalDeviceVulkan13Features
        {.extendedDynamicState = true}                               // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    };

    auto surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR( *m_surface );
    std::vector<vk::SurfaceFormatKHR> availableFormats = m_physicalDevice.getSurfaceFormatsKHR( *m_surface );
    std::vector<vk::PresentModeKHR> availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR( *m_surface );

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(s_requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = s_requiredDeviceExtensions.data()
    };

    m_logicalDevice = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
    m_graphicsComputeQueue = vk::raii::Queue( m_logicalDevice, m_graphicsComputeQueueIndex, 0 );
}

void Renderer::SelectPhysicalDevice() {
    std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
    auto const selectedDeviceIter = std::ranges::find_if( physicalDevices, [&]( auto const & physicalDevice ) { return IsDeviceSuitable( physicalDevice ); } );
    if ( selectedDeviceIter == physicalDevices.end() )
    {
        throw std::runtime_error( "failed to find a suitable GPU!" );
    }
    m_physicalDevice = *selectedDeviceIter;
}

bool Renderer::IsDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) {
    // Check if the physicalDevice supports the Vulkan 1.3 API version
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    // Check if any of the queue families support graphics operations
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphicsAndCompute = std::ranges::any_of( queueFamilyProperties, []( auto const & qfp ) { return !!( (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && (qfp.queueFlags & vk::QueueFlagBits::eCompute)); } );

    // Check if all required physicalDevice extensions are available
    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions =
    std::ranges::all_of( s_requiredDeviceExtensions,
                            [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                            {
                            return std::ranges::any_of( availableDeviceExtensions,
                                                        [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                        { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                            } );

    // Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
    auto features =
    physicalDevice
        .template getFeatures2<vk::PhysicalDeviceFeatures2, 
                               vk::PhysicalDeviceVulkan11Features, 
                               vk::PhysicalDeviceVulkan13Features, 
                               vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                    features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
                                    features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                    features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                    features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                                    features.template get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading;

    // Return true if the physicalDevice meets all the criteria
    return supportsVulkan1_3 && supportsGraphicsAndCompute && supportsAllRequiredExtensions && supportsRequiredFeatures;
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
}

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