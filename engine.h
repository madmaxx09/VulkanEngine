#pragma once

#include "window.h"
#include "renderer.h"
#include "imgui_layer.h"
#include <memory>
#include <chrono>

class Engine
{
    public:
        using DeltaTime = std::chrono::milliseconds;

        Engine() : window(), renderer(window) {}
        ~Engine() = default;

        void Start(const std::string &name, int width, int height);
        void Run();

        Window getWindow() { return window; };

        void handleMouseInput(float x, float y, uint32_t buttons);
        void handleKeyboardInput(uint32_t key, bool pressed);

    private:
        Window window;
        Renderer renderer;
        std::unique_ptr<ImguiSystem> imguiSystem;

        float mouseX = 0.0f;
        float mouseY = 0.0f;

        void handleMouseHover(float x, float y);
};