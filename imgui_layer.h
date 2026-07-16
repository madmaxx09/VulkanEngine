#pragma once 

#include <stdexcept>
#include <vector>
#include <memory>
#include "entity.h"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>


class Renderer;
class Engine;
struct ImGuiContext;

class ImguiSystem
{
    public:
        ImguiSystem() = default;

        ImguiSystem(Engine* engine, Renderer* renderer, int width, int height)
        {
            if (!init(engine, renderer, width, height))
            {
                throw std::runtime_error("Imgui init failed");
            }
        }

        ~ImguiSystem() = default;

        void NewFrame(std::vector<std::unique_ptr<Entity>> &entities);
        void Render(vk::raii::CommandBuffer& commandBuffer, uint32_t frameIndex);
        void updateBuffers(uint32_t frameIndex);

        //input handling
        void HandleMouse(float x, float y, uint32_t buttons);
        void HandleKeyboard(uint32_t key, bool pressed);
        void HandleChar(uint32_t c);
        bool ImguiWantsMouse() const;
        bool ImGuiWantsKeyboard() const;

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock;

        bool needsUpdateBuffers = false;

        // Modern Vulkan rendering configuration
        vk::PipelineRenderingCreateInfo renderingInfo{};        // Dynamic rendering setup parameters
        vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;   // Target framebuffer format

    private:

        bool init(Engine* engine, Renderer* renderer, int width, int height);
        bool createRessources();

        bool createFontTexture();
        bool createDescriptorSetLayout();
        bool createDescriptorPool();
        bool createDescriptorSet();
        bool createPipelineLayout();
        bool createPipeline();



        ImGuiContext* _context = nullptr;
        Renderer* _renderer = nullptr;
        Engine* _engine = nullptr;
        

        //Mouse state
        float mouseX = 0.0f;
        float mouseY = 0.0f;
        uint32_t mouseButtons = 0;

        // Vulkan resources
        vk::raii::DescriptorPool descriptorPool = nullptr;
        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::DescriptorSet descriptorSet = nullptr;
        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::Sampler fontSampler = nullptr;
        vk::raii::Image fontImage = nullptr;
        vk::raii::DeviceMemory fontMemory = nullptr;
        vk::raii::ImageView fontView = nullptr;
        // Per-frame dynamic buffers to avoid GPU/CPU contention when frames are in flight
        std::vector<vk::raii::Buffer> vertexBuffers;
        std::vector<vk::raii::DeviceMemory> vertexBufferMemories;
        std::vector<vk::raii::Buffer> indexBuffers;
        std::vector<vk::raii::DeviceMemory> indexBufferMemories;
        std::vector<uint32_t> vertexCounts;
        std::vector<uint32_t> indexCounts;
};