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
        imguiSystem->NewFrame();
        renderer.drawFrame(imguiSystem.get());
    }
    renderer.idle();
    renderer.cleanup();
}

void Engine::handleMouseInput(float x, float y, uint32_t buttons)
{
    bool imguiHandlesMouse = imguiSystem && imguiSystem->ImguiWantsMouse();
    if (!imguiHandlesMouse)
    {
        if (buttons & 1) //left click
            std::cout << "out of imgui left click" << std::endl;
        if (buttons & 2) //right click
            std::cout << "out of imgui right click" << std::endl;
    }
    if (imguiSystem)
        imguiSystem->HandleMouse(x, y, buttons);
    
    handleMouseHover(x, y); //really not sure about the usefullness yet
}

void Engine::handleMouseHover(float x, float y)
{
    mouseX = x;
    mouseY = y;
}