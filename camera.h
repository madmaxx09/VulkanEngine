#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class CameraSystem
{
    public:
        CameraSystem() = default;
        //CameraSystem();
        ~CameraSystem() = default;

        // Matrix generation for graphics pipeline integration
        // These methods bridge between the camera's spatial representation and GPU requirements
        const glm::mat4 &getViewMatrix();
        const glm::mat4 &getProjectionMatrix();

        void updateViewMatrix();
        void updateProjectionMatrix();

        void forceMatrixUpdate();


        glm::vec3 getPosition() const { return position; }
        void setPosition(const glm::vec3 &newPosition);

        glm::vec3 getRotation() const { return rotation; }
        void setRotation(const glm::vec3 &newRotation);

    private:
        glm::vec3 position = {0.0f, 0.0f, 4.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 up     = {0.0f, 1.0f, 0.0f};

       	// Matrices
        glm::mat4 viewMatrix       = glm::mat4(1.0f);
        glm::mat4 projectionMatrix = glm::mat4(1.0f);

        float fov = 45.0f;
        float aspectRatio = 16.0f / 9.0f;

        float orthoWidth  = 10.0f;
	    float orthoHeight = 10.0f;

        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        bool viewMatrixDirty       = true;
	    bool projectionMatrixDirty = true;
        
};