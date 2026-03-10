#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>
#include <iostream>

class Window
{
    public:
        Window() = default;
        ~Window() = default;

        bool Init(const std::string &name, int height, int width);
        GLFWwindow* getGLFWwindow() { return window; }
        bool shouldClose() { return glfwWindowShouldClose(window); }
    private:
        GLFWwindow* window;
        
};