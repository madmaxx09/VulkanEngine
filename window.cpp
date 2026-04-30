#include "window.h"
#include <iostream>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto wind = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    wind->framebufferResized = true;
    std::cout << "wind callback";
}

void Window::Init(const std::string &name, int height, int width)
{
    if (!glfwInit()) 
    {
        throw std::runtime_error("GLFW init fail");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); 

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        throw std::runtime_error("GLFW createwindow fail");
    }
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    // glfwSetKeyCallback(window, KeyCallback);
    // glfwSetCharCallback(window, CharCallback);
}

void Window::setMouseCallback(std::function<void(float, float, uint32_t)> callback)
{
    mouseCallback = std::move(callback);
}

void Window::setKeyboardCallback(std::function < void(uint32_t, bool) > callback)
{
    keyboardCallback = std::move(callback);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* platform = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (platform->mouseCallback) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        uint32_t buttons = 0;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            buttons |= 0x01;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            buttons |= 0x02;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            buttons |= 0x04;
        }
        platform->mouseCallback(static_cast<float>(xpos), static_cast<float>(ypos), buttons);
    }
}

void Window::mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto* platform = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (platform->mouseCallback)
    {
        uint32_t buttons = 0;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            buttons |= 0x01;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            buttons |= 0x02;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            buttons |= 0x04;
        }
        platform->mouseCallback(static_cast<float>(xpos), static_cast<float>(ypos), buttons);
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* platform = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (platform->keyboardCallback)
    {
        std::cout << "key pressed" << std::endl;
        platform->keyboardCallback(key, action != GLFW_RELEASE);
    }
}