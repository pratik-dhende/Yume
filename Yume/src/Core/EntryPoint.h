#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "Application.h"

// class HelloTriangleApplication {
// static constexpr int WIDTH = 800;
// static constexpr int HEIGHT = 600;

// public:
//     void run() {
// 		initWindow();
//         initVulkan();
//         mainLoop();
//         cleanup();
//     }

// 	private:
// 		void initWindow() {
// 			glfwInit();

// 			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
// 			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

// 			m_window = glfwCreateWindow(WIDTH, HEIGHT, "Hello Triangle", nullptr, nullptr);
// 		}

// 		void initVulkan() {

// 		}

// 		void mainLoop() {
// 			while (!glfwWindowShouldClose(m_window)) {
//         		glfwPollEvents();
//     		}
// 		}

// 		void cleanup() {

// 		}

// private:
// 	GLFWwindow* m_window;
// };

int main() {
	try
    {
        auto app = Yume::CreateApplication();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}