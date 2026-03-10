#pragma once

#include "window.h"
#include "renderer.h"

class Engine
{
    public:
        Engine() = default;
        ~Engine() = default;

        bool Start(const std::string &name, int width, int height);
        void Run();

        Window getWindow() { return window; };

    private:
        Window window;
        Renderer renderer;
};