#pragma once

#include <glm/glm.hpp>

struct CameraData
{
    glm::vec3 position;
    glm::vec3 target;
    float near, far;
    float fov;

    glm::vec4 proj;
    glm::vec4 view;
    glm::vec4 viewProj;
};

class CameraSystem
{
    public:
        CameraSystem() = default;
        ~CameraSystem() = default;
    private:
        void update();
};