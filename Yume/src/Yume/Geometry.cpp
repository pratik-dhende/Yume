#include "ympch.h"
#include "Geometry.h"

#include <array>

namespace Yume {
	Geometry::Mesh Geometry::getBox(const float width, const float height, const float depth, const uint32_t subDivisions) const {
		const float halfWidth = width * 0.5f;
		const float halfHeight = height * 0.5f;
		const float halfDepth = depth * 0.5f;

		const std::array<Vertex, 24> vertices = {
			// Front face
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 1.0)),

			// Back face
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 1.0)),

			// Left face
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0, 1.0)),

			// Right face
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0, 1.0)),

			// Bottom face
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, -halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 1.0)),

			// Top face
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 1.0)),
			Vertex(DirectX::XMFLOAT3(halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, -halfDepth), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 0.0)),
			Vertex(DirectX::XMFLOAT3(-halfWidth, halfHeight, halfDepth), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0, 1.0))
		};

		const std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			2, 3, 0,

			// back face
			4, 5, 6,
			6, 7, 4,

			// left face
			8, 9, 10,
			10, 11, 8,

			// right face
			12, 13, 14,
			14, 15, 12,

			// bottom face
			16, 17, 18,
			18, 19, 16,

			// top face
			20, 21, 22,
			22, 23, 20
		};

		Mesh mesh;
		mesh.m_vertices = std::vector<Vertex>(vertices.begin(), vertices.end());
		mesh.m_indices = std::vector<uint32_t>(indices.begin(), indices.end());

		for (int i = 0; i < subDivisions; ++i) {
			subdivide(mesh);
		}

		return mesh;
	}

	void Geometry::subdivide(Mesh& inOutMesh) const {
		const auto indices = inOutMesh.m_indices;
		const int numVertices = static_cast<int>(inOutMesh.m_vertices.size());

		inOutMesh.m_indices.resize(0);

		for (int i = 0; i < static_cast<int>(indices.size()); i += 3) {
			const Vertex v0 = inOutMesh.m_vertices[indices[i]];
			const Vertex v1 = inOutMesh.m_vertices[indices[i + 1]];
			const Vertex v2 = inOutMesh.m_vertices[indices[i + 2]];

			inOutMesh.m_vertices.push_back(bisect(v0, v1));
			inOutMesh.m_vertices.push_back(bisect(v1, v2));
			inOutMesh.m_vertices.push_back(bisect(v0, v2));

			inOutMesh.m_indices.push_back(indices[i]);
			inOutMesh.m_indices.push_back(numVertices + i);
			inOutMesh.m_indices.push_back(numVertices + i + 2);

			inOutMesh.m_indices.push_back(indices[i + 1]);
			inOutMesh.m_indices.push_back(numVertices + i + 1);
			inOutMesh.m_indices.push_back(numVertices + i);

			inOutMesh.m_indices.push_back(indices[i + 2]);
			inOutMesh.m_indices.push_back(numVertices + i + 2);
			inOutMesh.m_indices.push_back(numVertices + i + 1);

			inOutMesh.m_indices.push_back(numVertices + i + 2);
			inOutMesh.m_indices.push_back(numVertices + i);
			inOutMesh.m_indices.push_back(numVertices + i + 1);
		}
	}

	Geometry::Vertex Geometry::bisect(const Vertex& v0, const Vertex& v1) const {
		using namespace DirectX;

		auto position0 = XMLoadFloat3(&v0.m_position);
		auto position1 = XMLoadFloat3(&v1.m_position);

		auto normal0 = XMLoadFloat3(&v0.m_normal);
		auto normal1 = XMLoadFloat3(&v1.m_normal);

		auto tangent0 = XMLoadFloat3(&v0.m_tangent);
		auto tangent1 = XMLoadFloat3(&v1.m_tangent);

		auto uv0 = XMLoadFloat2(&v0.m_uv);
		auto uv1 = XMLoadFloat2(&v1.m_uv);

		Vertex midPoint;
		XMStoreFloat3(&midPoint.m_position, (position0 + position1) * 0.5f);
		XMStoreFloat3(&midPoint.m_normal, (normal0 + normal1) * 0.5f);
		XMStoreFloat3(&midPoint.m_tangent, (tangent0 + tangent1) * 0.5f);
		XMStoreFloat2(&midPoint.m_uv, (uv0 + uv1) * 0.5f);

		return midPoint;
	}
}