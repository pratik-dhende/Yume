#include "Shader.h"
#include "Services/ShaderCompiler.h"
#include "Services/ResourceManager/Resource.h"
#include "Services/ResourceManager/HotReloadResourceManager.h"

#include <string>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>
#include <slang.h>
#include <exception>
#include <iostream>

namespace Yume {
    Shader::Shader(const std::string& id, const std::string& importName, const vk::ShaderStageFlagBits stage, const std::string& entryPoint, vk::raii::Device& device)
        : Resource(id), m_importName(importName), m_stage(stage), m_device(device), m_entryPoint(entryPoint) {}

    Shader::~Shader() {
        Resource::Unload();
    }

    vk::ShaderModule Shader::GetShaderModule() const { return m_shaderModule; }
    vk::ShaderStageFlagBits Shader::GetStage() const { return m_stage; }

    bool Shader::DoLoad() {
        std::vector<char> shaderCode;
        if (!ServiceLocator::GetService<HotReloadResourceManager>().ReadFile<Shader>(Resource::GetId(), shaderCode)) {
            return false;
        }
        
        ShaderBlob* shaderBytecode = nullptr;

        std::cout << "Compiling shader: " << std::string(shaderCode.begin(), shaderCode.end()) << std::endl;

        auto result = ServiceLocator::GetService<ShaderCompiler>().Compile(std::string(shaderCode.begin(), shaderCode.end()), m_stage, m_entryPoint, m_importName, &shaderBytecode, GetId());
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