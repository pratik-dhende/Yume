#pragma once

#include <string>
#include <memory>
#include <GLFW/glfw3.h>

namespace Yume {

class Application {

public:
    Application(const int width, const int height, const std::string& title) : m_width(width), m_height(height), m_title(title) {

    }

    virtual ~Application() = default;

    void Run();

protected:
    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Shutdown() = 0;

private:
    int m_width;
    int m_height;
    std::string m_title;

    GLFWwindow* m_window;

private:
    void InitWindow();
    void DestroyWindow();
    void MainLoop();

};

extern std::unique_ptr<Application> CreateApplication();

}