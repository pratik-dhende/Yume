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

		return mesh;
	}
}