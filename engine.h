#pragma once

#include "window.h"
#include "renderer.h"

class Engine
{
    public:
        Engine() : window(), renderer(window) {}
        ~Engine() = default;

        void Start(const std::string &name, int width, int height);
        void Run();

        Window getWindow() { return window; };

    private:
        Window window;
        Renderer renderer;
};