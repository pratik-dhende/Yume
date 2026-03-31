#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

class RenderTarget;

namespace Yume {

class RenderPass {
private:
    std::string m_name;
    std::vector<std::string> m_dependencies;
    RenderTarget* m_target = nullptr;
    bool m_enabled = true;

public:
    explicit RenderPass(const std::string& passName) : m_name(passName) {}
    virtual ~RenderPass() = default;

    const std::string& GetName() const { return m_name; }

    void AddDependency(const std::string& dependency) {
        m_dependencies.push_back(dependency);
    }

    const std::vector<std::string>& GetDependencies() const {
        return m_dependencies;
    }

    void SetRenderTarget(RenderTarget* renderTarget) {
        m_target = renderTarget;
    }

    RenderTarget* GetRenderTarget() const {
        return m_target;
    }

    void SetEnabled(bool isEnabled) {
        m_enabled = isEnabled;
    }

    bool IsEnabled() const {
        return m_enabled;
    }

    virtual void Execute(vk::raii::CommandBuffer& commandBuffer) {
        if (!m_enabled) return;

        BeginPass(commandBuffer);
        Render(commandBuffer);
        EndPass(commandBuffer);
    }

protected:
    virtual void BeginPass(vk::raii::CommandBuffer& commandBuffer) = 0;
    virtual void Render(vk::raii::CommandBuffer& commandBuffer) = 0;
    virtual void EndPass(vk::raii::CommandBuffer& commandBuffer) = 0;
};

}