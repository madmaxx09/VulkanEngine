#include "camera.h"

void CameraSystem::setPosition(glm::vec3 newPosition)
{
    position = newPosition;
    viewMatrixDirty = true;
    projectionMatrixDirty = true;
}

// CameraSystem::CameraSystem()
// {
    
// }

const glm::mat4 &CameraSystem::getViewMatrix()
{
    if (viewMatrixDirty)
        updateViewMatrix();
    return viewMatrix;
}

const glm::mat4 &CameraSystem::getProjectionMatrix()
{
    if (projectionMatrixDirty)
        updateProjectionMatrix();
    return projectionMatrix;
}

void CameraSystem::updateViewMatrix()
{
    // Build camera world transform (T * R) from the camera entity's transform
    // and compute the view matrix as its inverse. This ensures consistency
    // with rasterization and avoids relying on an external target vector.

    const glm::quat qx = glm::angleAxis(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::quat qy = glm::angleAxis(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat qz = glm::angleAxis(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::quat q  = qz * qy * qx;        // match TransformComponent's ZYX composition

    const glm::mat4 T            = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 R            = glm::mat4_cast(q);
    const glm::mat4 worldNoScale = T * R;

    viewMatrix = glm::inverse(worldNoScale);

    viewMatrixDirty =  false;
}

void CameraSystem::updateProjectionMatrix()
{
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);


    //if orthograhic 
    // float halfWidth  = orthoWidth * 0.5f;
    // float halfHeight = orthoHeight * 0.5f;
    // projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
	projectionMatrixDirty = false;
}

