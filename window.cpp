#include "window.h"
#include <iostream>

bool Window::Init(const std::string &name, int height, int width)
{
    if (!glfwInit()) 
    {
        throw std::runtime_error("GLFW init fail");
        return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); 

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        throw std::runtime_error("GLFW createwindow fail");
        return false;
    }
    glfwSetWindowUserPointer(window, this);
    return true;
}