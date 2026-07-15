#pragma once

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <functional>

class Window
{
    public:
        Window() = default;
        ~Window() = default;

        void Init(const std::string &name, int height, int width);
        GLFWwindow* getGLFWwindow() { return window; }
        bool shouldClose() { return glfwWindowShouldClose(window); }
        
        void setMouseCallback(std::function<void(float, float, uint32_t)> callback);
        static void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        void setKeyboardCallback(std::function < void(uint32_t, bool) > callback);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        void setCharCallback(std::function<void(uint32_t)> callback);
        static void charCallback(GLFWwindow* window, unsigned int codepoint);
        
        bool framebufferResized;
    private:
        GLFWwindow* window;
        int width;
        int height;

        std::function<void(float, float, uint32_t)> mouseCallback;
        std::function<void(uint32_t, bool)> keyboardCallback;
        std::function<void(uint32_t)> characterCallback;
};