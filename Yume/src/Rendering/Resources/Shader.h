#pragma once

#include "Services/ResourceManager/Resource.h"
#include "Services/ShaderCompiler.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Yume {

class Shader : public Resource {
public:
    Shader(const std::string& id, const std::string& importName, const vk::ShaderStageFlagBits stage, const std::string& entryPoint, vk::raii::Device& device);
    ~Shader();

    vk::ShaderModule GetShaderModule() const;
    vk::ShaderStageFlagBits GetStage() const;

protected:
    bool DoLoad() override;
    bool DoUnload() override;

private:
    void CreateShaderModule(ShaderBlob* bytecode);

private:
    vk::raii::ShaderModule m_shaderModule = nullptr;
    vk::ShaderStageFlagBits m_stage;
    vk::raii::Device& m_device;

    std::string m_importName;
    std::string m_entryPoint;
};

}