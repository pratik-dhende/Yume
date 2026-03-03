#pragma once

#include "Resource.h"

#include <string>
#include <vulkan/vulkan.hpp>

namespace Yume {

class Shader : public Resource {
public:
    Shader(const std::string& id, vk::ShaderStageFlagBits shaderStage)
        : Resource(id), m_stage(shaderStage) {}

    ~Shader() override {
        Unload();
    }

    bool Load() override {
        std::string extension;
        switch (m_stage) {
            case vk::ShaderStageFlagBits::eVertex: extension = ".vert"; break;
            case vk::ShaderStageFlagBits::eFragment: extension = ".frag"; break;
            case vk::ShaderStageFlagBits::eCompute: extension = ".comp"; break;
            default: return false;
        }

        std::string filePath = "shaders/" + GetId() + extension + ".spv";

        std::vector<char> shaderCode;
        if (!ReadFile(filePath, shaderCode)) {
            return false;
        }

        CreateShaderModule(shaderCode);

        return Resource::Load();
    }

    void Unload() override {
        if (IsLoaded()) {
            // Get device from somewhere (e.g., singleton or parameter)
            vk::Device device = GetDevice();

            device.destroyShaderModule(m_shaderModule);

            Resource::Unload();
        }
    }

    vk::ShaderModule GetShaderModule() const { return m_shaderModule; }
    vk::ShaderStageFlagBits GetStage() const { return m_stage; }

private:
    bool ReadFile(const std::string& filePath, std::vector<char>& buffer) {
        // Implementation to read binary file
        // ...
        return true; // Placeholder
    }

    void CreateShaderModule(const std::vector<char>& code) {
        // Implementation to create Vulkan shader module
        // ...
    }

    vk::Device GetDevice() {
        // Get device from somewhere (e.g., singleton or parameter)
        // ...
        return vk::Device(); // Placeholder
    }

private:
    vk::ShaderModule m_shaderModule;
    vk::ShaderStageFlagBits m_stage;

};

}