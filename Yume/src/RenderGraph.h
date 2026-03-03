#pragma once

#include "Texture.h"

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

public:
    explicit RenderGraph(vk::raii::Device& device) : m_device(device) {}

    void AddTextureResource(const Texture& texture) {
        // m_textureResources[texture.GetId()] = texture; TOOO: Use later
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
        std::vector<std::vector<size_t>> passDependencies(m_passes.size());
        std::vector<std::vector<size_t>> passDependents(m_passes.size());

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
                    passDependencies[i].push_back(it->second);      
                    passDependents[it->second].push_back(i);         
                }
            }

            for (const auto& output : pass.outputs) {
                auto it = latestResourceReaders.find(output);
                if (it != latestResourceReaders.end()) {
                    // WAR
                    passDependencies[i].push_back(it->second);
                    passDependents[it->second].push_back(i);
                }
                
                it = latestResourceWriters.find(output);
                if (it != latestResourceWriters.end()) {
                    // WAW
                    passDependencies[i].push_back(it->second);
                    passDependents[it->second].push_back(i);
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
            for (auto dependent : passDependents[node]) {
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
    }

private:
    // std::unordered_map<std::string, Texture> m_textureResources;  TODO: Use later. Currently uncommented to avoid constructor error.
    std::vector<Pass> m_passes;                             
    std::vector<size_t> m_executionOrder;                   

    std::vector<vk::raii::Semaphore> m_semaphores;          
    std::vector<std::pair<size_t, size_t>> m_semaphoreSignalWaitPairs;

    vk::raii::Device& m_device;
};

}