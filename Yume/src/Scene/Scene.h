#pragma once

#include <glm/glm.hpp>

namespace Yume {

struct RaycastHit {
    glm::vec3 point;
};

struct Ray {
    // Ray definition with origin and direction
    glm::vec3 origin;
    glm::vec3 direction;
};

class Scene {

public:
    Scene();
    ~Scene();

    bool raycast(const Ray& ray, RaycastHit& hit, float maxDistance) const {
        return false;
    }

};

}