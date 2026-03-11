#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "window.h"
#include <ranges>
#include <vector>
#include <algorithm>
#include <map>

class Renderer
{
    public:
        Renderer() = default;
        ~Renderer() = default;

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
        {
            if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            {
                std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
            }

            return vk::False;
        }



        void Init();
        void initVulkan();
        void setupDebugMessenger();
        void createInstance();
        void pickPhysicalDevice();
        std::vector<const char*> getRequiredInstanceExtensions();
    private:
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
        vk::raii::PhysicalDevice physicalDevice = nullptr;
        
};