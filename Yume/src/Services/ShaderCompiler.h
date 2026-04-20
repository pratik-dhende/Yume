#pragma once

#include "ServiceLocator/ServiceLocator.h"

#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace Yume {

using ShaderBlob = slang::IBlob;

class ShaderCompiler : public ServiceLocator::IService {
    static inline const std::unordered_map<const vk::ShaderStageFlagBits, SlangStage> s_stageName = {
        {vk::ShaderStageFlagBits::eVertex, SLANG_STAGE_VERTEX},
        {vk::ShaderStageFlagBits::eFragment, SLANG_STAGE_FRAGMENT}
    };
public:
    ShaderCompiler();
    SlangResult Compile(const std::string& code, const vk::ShaderStageFlagBits& shaderStage,  const std::string& entryPoint, const std::string& importName, ShaderBlob** outSpirvCode, const std::string& resourceId);
    SlangResult loadModule(const std::string& code, const std::string& importName, const std::string& resourceId, Slang::ComPtr<slang::IModule>& outModule);
    SlangResult composeProgram(const std::vector<slang::IComponentType*> componentTypes, Slang::ComPtr<slang::IComponentType>& outComposedProgram);
    SlangResult linkProgram(const Slang::ComPtr<slang::IComponentType>& composedProgram, Slang::ComPtr<slang::IComponentType>& outLinkedProgram);
    SlangResult compileProgram(const Slang::ComPtr<slang::IComponentType>& linkedProgram, Slang::ComPtr<slang::IBlob>& outSpirvCode);
    SlangResult loadEntryPoints(const vk::ShaderStageFlagBits& shaderStage, const std::string& entryPoint, const Slang::ComPtr<slang::IModule>& slangModule, std::vector<slang::IEntryPoint*>& outEntryPoints);
    static void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob);

private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    Slang::ComPtr<slang::ISession> m_session;
};

}