#pragma once

#include "Services/ResourceManager/Resource.h"
#include "Services/ShaderCompiler.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Yume {

class Shader : public Resource {
public:
    Shader(const std::string& id);
    ~Shader();

    ShaderBlob* GetShaderBytecode();

protected:
    bool DoLoad() override;
    bool DoUnload() override;

private:
    ShaderBlob* m_bytecode;
};

}