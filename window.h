#pragma once

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
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

        void Init(const std::string &name, int height, int width);
        GLFWwindow* getGLFWwindow() { return window; }
        bool shouldClose() { return glfwWindowShouldClose(window); }

        bool framebufferResized;
    private:
        GLFWwindow* window;
        int width;
        int height;
        
};