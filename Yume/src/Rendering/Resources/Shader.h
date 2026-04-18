#pragma once

#include "ResourceManagement/Resource.h"
#include "Rendering/Core/ShaderCompiler.h"

#include <string>
#include <vulkan/vulkan.hpp>
#include <fstream>
#include <slang.h>
#include <exception>

namespace Yume {

class Shader : public Resource {
public:
    Shader(const std::string& id, vk::ShaderStageFlagBits shaderStage, vk::raii::Device& device)
        : Resource(id), m_stage(shaderStage), m_device(device) {}

    ~Shader() override {
        Unload();
    }

    vk::ShaderModule GetShaderModule() const { return m_shaderModule; }
    vk::ShaderStageFlagBits GetStage() const { return m_stage; }

protected:
    bool DoLoad() override {
        std::string extension;
        switch (m_stage) {
            case vk::ShaderStageFlagBits::eVertex: extension = ".vert"; break;
            case vk::ShaderStageFlagBits::eFragment: extension = ".frag"; break;
            case vk::ShaderStageFlagBits::eCompute: extension = ".comp"; break;
            default: return false;
        }

        std::string filepath = "shaders/" + GetId() + extension + ".spv";
        filepath = "C:\\dev\\Yume\\Yume\\shaders\\shader.slang";
        
        ShaderCompiler shaderCompiler;
        shaderCompiler.Init();

        std::vector<char> shaderCode;
        if (!ReadFile(filepath, shaderCode)) {
            return false;
        }
        
        ShaderBlob* shaderBytecode = nullptr;
        auto result = shaderCompiler.Compile(std::string(shaderCode.begin(), shaderCode.end()), {{"vertMain", SLANG_STAGE_VERTEX}, {"fragMain", SLANG_STAGE_FRAGMENT}}, "shader", &shaderBytecode, filepath);
        if (result != SLANG_OK) {
            throw std::runtime_error("Failed to compile shader!");
            return false;
        }
        CreateShaderModule(shaderBytecode);

        return true;
    }

    bool DoUnload() override {
        if (IsLoaded()) {
            m_shaderModule = nullptr;
        }
        return true;
    }

private:
    bool ReadFile(const std::string& filePath, std::vector<char>& buffer) {
        std::ifstream file(filePath, std::ios::ate);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

        file.close();

        return true;
    }

    void CreateShaderModule(ShaderBlob* bytecode) {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = bytecode->getBufferSize(), .pCode = reinterpret_cast<const uint32_t*>(bytecode->getBufferPointer()) };
        m_shaderModule = m_device.createShaderModule(createInfo);
    }

private:
    vk::raii::ShaderModule m_shaderModule = nullptr;
    vk::ShaderStageFlagBits m_stage;
    vk::raii::Device& m_device;

};

}