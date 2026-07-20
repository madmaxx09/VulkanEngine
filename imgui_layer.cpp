#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "imgui_layer.h"
#include <iostream>
#include "renderer.h"
#include "engine.h"
#include "transform_component.h"
#include "mesh_component.h"

bool ImguiSystem::init(Engine* engine, Renderer* renderer, int width, int height)
{

    _renderer = renderer;
    _engine = engine;

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

void ImguiSystem::HandleChar(uint32_t c)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

bool ImguiSystem::ImguiWantsMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

static ImGuiKey ConvertKey(uint32_t key)
{
    switch (key)
    {
        // Navigation
        case GLFW_KEY_TAB:         return ImGuiKey_Tab;
        case GLFW_KEY_LEFT:        return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT:       return ImGuiKey_RightArrow;
        case GLFW_KEY_UP:          return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN:        return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP:     return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN:   return ImGuiKey_PageDown;
        case GLFW_KEY_HOME:        return ImGuiKey_Home;
        case GLFW_KEY_END:         return ImGuiKey_End;
        case GLFW_KEY_INSERT:      return ImGuiKey_Insert;
        case GLFW_KEY_DELETE:      return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE:   return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE:       return ImGuiKey_Space;
        case GLFW_KEY_ENTER:       return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE:      return ImGuiKey_Escape;

        // Modifiers
        case GLFW_KEY_LEFT_SHIFT:      return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL:    return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT:        return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER:      return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT:     return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL:   return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT:       return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER:     return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU:            return ImGuiKey_Menu;

        // Numbers
        case GLFW_KEY_0: return ImGuiKey_0;
        case GLFW_KEY_1: return ImGuiKey_1;
        case GLFW_KEY_2: return ImGuiKey_2;
        case GLFW_KEY_3: return ImGuiKey_3;
        case GLFW_KEY_4: return ImGuiKey_4;
        case GLFW_KEY_5: return ImGuiKey_5;
        case GLFW_KEY_6: return ImGuiKey_6;
        case GLFW_KEY_7: return ImGuiKey_7;
        case GLFW_KEY_8: return ImGuiKey_8;
        case GLFW_KEY_9: return ImGuiKey_9;

        // Letters
        case GLFW_KEY_A: return ImGuiKey_A;
        case GLFW_KEY_B: return ImGuiKey_B;
        case GLFW_KEY_C: return ImGuiKey_C;
        case GLFW_KEY_D: return ImGuiKey_D;
        case GLFW_KEY_E: return ImGuiKey_E;
        case GLFW_KEY_F: return ImGuiKey_F;
        case GLFW_KEY_G: return ImGuiKey_G;
        case GLFW_KEY_H: return ImGuiKey_H;
        case GLFW_KEY_I: return ImGuiKey_I;
        case GLFW_KEY_J: return ImGuiKey_J;
        case GLFW_KEY_K: return ImGuiKey_K;
        case GLFW_KEY_L: return ImGuiKey_L;
        case GLFW_KEY_M: return ImGuiKey_M;
        case GLFW_KEY_N: return ImGuiKey_N;
        case GLFW_KEY_O: return ImGuiKey_O;
        case GLFW_KEY_P: return ImGuiKey_P;
        case GLFW_KEY_Q: return ImGuiKey_Q;
        case GLFW_KEY_R: return ImGuiKey_R;
        case GLFW_KEY_S: return ImGuiKey_S;
        case GLFW_KEY_T: return ImGuiKey_T;
        case GLFW_KEY_U: return ImGuiKey_U;
        case GLFW_KEY_V: return ImGuiKey_V;
        case GLFW_KEY_W: return ImGuiKey_W;
        case GLFW_KEY_X: return ImGuiKey_X;
        case GLFW_KEY_Y: return ImGuiKey_Y;
        case GLFW_KEY_Z: return ImGuiKey_Z;

        // Function keys
        case GLFW_KEY_F1:  return ImGuiKey_F1;
        case GLFW_KEY_F2:  return ImGuiKey_F2;
        case GLFW_KEY_F3:  return ImGuiKey_F3;
        case GLFW_KEY_F4:  return ImGuiKey_F4;
        case GLFW_KEY_F5:  return ImGuiKey_F5;
        case GLFW_KEY_F6:  return ImGuiKey_F6;
        case GLFW_KEY_F7:  return ImGuiKey_F7;
        case GLFW_KEY_F8:  return ImGuiKey_F8;
        case GLFW_KEY_F9:  return ImGuiKey_F9;
        case GLFW_KEY_F10: return ImGuiKey_F10;
        case GLFW_KEY_F11: return ImGuiKey_F11;
        case GLFW_KEY_F12: return ImGuiKey_F12;

        // Keypad
        case GLFW_KEY_KP_0:         return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1:         return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2:         return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3:         return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4:         return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5:         return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6:         return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7:         return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8:         return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9:         return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL:   return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE:    return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY:  return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT:  return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD:       return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER:     return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL:     return ImGuiKey_KeypadEqual;

        default:
            return ImGuiKey_None;
    }
}

void ImguiSystem::HandleKeyboard(uint32_t key, bool pressed)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imguiKey = ConvertKey(key);
    
    if (imguiKey != ImGuiKey_None)
        io.AddKeyEvent(imguiKey, pressed);
}

bool ImguiSystem::ImGuiWantsKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImguiSystem::NewFrame(std::vector<std::unique_ptr<Entity>> &entities)
{
    ImGui::NewFrame();
    ImGui::Begin("Vulkan ImGui Demo");
    ImGui::Text("Hello, Vulkan!");
    if (ImGui::Button("Create entity"))
        ImGui::OpenPopup("New Entity");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("New Entity", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char entityNameBuffer[128] = "Entity";
        static char placeholderName[128] = "Entity";
        static int entityNumber = 1;
        ImGui::Text("Entity name:");
        //ImGui::SameLine(0.0f, 10.0f);
        ImGui::InputText("##entityName", entityNameBuffer, IM_COUNTOF(entityNameBuffer));
        
        if (ImGui::Button("Create"))
        {
            _engine->createEntity(entityNameBuffer);
            std::strcpy(entityNameBuffer, placeholderName);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    ImGui::End();

    ImGui::Begin("Entities");

    //ImGui::BeginChild("EntityList", ImVec2(0, 200), true);
    for (auto& entity : entities)
    {
        if (ImGui::TreeNode(entity.get(), "%s", entity.get()->GetName().c_str()))
        {
            auto transformComp = entity->GetComponent<TransformComponent>();
            auto meshComp = entity->GetComponent<MeshComponent>();
            if (transformComp != nullptr)
            {
                if (ImGui::TreeNode(transformComp, "%s", transformComp->GetName().c_str()))
                {
                    glm::vec3 position = transformComp->GetPosition();
                    if (ImGui::DragFloat3("Position", &position.x, 0.1f))
                        transformComp->SetPosition(position);
    
                    glm::vec3 rotation = transformComp->GetRotation();
                    if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
                        transformComp->SetRotation(rotation);
    
                    glm::vec3 scale = transformComp->GetScale();
                    if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                        transformComp->SetScale(scale);
                    if (ImGui::Button("Delete component"))
                    {
                        ImGui::OpenPopup("Are you sure ?");
                    }
                    if (ImGui::BeginPopupModal("Are you sure ?"))
                    {
                        if (ImGui::Button("Yes"))
                        {
                            entity->RemoveComponent<TransformComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel"))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    ImGui::TreePop();
                }

            }

            if (ImGui::Button("AddComponent"))
            {
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent"))
            {
                if (!transformComp)
                {
                    if (ImGui::MenuItem("TransformComponent"))
                        entity->AddComponent<TransformComponent>();
                }
                if (!meshComp)
                {
                    if (ImGui::MenuItem("Model"))
                        _engine->loadGltfModel(std::string("./models/Avocado.glb"));
                }

                ImGui::EndPopup();
            }
            ImGui::TreePop();
        }
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