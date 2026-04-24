#include <iostream>
#include <map>
#include <vector>
#include <cassert>

#include "Renderer.h"
#include "Rendering/Resources/Shader.h"
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

    m_presentCompleteSemaphores.clear();
    m_inFlightFences.clear();
    m_renderFinishedSemaphores.clear();

    m_commandBuffers.clear();
    m_commandPool = nullptr;

    m_graphicsPipeline = nullptr;
    m_pipelineLayout = nullptr;

    CleanupSwapChain();

    m_graphicsQueue = nullptr;
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

void Renderer::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void Renderer::TransitionImageLayout(
	    uint32_t                imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask)
{
		vk::ImageMemoryBarrier2 barrier = {
		    .srcStageMask        = src_stage_mask,
		    .srcAccessMask       = src_access_mask,
		    .dstStageMask        = dst_stage_mask,
		    .dstAccessMask       = dst_access_mask,
		    .oldLayout           = old_layout,
		    .newLayout           = new_layout,
		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .image               = m_swapChainImages[imageIndex],
		    .subresourceRange    = {
		           .aspectMask     = vk::ImageAspectFlagBits::eColor,
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

void Renderer::RecreateSwapChain() {
    m_logicalDevice.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
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
        RecreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }

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

    m_graphicsQueue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
                                            .pWaitSemaphores    = &*m_renderFinishedSemaphores[imageIndex],
                                            .swapchainCount     = 1,
                                            .pSwapchains        = &*m_swapChain,
                                            .pImageIndices      = &imageIndex};

    result = m_graphicsQueue.presentKHR(presentInfoKHR);
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_windowResized)
    {
        RecreateSwapChain();
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
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},                                                        // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
    );

    // Set up the color attachment
    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
		    .imageView   = m_swapChainImageViews[imageIndex],
		    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		    .loadOp      = vk::AttachmentLoadOp::eClear,
		    .storeOp     = vk::AttachmentStoreOp::eStore,
		    .clearValue  = clearColor};

    // Set up the rendering info
    vk::RenderingInfo renderingInfo = {
		    .renderArea           = {.offset = {0, 0}, .extent = m_swapChainExtent},
		    .layerCount           = 1,
		    .colorAttachmentCount = 1,
		    .pColorAttachments    = &attachmentInfo};

    // Begin rendering
    commandBuffer.beginRendering(renderingInfo);

    // Rendering commands will go here
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_swapChainExtent.width), static_cast<float>(m_swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swapChainExtent));

    commandBuffer.draw(3, 1, 0, 0);

    // End rendering
    commandBuffer.endRendering();

    // Transition the image layout for presentation
    TransitionImageLayout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    commandBuffer.end();

}

void Renderer::CreateCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = m_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = s_maxFramesInFlight };

    m_commandBuffers = std::move(vk::raii::CommandBuffers(m_logicalDevice, allocInfo));
}

void Renderer::CreateCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                       .queueFamilyIndex = 0};

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
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
    vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

    vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable        = vk::False,
                                                        .rasterizerDiscardEnable = vk::False,
                                                        .polygonMode             = vk::PolygonMode::eFill,
                                                        .cullMode                = vk::CullModeFlagBits::eBack,
                                                        .frontFace               = vk::FrontFace::eClockwise,
                                                        .depthBiasEnable         = vk::False,
                                                        .lineWidth               = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{ .blendEnable = vk::False,
                                                                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};
    m_pipelineLayout = vk::raii::PipelineLayout(m_logicalDevice, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &m_swapChainSurfaceFormat.format };

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
    {.stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = m_pipelineLayout,
        .renderPass          = nullptr},
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

void Renderer::CreateImageViews() {
    assert(m_swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType         = vk::ImageViewType::e2D,
                                                 .format           = m_swapChainSurfaceFormat.format,
                                                 .subresourceRange = imageViewCreateInfo.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1} };

    imageViewCreateInfo.components = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

    for (auto &image : m_swapChainImages)
    {
        imageViewCreateInfo.image = image;
        m_swapChainImageViews.emplace_back( m_logicalDevice, imageViewCreateInfo );
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

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

void Renderer::CreateLogicalDevice() {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports both graphics and present
    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface))
        {
            // found a queue family that supports both graphics and present
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0)
    {
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }
    
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo { .queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };

    // Create a chain of feature structures
    vk::StructureChain<vk::PhysicalDeviceFeatures2, 
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features, 
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> 
    featureChain = {
        {},                                                          // vk::PhysicalDeviceFeatures2
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
    m_graphicsQueue = vk::raii::Queue( m_logicalDevice, queueIndex, 0 );
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
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

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
                                    features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2;

    // Return true if the physicalDevice meets all the criteria
    return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
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