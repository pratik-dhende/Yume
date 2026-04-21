#include "Shader.h"
#include "Services/ShaderCompiler.h"
#include "Services/ResourceManager/Resource.h"
#include "Services/ResourceManager/ResourceManager.h"

#include <string>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <fstream>
#include <slang.h>
#include <exception>
#include <iostream>

namespace Yume {
    Shader::Shader(const std::string& id) : Resource(id) {}

    Shader::~Shader() {
        Resource::Unload();
    }

    ShaderBlob* Shader::GetShaderBytecode() {
        return m_bytecode;
    }

    bool Shader::DoLoad() {
        std::vector<char> shaderCode;
        if (!ServiceLocator::GetService<ResourceManager>().ReadFile<Shader>(Resource::GetId(), shaderCode)) {
            return false;
        }

        auto result = ServiceLocator::GetService<ShaderCompiler>().Compile(std::string(shaderCode.begin(), shaderCode.end()), GetId(), &m_bytecode);
        if (result != SLANG_OK) {
            throw std::runtime_error("Failed to compile shader!");
        }
        
        return true;
    }

    bool Shader::DoUnload() {
        return true;
    }

    
}