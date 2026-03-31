#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

namespace Yume {

// TODO: Currently only supports texture resources. Extend to support other resources.
class RenderGraph{
private:
    struct Pass {
        std::string name;                     
        std::vector<std::string> inputs;      
        std::vector<std::string> outputs;     
        std::function<void(vk::raii::CommandBuffer&)> executeFunc;  
    };
                  
    struct ImageResource {
        vk::Format format;                    
        vk::Extent2D extent;                  
        vk::ImageUsageFlags usage;            
        vk::ImageLayout initialLayout;        
        vk::ImageLayout finalLayout;          

        vk::raii::Image image = nullptr;     
        vk::raii::DeviceMemory memory = nullptr;  
        vk::raii::ImageView view = nullptr;
        
        std::string name;
    };

public:
    explicit RenderGraph(vk::raii::Device& device) : m_device(device) {}

    void AddImageResource(const std::string& name, vk::Format format, vk::Extent2D extent, vk::ImageUsageFlags usage, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) {
        ImageResource imageResource;
        imageResource.name = name;
        imageResource.format = format;
        imageResource.extent = extent;
        imageResource.usage = usage;
        imageResource.initialLayout = initialLayout;
        imageResource.finalLayout = finalLayout;

        m_imageResources[name] = std::move(imageResource);
    }

    void AddPass(const std::string& name, const std::vector<std::string>& inputs, const std::vector<std::string>& outputs, std::function<void(vk::raii::CommandBuffer&)> executeFunc) {
        Pass pass;
        pass.name = name;                        
        pass.inputs = inputs;                    
        pass.outputs = outputs;                  
        pass.executeFunc = executeFunc;          

        m_passes.push_back(pass);                  
    }

    void Compile() {
        // Dependency Graph Construction
        // Build bidirectional dependency relationships between passes
        std::vector<std::vector<size_t>> dependencies(m_passes.size());
        std::vector<std::vector<size_t>> dependents(m_passes.size());

        // Track which pass produces each resource (write-after-write dependencies)
        std::unordered_map<std::string, size_t> latestResourceWriters;
        std::unordered_map<std::string, size_t> latestResourceReaders;

        // Dependency Discovery Through Resource Usage Analysis
        // Analyze each pass to determine data flow relationships
        for (size_t i = 0; i < m_passes.size(); ++i) {
            const auto& pass = m_passes[i];

            for (const auto& input : pass.inputs) {
                latestResourceReaders[input] = i;

                auto it = latestResourceWriters.find(input);
                if (it != latestResourceWriters.end()) {
                    // RAW
                    dependencies[i].push_back(it->second);      
                    dependents[it->second].push_back(i);         
                }
            }

            for (const auto& output : pass.outputs) {
                auto it = latestResourceReaders.find(output);
                if (it != latestResourceReaders.end()) {
                    // WAR
                    dependencies[i].push_back(it->second);
                    dependents[it->second].push_back(i);
                }
                
                it = latestResourceWriters.find(output);
                if (it != latestResourceWriters.end()) {
                    // WAW
                    dependencies[i].push_back(it->second);
                    dependents[it->second].push_back(i);
                }
                latestResourceWriters[output] = i;  
            }
        }

        // Topological Sort for Optimal Execution Order
        // Use depth-first search to compute valid execution sequence while detecting cycles
        std::vector<bool> visited(m_passes.size(), false);       // Track completed nodes
        std::vector<bool> inStack(m_passes.size(), false);       // Track current recursion path

        std::function<void(size_t)> visit = [&](size_t node) {
            if (inStack[node]) {
                // Cycle detection - circular dependency found
                throw std::runtime_error("Cycle detected in rendergraph");
            }

            if (visited[node]) {
                return;  // Already processed this node and its dependencies
            }

            inStack[node] = true;   // Mark as currently being processed

            // Recursively process all dependent passes first (post-order traversal)
            for (auto dependent : dependents[node]) {
                visit(dependent);
            }

            inStack[node] = false;  // Remove from current path
            visited[node] = true;   // Mark as completely processed
            m_executionOrder.push_back(node);  // Add to execution sequence
        };

        // Process all unvisited nodes to handle disconnected graph components
        for (size_t i = 0; i < m_passes.size(); ++i) {
            if (!visited[i]) {
                visit(i);
            }
        }

        // Generate semaphores for all dependencies identified during analysis
        for (size_t consumer = 0; consumer < m_passes.size(); ++consumer) {
            for (auto producer : dependencies[consumer]) {
                m_semaphores.emplace_back(m_device.createSemaphore({}));
                m_semaphoreSignalWaitPairs.emplace_back(producer, consumer);    // (producer, consumer) pair
            }
        }

        // Physical Resource Allocation and Creation
        // Transform resource descriptions into actual GPU objects
        for (auto& [name, imageResource] : m_imageResources) {
            vk::ImageCreateInfo imageInfo;
            imageInfo.setImageType(vk::ImageType::e2D)                    
                     .setFormat(imageResource.format)                          
                     .setExtent({imageResource.extent.width, imageResource.extent.height, 1})  
                     .setMipLevels(1)                                      
                     .setArrayLayers(1)                                    
                     .setSamples(vk::SampleCountFlagBits::e1)              
                     .setTiling(vk::ImageTiling::eOptimal)                 
                     .setUsage(imageResource.usage)                             
                     .setSharingMode(vk::SharingMode::eExclusive)          
                     .setInitialLayout(vk::ImageLayout::eUndefined);       

            imageResource.image = m_device.createImage(imageInfo);               

            // Allocate backing memory for the image
            vk::MemoryRequirements memRequirements = imageResource.image.getMemoryRequirements();

            vk::MemoryAllocateInfo allocInfo;
            allocInfo.setAllocationSize(memRequirements.size)             
                     .setMemoryTypeIndex(FindMemoryType(memRequirements.memoryTypeBits,
                                                       vk::MemoryPropertyFlagBits::eDeviceLocal));  

            imageResource.memory = m_device.allocateMemory(allocInfo);           
            imageResource.image.bindMemory(*imageResource.memory, 0);               

            // Create image view for shader access
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.setImage(*imageResource.image)                            // Reference the created image
                    .setViewType(vk::ImageViewType::e2D)                   // 2D view type
                    .setFormat(imageResource.format)                            // Match image format
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});  // Full image access

            imageResource.view = m_device.createImageView(viewInfo);             // Create shader-accessible view
        }
    }

    ImageResource* GetResource(const std::string& name) {
        auto it = m_imageResources.find(name);
        return (it != m_imageResources.end()) ? &it->second : nullptr;
    }

    // Rendergraph execution engine - coordinates pass execution with automatic synchronization
    // This method transforms the compiled rendergraph into actual GPU work
    void Execute(vk::raii::CommandBuffer& commandBuffer, vk::Queue queue) {
        // Execution state management for dynamic synchronization
        std::vector<vk::CommandBuffer> cmdBuffers;           
        std::vector<vk::Semaphore> waitSemaphores;           
        std::vector<vk::PipelineStageFlags> waitStages;      
        std::vector<vk::Semaphore> signalSemaphores;         

        // Ordered Pass Execution with Automatic Dependency Management
        // Execute each pass in the computed dependency-safe order
        for (auto passIdx : m_executionOrder) {
            const auto& pass = m_passes[passIdx];

            // Synchronization Setup - Collect Dependencies for Current Pass
            // Determine what this pass must wait for before executing
            waitSemaphores.clear();
            waitStages.clear();

            for (size_t i = 0; i < m_semaphoreSignalWaitPairs.size(); ++i) {
                if (m_semaphoreSignalWaitPairs[i].second == passIdx) {
                    waitSemaphores.push_back(*m_semaphores[i]);                           
                    waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
                }
            }

            signalSemaphores.clear();
            for (size_t i = 0; i < m_semaphoreSignalWaitPairs.size(); ++i) {
                if (m_semaphoreSignalWaitPairs[i].first == passIdx) {
                    signalSemaphores.push_back(*m_semaphores[i]);                         
                }
            }

            // Command Buffer Preparation and Resource Layout Transitions
            // Set up command recording and transition resources to appropriate layouts
            commandBuffer.begin({});                                                   

            // Transition input resources to shader-readable layouts
            for (const auto& input : pass.inputs) {
                auto& inputImageResource = m_imageResources[input];

                vk::ImageMemoryBarrier barrier;
                barrier.setOldLayout(inputImageResource.initialLayout)                           
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)          
                       .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                // No queue family transfer
                       .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setImage(*inputImageResource.image)                                      
                       .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})  
                       .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)             // Previous write access
                       .setDstAccessMask(vk::AccessFlagBits::eShaderRead);             // Required read access

                // Insert pipeline barrier for safe layout transition
                commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllCommands,                           // Wait for all previous work
                    vk::PipelineStageFlagBits::eFragmentShader,                        // Enable fragment shader access
                    vk::DependencyFlagBits::eByRegion,                                 // Region-local dependency
                    {}, {}, {barrier}                                                  // Image barrier only
                );
            }

            // Transition output resources to render target layouts
            for (const auto& output : pass.outputs) {
                auto& outputImageResource = m_imageResources[output];

                vk::ImageMemoryBarrier barrier;
                barrier.setOldLayout(outputImageResource.initialLayout)                           
                       .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)         
                       .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setImage(*outputImageResource.image)
                       .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                       .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)              // Previous read access
                       .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);   // Required write access

                // Insert barrier for safe transition to writable state
                commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllCommands,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,                
                    vk::DependencyFlagBits::eByRegion,
                    {}, {}, {barrier}
                );
            }

            // Pass Execution - Execute the Actual Rendering Logic
            // Call the user-provided rendering function with prepared command buffer
            pass.executeFunc(commandBuffer);                                           

            // Final Layout Transitions - Prepare Resources for Subsequent Use
            // Transition output resources to their final required layouts
            for (const auto& output : pass.outputs) {
                auto& outputImageResource = m_imageResources[output];

                vk::ImageMemoryBarrier barrier;
                barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)         
                       .setNewLayout(outputImageResource.finalLayout)                             
                       .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setImage(*outputImageResource.image)
                       .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                       .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)    // Previous write operations
                       .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);             // Enable subsequent reads

                // Insert final barrier for layout transition
                commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,                 // After color writes complete
                    vk::PipelineStageFlagBits::eAllCommands,                           // Before any subsequent work
                    vk::DependencyFlagBits::eByRegion,
                    {}, {}, {barrier}
                );
            }

            // Command Submission with Synchronization
            // Submit command buffer with appropriate dependency and signaling semaphores
            commandBuffer.end();                                                       

            vk::SubmitInfo submitInfo;
            submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))      
                      .setPWaitSemaphores(waitSemaphores.data())                                 
                      .setPWaitDstStageMask(waitStages.data())                                   
                      .setCommandBufferCount(1)                                                  
                      .setPCommandBuffers(&*commandBuffer)                                      
                      .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))  
                      .setPSignalSemaphores(signalSemaphores.data());                           

            auto result = queue.submit(1, &submitInfo, nullptr);                                              
        }
    }

private:
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        // Implementation to find suitable memory type
        // ...
        return 0; // Placeholder
    }

    // Comprehensive image layout transition implementation
    // This function demonstrates proper synchronization and layout management in Vulkan
    void TransitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {

        // This barrier will coordinate memory access and layout transitions
        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(oldLayout)                                        
            .setNewLayout(newLayout)                                        
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                // No queue family ownership transfer
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                // Same queue family throughout
            .setImage(image)                                                
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}); // Full color image range

        // Initialize pipeline stage tracking for synchronization timing
        vk::PipelineStageFlags sourceStage;      
        vk::PipelineStageFlags destinationStage; 

        // Configure synchronization for undefined-to-transfer layout transitions
        // This pattern is common when preparing images for data uploads
        if (oldLayout == vk::ImageLayout::eUndefined &&
            newLayout == vk::ImageLayout::eTransferDstOptimal) {

            // Configure memory access permissions for upload preparation
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)                // No previous access to synchronize
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);         // Enable transfer write operations

            // Set pipeline stage synchronization points for upload workflow
            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;               // No previous work to wait for
            destinationStage = vk::PipelineStageFlagBits::eTransfer;           // Transfer operations can proceed

        // Configure synchronization for transfer-to-shader layout transitions
        // This pattern prepares uploaded images for shader sampling
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {

            // Configure memory access transition from writing to reading
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)       // Previous transfer writes must complete
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);         // Enable shader read access

            // Set pipeline stage synchronization for shader usage workflow
            sourceStage = vk::PipelineStageFlagBits::eTransfer;                // Transfer operations must complete
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;     // Fragment shaders can access

        } else {
            // Handle unsupported transition combinations
            // Production code would include additional common transition patterns
            throw std::invalid_argument("Unsupported layout transition!");
        }

        // Execute the pipeline barrier with configured synchronization
        // This commits the layout transition and memory synchronization to the command buffer
        commandBuffer.pipelineBarrier(
            sourceStage,                                                       // Wait for these operations to complete
            destinationStage,                                                  // Before allowing these operations to begin
            vk::DependencyFlagBits::eByRegion,                                 // Enable region-local optimizations
            {}, {}, {barrier}                                                       // Apply our image memory barrier
        );
    }

    void RenderFrame(vk::raii::Device& device, vk::Queue graphicsQueue, vk::Queue presentQueue) {
        // Synchronize with previous frame completion
        // Prevent CPU from submitting work faster than GPU can process it
        vk::Result waitFenceResult = device.waitForFences(vk::ArrayProxy<const vk::Fence>{*m_inFlightFence}, true, UINT64_MAX);

        // Reset fence for this frame's completion tracking
        // Prepare the fence to signal when this frame's GPU work completes
        device.resetFences(vk::ArrayProxy<const vk::Fence>{*m_inFlightFence});

        // Acquire next available image from the swapchain
        // This operation coordinates with the presentation engine and display system
        auto imageIndex = m_swapchain.acquireNextImage(UINT64_MAX, *m_imageAvailableSemaphore).value;

        // Configure GPU work submission with comprehensive synchronization
        // This submission coordinates image availability, rendering, and presentation readiness
        vk::SubmitInfo submitInfo;
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.setWaitSemaphoreCount(1)                                    // Wait for one semaphore before execution
                .setPWaitSemaphores(&*m_imageAvailableSemaphore)               // Don't start until image is available
                .setPWaitDstStageMask(waitStages)                            // Specifically wait before color output
                .setCommandBufferCount(1)                                    // Submit one command buffer
                .setPCommandBuffers(&*m_commandBuffer)                        // The recorded rendering commands
                .setSignalSemaphoreCount(1)                                  // Signal one semaphore when complete
                .setPSignalSemaphores(&*m_renderFinishedSemaphore);            // Notify when rendering is finished

        // Submit work to GPU with fence-based completion tracking
        // The fence allows CPU to know when this frame's GPU work has completed
        graphicsQueue.submit(submitInfo, *m_inFlightFence);

        // Present the rendered image to the display
        // This operation transfers the completed frame from rendering to display system
        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphoreCount(1)                                   // Wait for rendering completion
               .setPWaitSemaphores(&*m_renderFinishedSemaphore)              // Don't present until rendering finishes
               .setSwapchainCount(1)                                       // Present to one swapchain
               .setPSwapchains(&*m_swapchain)                                // Target swapchain for presentation
               .setPImageIndices(&imageIndex);                             // Present the image we rendered to

        // Submit presentation request to the presentation engine
        presentQueue.presentKHR(presentInfo);
    }
    
    private:
        std::unordered_map<std::string, ImageResource> m_imageResources;
        std::vector<Pass> m_passes;                             
        std::vector<size_t> m_executionOrder;                   

        std::vector<vk::raii::Semaphore> m_semaphores;   
        vk::raii::Semaphore m_imageAvailableSemaphore = nullptr;  
        vk::raii::Semaphore m_renderFinishedSemaphore = nullptr; 
        vk::raii::SwapchainKHR m_swapchain = nullptr;    
        std::vector<std::pair<size_t, size_t>> m_semaphoreSignalWaitPairs;

        vk::raii::Fence m_inFlightFence = nullptr;
        vk::raii::CommandBuffer m_commandBuffer = nullptr;

        vk::raii::Device& m_device;
};

}