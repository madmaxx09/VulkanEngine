#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "imgui_layer.h"
#include <iostream>
#include "renderer.h"

bool ImguiSystem::init(Renderer* renderer, int width, int height)
{

    _renderer = renderer;

    // Initialize ImGui context
    _context = ImGui::CreateContext();

    // Configure ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable docking

    // Set display size
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Set up ImGui style
    ImGui::StyleColorsDark();
    
    if (!createRessources())
        return false;

    if (_renderer)
    {
        uint32_t frames = MAX_FRAMES_IN_FLIGHT;
        vertexBuffers.clear();
        vertexBuffers.reserve(frames);
        vertexBufferMemories.clear();
        vertexBufferMemories.reserve(frames);
        indexBuffers.clear();
        indexBuffers.reserve(frames);
        indexBufferMemories.clear();
        indexBufferMemories.reserve(frames);
        for (uint32_t i = 0; i < frames; ++i)
        {
            vertexBuffers.emplace_back(nullptr);
            vertexBufferMemories.emplace_back(nullptr);
            indexBuffers.emplace_back(nullptr);
            indexBufferMemories.emplace_back(nullptr);
        }
        vertexCounts.assign(frames, 0);
        indexCounts.assign(frames, 0);
    }

    return true;
}

void ImguiSystem::HandleMouse(float x, float y, uint32_t buttons)
{
    ImGuiIO& io = ImGui::GetIO();

    io.MousePos = ImVec2(x, y); 

    io.MouseDown[0] = (buttons & 0x01) != 0; // Left button
    io.MouseDown[1] = (buttons & 0x02) != 0; // Right button
    io.MouseDown[2] = (buttons & 0x04) != 0; // Middle button
}

bool ImguiSystem::ImguiWantsMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

void ImguiSystem::NewFrame()
{
    ImGui::NewFrame();
    ImGui::Begin("Vulkan ImGui Demo");
    ImGui::Text("Hello, Vulkan!");
    if (ImGui::Button("Click me!")) {
        std::cout << "clicked" << std::endl;
    }
    ImGui::End();
}

void ImguiSystem::Render(vk::raii::CommandBuffer& commandBuffer, uint32_t frameIndex)
{
    ImGui::Render();

    updateBuffers(frameIndex);

    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData || drawData->CmdListsCount == 0) {
        return;
    }
    try {
        // Bind the pipeline
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

        // Set viewport
        vk::Viewport viewport;
        viewport.width = ImGui::GetIO().DisplaySize.x;
        viewport.height = ImGui::GetIO().DisplaySize.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        commandBuffer.setViewport(0, {viewport});

        // Set push constants
        struct PushConstBlock {
        float scale[2];
        float translate[2];
        } pushConstBlock{};

        pushConstBlock.scale[0] = 2.0f / ImGui::GetIO().DisplaySize.x;
        pushConstBlock.scale[1] = 2.0f / ImGui::GetIO().DisplaySize.y;
        pushConstBlock.translate[0] = -1.0f;
        pushConstBlock.translate[1] = -1.0f;

        commandBuffer.pushConstants<PushConstBlock>(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstBlock);

        // Bind vertex and index buffers for this frame
        commandBuffer.bindVertexBuffers(0, *vertexBuffers[frameIndex], vk::DeviceSize{0});
        commandBuffer.bindIndexBuffer(*indexBuffers[frameIndex], 0, vk::IndexType::eUint16);

        // Render command lists
        int vertexOffset = 0;
        int indexOffset = 0;

        for (int i = 0; i < drawData->CmdListsCount; i++) {
        const ImDrawList* cmdList = drawData->CmdLists[i];

        for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];

            // Set scissor rectangle
            vk::Rect2D scissor;
            scissor.offset.x = std::max(static_cast<int32_t>(pcmd->ClipRect.x), 0);
            scissor.offset.y = std::max(static_cast<int32_t>(pcmd->ClipRect.y), 0);
            scissor.extent.width = static_cast<uint32_t>(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissor.extent.height = static_cast<uint32_t>(pcmd->ClipRect.w - pcmd->ClipRect.y);
            commandBuffer.setScissor(0, {scissor});

            // Bind descriptor set (font texture)
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, {*descriptorSet}, {});

            // Draw
            commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }

        vertexOffset += cmdList->VtxBuffer.Size;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to render ImGui: " << e.what() << std::endl;
    }
}

void ImguiSystem::updateBuffers(uint32_t frameIndex)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData || drawData->CmdListsCount == 0) {
        return;
    }

    try {
        const vk::raii::Device& device = _renderer->getDevice();

        // Calculate required buffer sizes
        vk::DeviceSize vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        vk::DeviceSize indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

        // Resize buffers if needed for this frame
        if (frameIndex >= vertexCounts.size())
            return; // Safety

        if (static_cast<uint32_t>(drawData->TotalVtxCount) > vertexCounts[frameIndex]) {
            // Clean up old buffer
            vertexBuffers[frameIndex] = vk::raii::Buffer(nullptr);
            vertexBufferMemories[frameIndex] = vk::raii::DeviceMemory(nullptr);

            // Create new vertex buffer
            vk::BufferCreateInfo bufferInfo;
            bufferInfo.size = vertexBufferSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;

            vertexBuffers[frameIndex] = vk::raii::Buffer(device, bufferInfo);

            vk::MemoryRequirements memRequirements = vertexBuffers[frameIndex].getMemoryRequirements();

            vk::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = _renderer->findMemoryType(memRequirements.memoryTypeBits,
                                                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            vertexBufferMemories[frameIndex] = vk::raii::DeviceMemory(device, allocInfo);
            vertexBuffers[frameIndex].bindMemory(*vertexBufferMemories[frameIndex], 0);
            vertexCounts[frameIndex] = drawData->TotalVtxCount;
        }

        if (static_cast<uint32_t>(drawData->TotalIdxCount) > indexCounts[frameIndex]) {
            // Clean up old buffer
            indexBuffers[frameIndex] = vk::raii::Buffer(nullptr);
            indexBufferMemories[frameIndex] = vk::raii::DeviceMemory(nullptr);

            // Create new index buffer
            vk::BufferCreateInfo bufferInfo;
            bufferInfo.size = indexBufferSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;

            indexBuffers[frameIndex] = vk::raii::Buffer(device, bufferInfo);

            vk::MemoryRequirements memRequirements = indexBuffers[frameIndex].getMemoryRequirements();

            vk::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = _renderer->findMemoryType(memRequirements.memoryTypeBits,
                                                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            indexBufferMemories[frameIndex] = vk::raii::DeviceMemory(device, allocInfo);
            indexBuffers[frameIndex].bindMemory(*indexBufferMemories[frameIndex], 0);
            indexCounts[frameIndex] = drawData->TotalIdxCount;
        }

        // Upload data to buffers for this frame (only if we have data to upload)
        if (drawData->TotalVtxCount > 0 && drawData->TotalIdxCount > 0) {
            void* vtxMappedMemory = vertexBufferMemories[frameIndex].mapMemory(0, vertexBufferSize);
            void* idxMappedMemory = indexBufferMemories[frameIndex].mapMemory(0, indexBufferSize);

            ImDrawVert* vtxDst = static_cast<ImDrawVert *>(vtxMappedMemory);
            ImDrawIdx* idxDst = static_cast<ImDrawIdx *>(idxMappedMemory);

            for (int n = 0; n < drawData->CmdListsCount; n++) {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmdList->VtxBuffer.Size;
            idxDst += cmdList->IdxBuffer.Size;
            }

            vertexBufferMemories[frameIndex].unmapMemory();
            indexBufferMemories[frameIndex].unmapMemory();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to update buffers: " << e.what() << std::endl;
    }
}

bool ImguiSystem::createRessources()
{
    if (!createFontTexture()) {
        return false;
    }

    if (!createDescriptorSetLayout()) {
        return false;
    }

    if (!createDescriptorPool()) {
        return false;
    }

    if (!createDescriptorSet()) {
        return false;
    }

    if (!createPipelineLayout()) {
        return false;
    }

    if (!createPipeline()) {
        return false;
    }

    return true;
}

bool ImguiSystem::createFontTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

    try {
        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.format = vk::Format::eR8G8B8A8Unorm;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;

        const vk::raii::Device& device = _renderer->getDevice();
        fontImage = vk::raii::Image(device, imageInfo);

        // Allocate memory for the image
        vk::MemoryRequirements memRequirements = fontImage.getMemoryRequirements();

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _renderer->findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        fontMemory = vk::raii::DeviceMemory(device, allocInfo);
        fontImage.bindMemory(*fontMemory, 0);

        // Create a staging buffer for uploading the font data
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = uploadSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::raii::Buffer stagingBuffer(device, bufferInfo);

        vk::MemoryRequirements stagingMemRequirements = stagingBuffer.getMemoryRequirements();

        vk::MemoryAllocateInfo stagingAllocInfo;
        stagingAllocInfo.allocationSize = stagingMemRequirements.size;
        stagingAllocInfo.memoryTypeIndex = _renderer->findMemoryType(stagingMemRequirements.memoryTypeBits,
                                                                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::raii::DeviceMemory stagingBufferMemory(device, stagingAllocInfo);
        stagingBuffer.bindMemory(*stagingBufferMemory, 0);

        // Copy font data to staging buffer
        void* data = stagingBufferMemory.mapMemory(0, uploadSize);
        memcpy(data, fontData, uploadSize);
        stagingBufferMemory.unmapMemory();
        // Transition image layout and copy data
        _renderer->transitionImageLayout(fontImage,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal);
        _renderer->copyBufferToImage(stagingBuffer,
                                    fontImage,
                                    static_cast<uint32_t>(texWidth),
                                    static_cast<uint32_t>(texHeight));
        _renderer->transitionImageLayout(fontImage,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal);

        // Staging buffer and memory will be automatically cleaned up by RAII

        // Create image view
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = *fontImage;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = vk::Format::eR8G8B8A8Unorm;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        fontView = vk::raii::ImageView(device, viewInfo);

        // Create sampler
        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        fontSampler = vk::raii::Sampler(device, samplerInfo);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create font texture: " << e.what() << std::endl;
        return false;
    }
}

bool ImguiSystem::createDescriptorSetLayout()
{
    try
    {
        vk::DescriptorSetLayoutBinding binding;
        binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        binding.descriptorCount = 1;
        binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        binding.binding = 0;

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        const vk::raii::Device& device = _renderer->getDevice();
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create desc set layout" << e.what() << std::endl;
        return false;
    }
}

bool ImguiSystem::createDescriptorPool()
{
    try {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        const vk::raii::Device& device = _renderer->getDevice();
        descriptorPool = vk::raii::DescriptorPool(device, poolInfo);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create descriptor pool: " << e.what() << std::endl;
        return false;
    }
}

bool ImguiSystem::createDescriptorSet()
{
    try {
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &(*descriptorSetLayout);

        const vk::raii::Device& device = _renderer->getDevice();
        vk::raii::DescriptorSets descriptorSets(device, allocInfo);
        descriptorSet = std::move(descriptorSets[0]); // Store the first (and only) descriptor set
        std::cout << "ImGui created descriptor set with handle: " << *descriptorSet << std::endl;

        // Update descriptor set
        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = *fontView;
        imageInfo.sampler = *fontSampler;

        vk::WriteDescriptorSet writeSet;
        writeSet.dstSet = *descriptorSet;
        writeSet.descriptorCount = 1;
        writeSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        writeSet.pImageInfo = &imageInfo;
        writeSet.dstBinding = 0;

        device.updateDescriptorSets({writeSet}, {});

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create descriptor set: " << e.what() << std::endl;
        return false;
    }
}

bool ImguiSystem::createPipelineLayout()
{
    try {
        // Push constant range for the transformation matrix
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(float) * 4; // 2 floats for scale, 2 floats for translate

        // Create pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &(*descriptorSetLayout);
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        const vk::raii::Device& device = _renderer->getDevice();
        pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create pipeline layout: " << e.what() << std::endl;
        return false;
    }
}

bool ImguiSystem::createPipeline()
{
    try {
        // Load shaders
        vk::raii::ShaderModule shaderModule = _renderer->createShaderModule("shaders/imgui.spv");

        // Shader stage creation
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = *shaderModule;
        vertShaderStageInfo.pName = "vertMain";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = *shaderModule;
        fragShaderStageInfo.pName = "fragMain";

        std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

        // Vertex input
        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ImDrawVert);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions;
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR8G8B8A8Unorm;
        attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Input assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Viewport and scissor
        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        viewportState.pViewports = nullptr; // Dynamic state
        viewportState.pScissors = nullptr; // Dynamic state

        // Rasterization
        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eNone;
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisampling
        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        // Depth and stencil testing
        vk::PipelineDepthStencilStateCreateInfo depthStencil;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // Color blending
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlending;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // Dynamic state
        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::Format depthFormat = _renderer->findDepthFormat();
        // Create the graphics pipeline with dynamic rendering
        vk::PipelineRenderingCreateInfo renderingInfo;
        renderingInfo.colorAttachmentCount = 1;
        vk::Format colorFormat = _renderer->getSwapChainImageFormat(); // Get the actual swapchain format
        renderingInfo.pColorAttachmentFormats = &colorFormat;
        renderingInfo.depthAttachmentFormat = depthFormat;

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = *pipelineLayout;
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.basePipelineHandle = nullptr;

        const vk::raii::Device& device = _renderer->getDevice();
        pipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create graphics pipeline: " << e.what() << std::endl;
        return false;
    }
}