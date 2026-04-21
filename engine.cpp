#include "engine.h"
#include <iostream>

void Engine::Start(const std::string &name, int width, int height)
{
    window.Init(name, height, width);
    window.setMouseCallback([this](float x, float y, uint32_t buttons) {
        handleMouseInput(x, y, buttons);
    });
    renderer.init();
    imguiSystem = std::make_unique<ImguiSystem>(&renderer, width, height);
    std::cout << "Engine setup done" << std::endl;
}

void Engine::Run()
{
    while(!window.shouldClose())
    {
        glfwPollEvents();
        renderer.drawFrame(imguiSystem.get());
    }
    renderer.idle();
    renderer.cleanup();
}

void Engine::handleMouseInput(float x, float y, uint32_t buttons)
{
    bool imguiHandlesMouse = imguiSystem && imguiSystem->ImguiWantsMouse();

    if (imguiHandlesMouse) 
    {
        
    }
}