#pragma once

#include "BoundingBox.h"

#include <glm/glm.hpp>

namespace Yume {

class Frustum {
public:
    bool Intersects(const BoundingBox& box) const;
};

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
protected:
    // Spatial positioning and orientation vectors
    // These form the camera's local coordinate system in world space
    glm::vec3 m_position;     // Camera's location in world coordinates
    glm::vec3 m_front;        // Forward direction (where camera is looking)
    glm::vec3 m_up;           // Camera's local up direction (for roll control)
    glm::vec3 m_right;        // Camera's local right direction (perpendicular to front and up)
    glm::vec3 m_worldUp;      // Global up vector reference (typically Y-axis)

    // Rotation representation using Euler angles
    // Provides intuitive control while managing gimbal lock and other rotation complexities
    float m_yaw;              // Horizontal rotation around the world up-axis (left-right looking)
    float m_pitch;            // Vertical rotation around the camera's right axis (up-down looking)

    // User interaction and behavior parameters
    // These control how the camera responds to input and environmental factors
    float m_movementSpeed;    // Units per second for translation movement
    float m_mouseSensitivity; // Multiplier for mouse input to rotation angle conversion
    float m_zoom;             // Field of view control for perspective projection

    // Internal coordinate system maintenance
    // Ensures mathematical consistency when orientation changes occur
    void updateCameraVectors();

    // Matrix generation for graphics pipeline integration
    // These methods bridge between the camera's spatial representation and GPU requirements
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

public:
    // Constructor with sensible defaults for common use cases
    // Provides flexibility while ensuring the camera starts in a predictable state
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),  // Start at world origin
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),        // Y-axis as world up
        float yaw = -90.0f,                                 // Look along negative Z-axis (OpenGL convention)
        float pitch = 0.0f                                  // Level horizon
    );

    // Input processing methods for different interaction modalities
    // Each method handles a specific type of user input with appropriate transformations
    void ProcessKeyboard(CameraMovement direction, float deltaTime);     // Keyboard-based translation
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);  // Mouse-based rotation
    void ProcessMouseScroll(float yOffset);                              // Scroll-based zoom control

    // Property access methods for external systems
    // Provide controlled access to internal state without exposing implementation details
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetFront() const { return m_front; }
    float GetZoom() const { return m_zoom; }

    Frustum GetFrustum() const;
};

}