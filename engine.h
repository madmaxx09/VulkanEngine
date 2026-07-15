#pragma once

#include "window.h"
#include "renderer.h"
#include "imgui_layer.h"
#include "camera.h"
#include "entity.h"
#include <memory>
#include <chrono>
#include <vector>

class Engine
{
    public:
        using DeltaTime = std::chrono::milliseconds;

        Engine() : window(), renderer(window) {}
        ~Engine() = default;

        void Start(const std::string &name, int width, int height);
        void Run();

        void updateCameraPosition(DeltaTime deltaTime);

        Window getWindow() { return window; };

        void handleMouseInput(float x, float y, uint32_t buttons);
        void handleKeyboardInput(uint32_t key, bool pressed);

    private:
        Window window;
        Renderer renderer;
        std::unique_ptr<ImguiSystem> imguiSystem;
        std::unique_ptr<CameraSystem> cameraSystem;


        std::vector<std::unique_ptr<Entity>> entities;

        float mouseX = 0.0f;
        float mouseY = 0.0f;

        std::chrono::milliseconds deltaTimeMS{0};
        uint64_t                  lastFrameTimeMs = 0;

        std::chrono::milliseconds CalculateDeltaTimeMs();

        struct CameraState
        {
           	bool      moveForward             = false;
            bool      moveBackward            = false;
            bool      moveLeft                = false;
            bool      moveRight               = false;
            bool      moveUp                  = false;
            bool      moveDown                = false;
            bool      mouseLeftPressed        = false;
            bool      mouseRightPressed       = false;
            bool      firstMouse              = false;

            float     lastMouseX              = 0.0f;
            float     lastMouseY              = 0.0f;
            float     yaw                     = 0.0f;
            float     pitch                   = 0.0f;
            float     cameraSpeed             = 5.0f;
            float     mouseSensitivity        = 0.1f;
            glm::quat baseOrientation{1.0f, 0.0f, 0.0f, 0.0f};
        } cameraControl;

        void handleMouseHover(float x, float y);
};