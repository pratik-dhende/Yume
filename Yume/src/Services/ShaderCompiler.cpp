#include "ShaderCompiler.h"
#include "ServiceLocator/ServiceLocator.h"

#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>
#include <string>
#include <vector>
#include <array>

namespace Yume {

using ShaderBlob = slang::IBlob;

ShaderCompiler::ShaderCompiler()  {
    createGlobalSession(m_globalSession.writeRef());

    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};

    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = m_globalSession->findProfile("spirv_1_4");

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    std::array<slang::CompilerOptionEntry, 1> options = 
    {
        {
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
        }
    };
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    m_globalSession->createSession(sessionDesc, m_session.writeRef());
}

SlangResult ShaderCompiler::Compile(const std::string& code, const std::vector<std::pair<std::string, SlangStage>>& entryPoints, const std::string& importName, ShaderBlob** outSpirvCode, const std::string& shaderPath) {
    Slang::ComPtr<slang::IModule> slangModule;
    SLANG_RETURN_ON_FAIL(loadModule(code, importName, shaderPath, slangModule));

    std::vector<slang::IEntryPoint*> slangEntryPoints;
    SLANG_RETURN_ON_FAIL(loadEntryPoints(entryPoints, slangEntryPoints, slangModule));

    std::vector<slang::IComponentType*> componentTypes(1, slangModule.get());
    componentTypes.insert(componentTypes.end(), slangEntryPoints.begin(), slangEntryPoints.end());
    Slang::ComPtr<slang::IComponentType> composedProgram;
    SLANG_RETURN_ON_FAIL(composeProgram(componentTypes, composedProgram));

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(linkProgram(composedProgram, linkedProgram));

    Slang::ComPtr<slang::IBlob> spirvCode;
    SLANG_RETURN_ON_FAIL(compileProgram(linkedProgram, spirvCode));
    
    *outSpirvCode = spirvCode.detach();

    return SLANG_OK;
}

SlangResult ShaderCompiler::loadModule(const std::string& code, const std::string& importName, const std::string& shaderPath, Slang::ComPtr<slang::IModule>& outModule) {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    const char* moduleName = importName.c_str();
    const char* modulePath = shaderPath.c_str();

    outModule = m_session->loadModuleFromSourceString(moduleName, modulePath, code.c_str(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);

    if (!outModule)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult ShaderCompiler::loadEntryPoints(const std::vector<std::pair<std::string, SlangStage>>& entryPoints, std::vector<slang::IEntryPoint*>& outEntryPoints, const Slang::ComPtr<slang::IModule>& slangModule) {
    for (const auto& [entryPoint, stage] : entryPoints)
    {   
        Slang::ComPtr<slang::IEntryPoint> slangEntryPoint;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            slangModule->findAndCheckEntryPoint(entryPoint.c_str(), stage, slangEntryPoint.writeRef(), diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
            if (!slangEntryPoint)
            {
                return SLANG_FAIL;
            }
        }
        outEntryPoints.push_back(slangEntryPoint.get());
    }
    return SLANG_OK;
}

SlangResult ShaderCompiler::composeProgram(const std::vector<slang::IComponentType*> componentTypes, Slang::ComPtr<slang::IComponentType>& outComposedProgram) {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = m_session->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        outComposedProgram.writeRef(),
        diagnosticsBlob.writeRef());

    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    return SLANG_OK;
}

SlangResult ShaderCompiler::linkProgram(const Slang::ComPtr<slang::IComponentType>& composedProgram, Slang::ComPtr<slang::IComponentType>& outLinkedProgram) {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = composedProgram->link(
        outLinkedProgram.writeRef(),
        diagnosticsBlob.writeRef());

    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);
    
    return SLANG_OK;
}

SlangResult ShaderCompiler::compileProgram(const Slang::ComPtr<slang::IComponentType>& linkedProgram, Slang::ComPtr<slang::IBlob>& outSpirvCode) {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = linkedProgram->getTargetCode(
        0,
        outSpirvCode.writeRef(),
        diagnosticsBlob.writeRef());

    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);
    
    return SLANG_OK;
}

void ShaderCompiler::diagnoseIfNeeded(slang::IBlob* diagnosticsBlob) { 
    if (diagnosticsBlob)
    {
        const char* diagnostics = (const char*)diagnosticsBlob->getBufferPointer();
        size_t diagnosticsSize = diagnosticsBlob->getBufferSize();
        std::string diagnosticsStr(diagnostics, diagnosticsSize);

        if (!diagnosticsStr.empty())
        {
            printf("Shader Compilation Diagnostics:{%s}\n", diagnosticsStr.c_str());
        }
    }
}

}