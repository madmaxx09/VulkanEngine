#include "engine.h"
#include <iostream>

bool Engine::Start(const std::string &name, int width, int height)
{
    if(!window.Init(name, height, width))
        return false;
    if(!renderer.Init())
        return false;
    std::cout << "Engine setup done" << std::endl;
    return true;
}

void Engine::Run()
{
    while(!window.shouldClose())
    {
        glfwPollEvents();
    }
}