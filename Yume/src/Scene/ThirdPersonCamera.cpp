#include "ThirdPersonCamera.h"
#include "Scene.h"

#include <glm/glm.hpp>
#include <algorithm>

namespace Yume {

void ThirdPersonCamera::UpdatePosition(
    const glm::vec3& targetPos,
    const glm::vec3& targetFwd,
    float deltaTime
) {
    // Update target properties
    m_targetPosition = targetPos;
    m_targetForward = glm::normalize(targetFwd);

    // Calculate the desired camera position
    // Position the camera behind and above the character
    glm::vec3 offset = -m_targetForward * m_followDistance;
    offset.y = m_followHeight;

    m_desiredPosition = m_targetPosition + offset;

    // Smooth camera movement using exponential smoothing
    m_position = glm::mix(m_position, m_desiredPosition, 1.0f - pow(m_followSmoothness, deltaTime * 60.0f));

    // Update the camera to look at the target
    m_front = glm::normalize(m_targetPosition - m_position);

    // Recalculate right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void ThirdPersonCamera::HandleOcclusion(const Scene& scene) {
    // Cast a ray from the target to the desired camera position
    Ray ray;
    ray.origin = m_targetPosition;
    ray.direction = glm::normalize(m_desiredPosition - m_targetPosition);

    // Check for intersections with scene objects
    RaycastHit hit;
    if (scene.raycast(ray, hit, glm::length(m_desiredPosition - m_targetPosition))) {
        // If there's an intersection, move the camera to the hit point
        // minus a small offset to avoid clipping
        float offsetDistance = 0.2f;
        m_position = hit.point - (ray.direction * offsetDistance);

        // Ensure we don't get too close to the target
        float currentDistance = glm::length(m_position - m_targetPosition);
        if (currentDistance < m_minDistance) {
            m_position = m_targetPosition + ray.direction * m_minDistance;
        }

        // Update the camera to look at the target
        m_front = glm::normalize(m_targetPosition - m_position);
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }
}

void ThirdPersonCamera::Orbit(float horizontalAngle, float verticalAngle) {
    // Update yaw and pitch based on input
    m_yaw += horizontalAngle;
    m_pitch += verticalAngle;

    // Constrain pitch to avoid flipping
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    // Calculate the new camera position based on spherical coordinates
    float radius = m_followDistance;
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    // Convert spherical coordinates to Cartesian
    glm::vec3 offset;
    offset.x = radius * cos(yawRad) * cos(pitchRad);
    offset.y = radius * sin(pitchRad);
    offset.z = radius * sin(yawRad) * cos(pitchRad);

    // Set the desired position
    m_desiredPosition = m_targetPosition + offset;

    // Update camera vectors
    m_front = glm::normalize(m_targetPosition - m_desiredPosition);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

}