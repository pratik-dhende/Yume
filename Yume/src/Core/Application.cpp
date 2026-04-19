#include "Application.h"
#include "Rendering/Core/Renderer.h"
#include "Rendering/Resources/Shader.h"
#include "Services/ShaderCompiler.h"
#include "Services/ResourceManager/HotReloadResourceManager.h"

namespace Yume {

void Application::Run() {
    RegisterServices();

    InitWindow();
    InitRenderer();

    Init();
    MainLoop();
    Shutdown();

    DestroyWindow();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        ServiceLocator::GetService<HotReloadResourceManager>().HotReload();
    }
}

void Application::InitWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OPENGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resizing

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
}

void Application::InitRenderer() {
#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

    m_renderer = std::make_unique<Renderer>(enableValidationLayers, m_window);
    m_renderer->Init();
}

void Application::DestroyWindow() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::RegisterServices() {
    auto shaderCompiler = std::make_unique<ShaderCompiler>();
    auto resourceManager = std::make_unique<HotReloadResourceManager>();

    ServiceLocator::RegisterService(std::move(shaderCompiler));
    ServiceLocator::RegisterService(std::move(resourceManager));
}

}