#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Yume {

class RenderTarget {
private:
    vk::raii::Image colorImage = nullptr;
    vk::raii::DeviceMemory colorMemory = nullptr;
    vk::raii::ImageView colorImageView = nullptr;

    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;

    uint32_t width;
    uint32_t height;

public:
    RenderTarget(const uint32_t w, const uint32_t h) : width(w), height(h) {
        CreateColorResources();
        CreateDepthResources();
    }

    vk::ImageView GetColorImageView() const { return *colorImageView; }
    vk::ImageView GetDepthImageView() const { return *depthImageView; }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    void CreateColorResources() {
        // TODO
        // Implementation to create color image, memory, and view
        // With dynamic rendering, we just need to create the image and image view
        // that will be used with vkCmdBeginRendering
        // ...
    }

    void CreateDepthResources() {
        // TODO
        // Implementation to create depth image, memory, and view
        // With dynamic rendering, we just need to create the image and image view
        // that will be used with vkCmdBeginRendering
        // ...
    }

    vk::raii::Device& GetDevice() {
        // TODO
        // Get device from somewhere (e.g., singleton or parameter)
        // ...
        static vk::raii::Device device = nullptr; // Placeholder
        return device;
    }
};

}