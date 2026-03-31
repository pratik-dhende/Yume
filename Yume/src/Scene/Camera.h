#pragma once

#include "BoundingBox.h"

namespace Yume {

class Frustum {
public:
    bool Intersects(const BoundingBox& box) const;
};

class Camera {
public:
    Frustum GetFrustum() const;
};

}