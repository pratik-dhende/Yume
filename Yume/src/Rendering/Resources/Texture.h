#pragma once

#include "ResourceManagement/Resource.h"

#include <string>
#include <vulkan/vulkan.hpp>

namespace Yume {

class Texture : public Resource {
public:
    explicit Texture(const std::string& id) : Resource(id) {}

    ~Texture() override {
        Unload();                 
    }

    vk::Image GetImage() const { return m_image; }
    vk::ImageView GetImageView() const { return m_imageView; }
    vk::Sampler GetSampler() const { return m_sampler; }

protected:
     bool DoLoad() override {
        std::string filePath = "textures/" + GetId() + ".ktx";

        unsigned char* data = LoadImageData(filePath, &m_width, &m_height, &m_channels);
        if (!data) {
            return false;           
        }

        CreateVulkanImage(data, m_width, m_height, m_channels);

        FreeImageData(data);

        return true;
    }

    bool DoUnload() override {
        if (IsLoaded()) {
            vk::Device device = GetDevice();

            // Destruction order matters
            device.destroySampler(m_sampler);       
            device.destroyImageView(m_imageView);   
            device.destroyImage(m_image);           
            device.freeMemory(m_memory);            
        }
        return true;
    }

private:
    vk::Image m_image;              
    vk::DeviceMemory m_memory;      
    vk::DeviceSize m_offset;        
    vk::ImageView m_imageView;      
    vk::Sampler m_sampler;          

    int m_width = 0;                
    int m_height = 0;               
    int m_channels = 0; 

    vk::Format m_format;                    
    vk::Extent2D m_extent;                  
    vk::ImageUsageFlags m_usage;            
    vk::ImageLayout m_initialLayout;        // Expected layout when the frame begins
    vk::ImageLayout m_finalLayout;          // Required layout when the frame ends

private:
    unsigned char* LoadImageData(const std::string& filePath, int* width, int* height, int* channels) {
        // Implementation using stb_image or ktx library
        // This method abstracts the details of different image format support
        // and provides a consistent interface for pixel data loading
        // ...
        return nullptr; // Placeholder
    }

    void FreeImageData(unsigned char* data) {
        // Implementation using stb_image or ktx library
        // Ensures proper cleanup of image loader specific memory allocations
        // Different libraries may require different cleanup approaches
        // ...
    }

    void CreateVulkanImage(unsigned char* data, int width, int height, int channels) {
        // Implementation to create Vulkan image, allocate memory, and upload data
        // This involves complex Vulkan operations including:
        // - Format selection based on channel count and data type
        // - Memory allocation with appropriate usage flags
        // - Image creation with optimal tiling and layout
        // - Data upload via staging buffers for efficiency
        // - Image view creation for shader access
        // - Sampler creation with appropriate filtering settings
        // ...
    }

    vk::Device GetDevice() {
        // Get device from somewhere (e.g., singleton or parameter)
        // Production code would use dependency injection or service location
        // to provide the Vulkan device handle without tight coupling
        // ...
        return vk::Device(); // Placeholder
    }
};
}