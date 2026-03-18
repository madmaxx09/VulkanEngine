#include "engine.h"
#include <iostream>

void Engine::Start(const std::string &name, int width, int height)
{
    window.Init(name, height, width);
    renderer.init();
    std::cout << "Engine setup done" << std::endl;
}

void Engine::Run()
{
    while(!window.shouldClose())
    {
        glfwPollEvents();
        renderer.drawFrame();
    }
    renderer.idle();
    renderer.cleanup();
}