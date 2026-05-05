#include "engine.h"
#include <iostream>

void Engine::Start(const std::string &name, int width, int height)
{
    window.Init(name, height, width);
    window.setMouseCallback([this](float x, float y, uint32_t buttons) {
        handleMouseInput(x, y, buttons);
    });
    window.setKeyboardCallback([this](uint32_t key, bool pressed) {
        handleKeyboardInput(key, pressed);
    });
    renderer.init();
    imguiSystem = std::make_unique<ImguiSystem>(&renderer, width, height);
    cameraSystem = std::make_unique<CameraSystem>();
    cameraSystem->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
    //cameraSystem
    std::cout << "Engine setup done" << std::endl;
}

void Engine::Run()
{
    deltaTimeMS = CalculateDeltaTimeMs();
    while(!window.shouldClose())
    {
        glfwPollEvents();
        imguiSystem->NewFrame();
        updateCameraPosition(deltaTimeMS);
        renderer.drawFrame(imguiSystem.get(), cameraSystem.get());
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

void Engine::handleKeyboardInput(uint32_t key, bool pressed)
{
    switch (key) {
        case GLFW_KEY_UP:
            cameraControl.moveUp = pressed;
        case GLFW_KEY_DOWN:
            cameraControl.moveDown = pressed;
        case GLFW_KEY_RIGHT:
            cameraControl.moveRight = pressed;
        case GLFW_KEY_LEFT:
            cameraControl.moveLeft = pressed;
    }
}

void Engine::updateCameraPosition(DeltaTime deltaTime)
{
    float velocity = cameraControl.cameraSpeed * deltaTime.count() * .001f;
 
    // Build delta orientation from yaw/pitch mouse deltas (degrees -> radians)
    const float yawRad = glm::radians(cameraControl.yaw);
    const float pitchRad = glm::radians(cameraControl.pitch);
    const glm::quat qDeltaY = glm::angleAxis(yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat qDeltaX = glm::angleAxis(pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));
    // Apply yaw then pitch in the same convention as CameraComponent (ZYX overall), so delta = Ry * Rx
    glm::quat qDelta = qDeltaY * qDeltaX;
    glm::quat qFinal = cameraControl.baseOrientation * qDelta;
    
    // Derive camera basis directly from rotated axes to avoid ambiguity
    glm::vec3 right = glm::normalize(qFinal * glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 up = glm::normalize(qFinal * glm::vec3(0.0f, 1.0f, 0.0f));
    // Camera forward in world space.
    // Our view/projection conventions assume the camera looks down -Z in its local space.
    glm::vec3 front = glm::normalize(qFinal * glm::vec3(0.0f, 0.0f, -1.0f));


    glm::vec3 camPosition = cameraSystem->getPosition();

    if (cameraControl.moveForward)
        camPosition += front * velocity;
    if (cameraControl.moveBackward)
        camPosition -= front * velocity;
    if (cameraControl.moveLeft)
        camPosition -= right * velocity;
    if (cameraControl.moveRight)
        camPosition += right * velocity;
    if (cameraControl.moveUp)
        camPosition += up * velocity;
    if (cameraControl.moveDown)
        camPosition -= up * velocity;
    

    cameraSystem->setPosition(camPosition);
    
}

std::chrono::milliseconds Engine::CalculateDeltaTimeMs()
{
    // Get current time using a steady clock to avoid system time jumps
    uint64_t currentTime = static_cast<uint64_t>(
    std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
    .count());

    // Initialize lastFrameTimeMs on first call
    if (lastFrameTimeMs == 0) {
    lastFrameTimeMs = currentTime;
    return std::chrono::milliseconds(16); // ~16ms as a sane initial guess
    }

    // Calculate delta time in milliseconds
    uint64_t delta = currentTime - lastFrameTimeMs;

    // Update last frame time
    lastFrameTimeMs = currentTime;

    return std::chrono::milliseconds(static_cast<long long>(delta));
}