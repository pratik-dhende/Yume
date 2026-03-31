#include "Application.h"

namespace Yume {

void Application::Run() {
    InitWindow();

    Init();
    MainLoop();
    Shutdown();

    DestroyWindow();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void Application::InitWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OPENGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resizing

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
}

void Application::DestroyWindow() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}