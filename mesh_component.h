#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "component.h"

struct Vertex
{
	glm::vec3 pos;
    glm::vec3 normal;
	glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord;
    }

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
	}

	static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		return {
		    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
		    vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))
        };
	}
};



class MeshComponent final : public Component
{
    private:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::string texturePath;

    explicit MeshComponent(const std::string &name = "MeshComponent") : Component(name)
    {

    }

    void setTexturePath(const std::string &name)
    {
        texturePath = name;
    }

    void setVertices(const std::vector<Vertex> &newVertices)
    {
        vertices = newVertices;
    }

    void setIndices(const std::vector<uint32_t> &newIndices)
    {
        indices = newIndices;
    }
};

