#pragma once

#include "Component.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "BoundingBox.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Yume {

class TransformComponent : public Component {
public:
	void SetPosition(const glm::vec3& position) {
		m_position = position;
		m_transformDirty = true;
	}

	void SetRotation(const glm::quat& rotation) {
		m_rotation = rotation;
		m_transformDirty = true;
	}

	void SetScale(const glm::vec3& scale) {
		m_scale = scale;
		m_transformDirty = true;
	}

	const glm::vec3& GetPosition() const { return m_position; }
	const glm::quat& GetRotation() const { return m_rotation; }
	const glm::vec3& GetScale() const { return m_scale; }

	glm::mat4 GetTransformMatrix() const {
		if (m_transformDirty) {
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position);
			glm::mat4 rotationMatrix = glm::mat4_cast(m_rotation);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);

			m_transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
			m_transformDirty = false;
		}
		return m_transformMatrix;
	}

private:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);

	mutable glm::mat4 m_transformMatrix = glm::mat4(1.0f);
	mutable bool m_transformDirty = true;
};

class MeshComponent : public Component {
public:
	MeshComponent(Mesh* mesh, Material* material) : m_mesh(mesh), m_material(material) {}

	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void SetMaterial(Material* material) { m_material = material; }

	Mesh* GetMesh() const { return m_mesh; }
	Material* GetMaterial() const { return m_material; }

	void Render() override {
		if (!m_mesh || !m_material) {
			return;
		}

		auto transform = GetOwner()->GetComponent<TransformComponent>();
		if (!transform) {
			return;
		}

		m_material->Bind();
		m_material->SetUniform("modelMatrix", transform->GetTransformMatrix());
		m_mesh->Render();
	}

	BoundingBox GetBoundingBox() const;

private:
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
};

class CameraComponent : public Component {
public:
	void SetPerspective(float fov, float aspectRatio, float near, float far) {
		m_fov = fov;
		m_aspectRatio = aspectRatio;
		m_near = near;
		m_far = far;
	}

	glm::mat4 GetViewMatrix() const {
		auto transform = GetOwner()->GetComponent<TransformComponent>();
		if (transform) {
			glm::vec3 position = transform->GetPosition();
			glm::quat rotation = transform->GetRotation();

			glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f); // TODO: Make sure it isn't parallel to forward

			return glm::lookAt(position, position + forward, up);
		}
		return glm::mat4(1.0f);
	}

	glm::mat4 GetProjectionMatrix() const {
		if (m_projectionDirty) {
			m_projectionMatrix = glm::perspective(
				glm::radians(m_fov),
				m_aspectRatio,
				m_near,
				m_far
			);
			m_projectionDirty = false;
		}
		return m_projectionMatrix;
	}

private:
	float m_fov = 45.0f;
	float m_aspectRatio = 16.0f / 9.0f;
	float m_near = 0.1f;
	float m_far = 1000.0f;

	glm::mat4 m_viewMatrix = glm::mat4(1.0f);
	mutable glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
	mutable bool m_projectionDirty = true;
};

}