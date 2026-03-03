#pragma once

#include <glm/glm.hpp>

namespace Yume {

class BoundingBox {
public: 
    void Transform(const glm::mat4& transformMatrix) const;
};

}