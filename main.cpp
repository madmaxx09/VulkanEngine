#include <iostream>
#include <stdexcept>


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.h>
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "external/imgui/imgui.h"

#include "engine.h"

int main()
{
    try
    {
        Engine engine;
        if (!engine.Start("compile", 1280, 800))
            throw std::runtime_error("Engine init failed");
    
        engine.Run();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

}