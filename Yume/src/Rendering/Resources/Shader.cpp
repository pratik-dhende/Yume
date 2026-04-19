#include "Shader.h"
#include "Services/ShaderCompiler.h"
#include "Services/ResourceManager/Resource.h"
#include "Services/FileUtils.h"

#include <string>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>
#include <slang.h>
#include <exception>

namespace Yume {
    Shader::Shader(const std::string& id, const std::string& importName, const vk::ShaderStageFlagBits shaderStage, vk::raii::Device& device)
        : Resource(id), m_importName(importName), m_stage(shaderStage), m_device(device) {}

    Shader::~Shader() {
        Resource::Unload();
    }

    vk::ShaderModule Shader::GetShaderModule() const { return m_shaderModule; }
    vk::ShaderStageFlagBits Shader::GetStage() const { return m_stage; }

    bool Shader::DoLoad(const std::string& filepath) {
        std::vector<char> shaderCode;
        if (!ServiceLocator::GetService<FileUtils>().ReadFile(filepath, shaderCode)) {
            return false;
        }
        
        ShaderBlob* shaderBytecode = nullptr;

        auto result = ServiceLocator::GetService<ShaderCompiler>().Compile(std::string(shaderCode.begin(), shaderCode.end()), {{"vertMain", SLANG_STAGE_VERTEX}, {"fragMain", SLANG_STAGE_FRAGMENT}}, m_importName, &shaderBytecode, filepath);
        if (result != SLANG_OK) {
            throw std::runtime_error("Failed to compile shader!");
            return false;
        }

        CreateShaderModule(shaderBytecode);

        return true;
    }

    bool Shader::DoUnload() {
        if (IsLoaded()) {
            m_shaderModule = nullptr;
        }
        return true;
    }

    void Shader::CreateShaderModule(ShaderBlob* bytecode) {
        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.codeSize = bytecode->getBufferSize();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode->getBufferPointer());
        m_shaderModule = m_device.createShaderModule(createInfo);
    }
}