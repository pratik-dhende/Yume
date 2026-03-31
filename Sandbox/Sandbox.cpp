#pragma once

#include "Yume/Yume.h"

class Sandbox : public Yume::Application {
public:
    Sandbox(int width, int height, const std::string& title) : Yume::Application(width, height, title) {}

protected:
    void Init() override {
        // Initialize your application here
    }

    void Update() override {
        // Main loop of your application
    }

    void Shutdown() override {
        // Clean up resources here
    }
};

std::unique_ptr<Yume::Application> Yume::CreateApplication() {
    return std::make_unique<Sandbox>(800, 600, "Sandbox");
}

