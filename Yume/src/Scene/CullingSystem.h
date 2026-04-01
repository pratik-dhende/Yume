#pragma once

#include "Scene/Camera.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"

namespace Yume {

class CullingSystem {

public:
    explicit CullingSystem(Camera* camera = nullptr) : m_camera(camera) {}

    void SetCamera(Camera* camera) {
        m_camera = camera;
    }

    void CullScene(const std::vector<Entity*>& allEntities) {
        m_visibleEntities.clear();

        if (!m_camera) return;

        Frustum frustum = m_camera->GetFrustum();

        for (auto entity : allEntities) {
            if (!entity->IsActive()) continue;

            auto meshComponent = entity->GetComponent<MeshComponent>();
            if (!meshComponent) continue;

            auto transformComponent = entity->GetComponent<TransformComponent>();
            if (!transformComponent) continue;

            BoundingBox boundingBox = meshComponent->GetBoundingBox();

            boundingBox.Transform(transformComponent->GetTransformMatrix());

            if (frustum.Intersects(boundingBox)) {
                m_visibleEntities.push_back(entity);
            }
        }
    }

    const std::vector<Entity*>& GetVisibleEntities() const {
        return m_visibleEntities;
    }

private:
    Camera* m_camera;
    std::vector<Entity*> m_visibleEntities;
};

}