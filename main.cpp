#include <iostream>
#include <stdexcept>

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.h>
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "external/imgui/imgui.h"

#include "engine.h"

int main()
{
    std::cout << VK_HEADER_VERSION << std::endl;
    try
    {
        Engine engine;
        engine.Start("Max Engine", 1280, 800);
    
        engine.Run();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}