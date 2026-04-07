#pragma once 

#include <stdexcept>
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class Renderer;
struct ImGuiContext;

class ImguiSystem
{
    public:
        ImguiSystem() = default;

        ImguiSystem(Renderer* renderer, int width, int height)
        {
            if (!init(renderer, width, height))
            {
                throw std::runtime_error("Imgui init failed");
            }
        }

        ~ImguiSystem() = default;

        void Render(vk::raii::CommandBuffer& commandBuffer, uint32_t frameIndex);
        void updateBuffers(uint32_t frameIndex);

    private:

        bool init(Renderer* renderer, int width, int height);
        bool createRessources();

        ImGuiContext* _context = nullptr;
        Renderer* _renderer = nullptr;
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