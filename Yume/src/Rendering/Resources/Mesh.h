#pragma once

#include "ResourceManagement/Resource.h"

#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Yume {
	
class Mesh : public Resource {
private:
	struct Vertex {
		glm::vec3 position;   
		glm::vec3 normal;     
		glm::vec2 uv;         
	};
public:
    explicit Mesh(const std::string& id) : Resource(id) {}

    ~Mesh() override {
        Unload();                           
    }

    vk::Buffer GetVertexBuffer() const { return m_vertexBuffer; }
    vk::Buffer GetIndexBuffer() const { return m_indexBuffer; }
    uint32_t GetVertexCount() const { return m_vertexCount; }
    uint32_t GetIndexCount() const { return m_indexCount; }

	void Render();

protected:
	bool DoLoad() override {
        std::string filePath = "models/" + GetId() + ".gltf";

        std::vector<Vertex> vertices;      
        std::vector<uint32_t> indices;     
        if (!LoadMeshData(filePath, vertices, indices)) {
            return false;                   
        }

        CreateVertexBuffer(vertices);       
        CreateIndexBuffer(indices);         

        m_vertexCount = static_cast<uint32_t>(vertices.size());
        m_indexCount = static_cast<uint32_t>(indices.size());

        return true;   
    }

	bool DoUnload() override {
        if (IsLoaded()) {
            vk::Device device = GetDevice();

            // Destroy buffers and free GPU memory in proper sequence
            // Index resources cleaned up first to maintain clear dependency order
            device.destroyBuffer(m_indexBuffer);         
            device.freeMemory(m_indexBufferMemory);      

            // Vertex resources cleaned up second
            device.destroyBuffer(m_vertexBuffer);        
            device.freeMemory(m_vertexBufferMemory);     
        }
        return true;
    }

private:
	bool LoadMeshData(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        // Implementation using tinygltf or similar library
        // This method handles the complex task of:
        // - Opening and validating the mesh file format
        // - Parsing vertex attributes (positions, normals, UVs, etc.)
        // - Extracting index data that defines triangle connectivity
        // - Converting from file format to engine-specific vertex structures
        // - Performing validation to ensure data integrity
        // ...
        return true; // Placeholder
    }

    void CreateVertexBuffer(const std::vector<Vertex>& vertices) {
        // Implementation to create Vulkan buffer, allocate memory, and upload data
        // This involves several complex Vulkan operations:
        // - Calculating buffer size requirements based on vertex count and structure
        // - Creating buffer with appropriate usage flags (vertex buffer usage)
        // - Allocating GPU memory with optimal memory type selection
        // - Uploading data via staging buffer for efficient transfer
        // - Setting up memory barriers to ensure data availability
        // ...
    }

    void CreateIndexBuffer(const std::vector<uint32_t>& indices) {
        // Implementation to create Vulkan buffer, allocate memory, and upload data
        // Similar to vertex buffer creation but optimized for index data:
        // - Buffer creation with index buffer specific usage flags
        // - Memory allocation optimized for read-heavy access patterns
        // - Efficient data transfer using appropriate staging mechanisms
        // - Index format validation (16-bit vs 32-bit indices)
        // ...
    }

    vk::Device GetDevice() {
        // Get device from somewhere (e.g., singleton or parameter)
        // Production implementations typically use dependency injection
        // to avoid tight coupling between resource classes and core engine systems
        // ...
        return vk::Device(); // Placeholder
    }

private:
    vk::Buffer m_vertexBuffer;                
    vk::DeviceMemory m_vertexBufferMemory;    
    vk::DeviceSize m_vertexBufferOffset;      
    uint32_t m_vertexCount = 0;               

    vk::Buffer m_indexBuffer;                 
    vk::DeviceMemory m_indexBufferMemory;     
    vk::DeviceSize m_indexBufferOffset;       
    uint32_t m_indexCount = 0;
};
}