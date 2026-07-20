#include "engine.h"
#include <iostream>

void Engine::Start(const std::string &name, int width, int height)
{
    //window = std::make_unique<Window>();
    window.Init(name, height, width);
    window.setMouseCallback([this](float x, float y, uint32_t buttons) {
        handleMouseInput(x, y, buttons);
    });
    window.setKeyboardCallback([this](uint32_t key, bool pressed) {
        handleKeyboardInput(key, pressed);
    });
    renderer.init();
    imguiSystem = std::make_unique<ImguiSystem>(this, &renderer, width, height);
    window.setCharCallback([this](uint32_t key) {
        if (imguiSystem)
        {
            imguiSystem->HandleChar(key);
        }
    });
    cameraSystem = std::make_unique<CameraSystem>();
    modelLoader = std::make_unique<ModelLoader>(&renderer);
    std::cout << "Engine setup done" << std::endl;
}

void Engine::Run()
{
    //std::cout << cameraControl.cameraSpeed << std::endl;
    while(!window.shouldClose())
    {
        glfwPollEvents();
        deltaTimeMS = CalculateDeltaTimeMs();
        imguiSystem->NewFrame(entities);
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
        else
            cameraControl.mouseLeftPressed = false;

        if (buttons & 2) //right click when right click handle camera rotation and
        {
            if (!cameraControl.mouseRightPressed)
            {
                cameraControl.mouseRightPressed = true;
                cameraControl.firstMouse = true;
            }

            if (cameraControl.firstMouse)
            {
                cameraControl.lastMouseX = x;
                cameraControl.lastMouseY = y;
                cameraControl.firstMouse = false;
            }

            float xOffset = x - cameraControl.lastMouseX;
            float yOffset = y - cameraControl.lastMouseY;
            cameraControl.lastMouseX = x;
            cameraControl.lastMouseY = y;

            xOffset *= cameraControl.mouseSensitivity;
            yOffset *= cameraControl.mouseSensitivity;

            cameraControl.yaw -= xOffset;
            cameraControl.pitch -= yOffset;
            
            // Constrain pitch to avoid gimbal lock
            if (cameraControl.pitch > 89.0f)
                cameraControl.pitch = 89.0f;
            if (cameraControl.pitch < -89.0f)
                cameraControl.pitch = -89.0f;
        }
        else
            cameraControl.mouseRightPressed = false;
        
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
    bool imGuiHandleKeyboard = imguiSystem && imguiSystem->ImGuiWantsKeyboard();
    //std::cout << imGuiHandleKeyboard << std::endl;
    if(!imGuiHandleKeyboard)
    {
        switch (key) {
            case GLFW_KEY_UP:
            case GLFW_KEY_W:
                cameraControl.moveForward = pressed;
                break;
            case GLFW_KEY_DOWN:
            case GLFW_KEY_S:
                cameraControl.moveBackward = pressed;
                break;
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D:
                cameraControl.moveRight = pressed;
                break;
            case GLFW_KEY_LEFT:
            case GLFW_KEY_A:
                cameraControl.moveLeft = pressed;
                break;
            default:
                break;
        }
    }
    else
    {
        imguiSystem->HandleKeyboard(key, pressed);
    }

}

void Engine::updateCameraPosition(DeltaTime deltaTime)
{
    if (!cameraControl.mouseRightPressed)
        return;
    
    float velocity = cameraControl.cameraSpeed * deltaTime.count() * .001f;

    // Build delta orientation from yaw/pitch mouse deltas (degrees -> radians)
    const float yawRad = glm::radians(cameraControl.yaw);
    const float pitchRad = glm::radians(cameraControl.pitch);
    
    const glm::quat qDeltaY = glm::angleAxis(yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat qDeltaX = glm::angleAxis(pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));
    // Apply yaw then pitch in the same convention as CameraComponent (ZYX overall), so delta = Ry * Rx
    glm::quat qFinal = qDeltaY * qDeltaX;
    
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

    cameraSystem->setRotation(glm::eulerAngles(qFinal));
    
    cameraSystem->forceMatrixUpdate();
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

void Engine::createEntity(const std::string &name)
{
    auto entity = std::make_unique<Entity>(name);
    entities.push_back(std::move(entity));
}