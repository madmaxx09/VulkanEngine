#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "window.h"
#include <ranges>
#include <vector>
#include <algorithm>
#include <map>
#include <iterator>
#include <cassert>
#include <limits>
#include <fstream>

class Renderer
{
    public:
        Renderer(Window &window) : window(window) {};
        ~Renderer() = default;

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
        {
            if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            {
                std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
            }

            return vk::False;
        }

        void init();
        void idle() { device.waitIdle(); }
        void drawFrame();

        private:
        Window &window;
        
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        
        vk::raii::PhysicalDevice physicalDevice = nullptr;
        vk::raii::Device device = nullptr;
        uint32_t                         queueIndex     = ~0;
        
        vk::raii::Queue queue = nullptr;
        
        vk::raii::SwapchainKHR           swapChain      = nullptr;
        std::vector<vk::Image>           swapChainImages;
        vk::SurfaceFormatKHR             swapChainSurfaceFormat;
        vk::Extent2D                     swapChainExtent;
        std::vector<vk::raii::ImageView> swapChainImageViews;
        
        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline       graphicsPipeline = nullptr;
        vk::raii::CommandPool    commandPool      = nullptr;
	    vk::raii::CommandBuffer  commandBuffer    = nullptr;
        
        vk::raii::Semaphore presentCompleteSemaphore = nullptr;
        vk::raii::Semaphore renderFinishedSemaphore = nullptr;
        vk::raii::Fence drawFence = nullptr;
        

        void initVulkan();
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();
        void createGraphicsPipeline();
        void createCommandPool();
	    void createCommandBuffer();
        void createSyncObjects();

        void recordCommandBuffer(uint32_t imageIndex);
        void transition_image_layout(
            uint32_t                imageIndex,
            vk::ImageLayout         old_layout,
            vk::ImageLayout         new_layout,
            vk::AccessFlags2        src_access_mask,
            vk::AccessFlags2        dst_access_mask,
            vk::PipelineStageFlags2 src_stage_mask,
            vk::PipelineStageFlags2 dst_stage_mask);
        

        vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;


        std::vector<const char*> getRequiredInstanceExtensions();
        
};