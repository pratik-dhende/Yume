#include "Application.h"
#include "Rendering/Core/Renderer.h"
#include "Rendering/Resources/Shader.h"
#include "Services/ShaderCompiler.h"
#include "Services/ResourceManager/ResourceManager.h"
#include "Services/EventSystem/EventBus.h"

namespace Yume {

void Application::Run() {
    RegisterServices();

    InitWindow();
    InitRenderer();

    Init();
    MainLoop();
    Shutdown();

    DestroyRenderer();
    DestroyWindow();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        m_renderer->DrawFrame();
    }
}

void Application::InitWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OPENGL

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
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

void Application::DestroyRenderer() {
    m_renderer->ShutDown();
    m_renderer = nullptr;
}

void Application::DestroyWindow() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::RegisterServices() {
    auto shaderCompiler = std::make_unique<ShaderCompiler>();
    auto resourceManager = std::make_unique<ResourceManager>();
    auto eventBus = std::make_unique<EventBus>();

    ServiceLocator::RegisterService(std::move(shaderCompiler));
    ServiceLocator::RegisterService(std::move(resourceManager));
    ServiceLocator::RegisterService(std::move(eventBus));
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    ServiceLocator::GetService<EventBus>().PublishEvent(WindowResizeEvent(width, height));
}

}