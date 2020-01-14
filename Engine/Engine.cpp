#include <Engine/Engine.h>
#include <GLFW/glfw3.h>
#include <iostream>

Engine::Engine() {
    glfwSetErrorCallback([](int errorCode, const char* description) {
        std::cout << "GLFW error: " << description << std::endl;
    });
    glfwInit();

    m_updateClock = std::make_unique<Clock>(0.0f);
    m_updateGroup = std::make_unique<SystemGroup>(*m_updateClock);
}

Engine::~Engine() {
    glfwTerminate();
}

void Engine::addWindow(Window& window) {
    m_window = &window;
}

void Engine::run() {
    while (!m_window->shouldClose()) {
        glfwPollEvents();
        m_updateClock->update();
        m_updateGroup->update();
    }
}