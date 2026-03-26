#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include "window.h"
#include <ranges>
#include <vector>
#include <array>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <iterator>
#include <cassert>
#include <limits>
#include <fstream>
#include <memory>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>




#include <chrono>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		return {
		    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
		    vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))
        };
	}
};

template <>
struct std::hash<Vertex>
{
	size_t operator()(Vertex const &vertex) const noexcept
	{
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
        void cleanup();
        void idle() { device.waitIdle(); }
        void drawFrame();

        bool framebufferResized = false;

    private:

        const std::string MODEL_PATH = "models/viking_room.obj";
        const std::string TEXTURE_PATH = "textures/viking_room.png";

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
        
        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline       graphicsPipeline = nullptr;
        vk::raii::CommandPool    commandPool      = nullptr;

        vk::raii::Image textureImage = nullptr;
        vk::raii::DeviceMemory textureImageMemory = nullptr;
        vk::raii::ImageView textureImageView = nullptr;
        vk::raii::Sampler textureSampler = nullptr;

        vk::raii::Image depthImage = nullptr;
        vk::raii::DeviceMemory depthImageMemory = nullptr;
        vk::raii::ImageView depthImageView = nullptr;

        vk::raii::Image colorImage = nullptr;
        vk::raii::DeviceMemory colorImageMemory = nullptr;
        vk::raii::ImageView colorImageView = nullptr;

	    std::vector<vk::raii::CommandBuffer> commandBuffers;
        std::vector<vk::raii::Buffer> uniformBuffers;
        std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;
        vk::raii::DescriptorPool descriptorPool = nullptr;
        std::vector<vk::raii::DescriptorSet> descriptorSets;

        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
        std::vector<vk::raii::Fence> inFlightFences;
        
        uint32_t                         frameIndex = 0;
        vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

        //Model related variables
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vk::raii::Buffer vertexBuffer = nullptr;
        vk::raii::DeviceMemory vertexBufferMemory = nullptr;
        vk::raii::Buffer indexBuffer = nullptr;
        vk::raii::DeviceMemory indexBufferMemory = nullptr;
        uint32_t mipLevels;


        

        void initVulkan();


        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();

        void createSwapChain();
        void recreateSwapChain();
        void cleanupSwapChain();

        void createImageViews();
        vk::raii::ImageView createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createDepthResources();
        bool hasStencilComponent(vk::Format format);
        vk::Format findDepthFormat();
        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory);
        void transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
        void copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);
        void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void createColorRessources();

        std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
        void endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer);



        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
        void copyBuffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & dstBuffer, vk::DeviceSize size);
        
        void loadModel();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void updateUniformBuffer(uint32_t currentImage);
	    void createCommandBuffers();
        void createSyncObjects();

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

        void recordCommandBuffer(uint32_t imageIndex);
        void transition_image_layout(
            vk::Image               image,
            vk::ImageLayout         old_layout,
            vk::ImageLayout         new_layout,
            vk::AccessFlags2        src_access_mask,
            vk::AccessFlags2        dst_access_mask,
            vk::PipelineStageFlags2 src_stage_mask,
            vk::PipelineStageFlags2 dst_stage_mask,
            vk::ImageAspectFlags    image_aspect_flags);
        

        vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;


        std::vector<const char*> getRequiredInstanceExtensions();
        vk::SampleCountFlagBits getMaxUsableSampleCount();
};