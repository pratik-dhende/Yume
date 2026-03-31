#pragma once

#include <string>
#include <memory>

namespace Yume {

class Application {

public:
    Application(const int width, const int height, const std::string& title) : m_width(width), m_height(height) {

    }

    virtual ~Application() = default;

protected:
    virtual void Init() = 0;
    virtual void Run() = 0;
    virtual void Shutdown() = 0;

private:
    int m_width;
    int m_height;
};

}