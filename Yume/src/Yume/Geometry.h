#pragma once

#include <DirectXMath.h>
#include <vector>

namespace Yume {
	class Geometry {
	public:
		struct Vertex {
			Vertex() = default;

			Vertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& tangent, const DirectX::XMFLOAT2& uv)
				: m_position(position), m_normal(normal), m_tangent(tangent), m_uv(uv) { }

			DirectX::XMFLOAT3 m_position;
			DirectX::XMFLOAT3 m_normal;
			DirectX::XMFLOAT3 m_tangent;
			DirectX::XMFLOAT2 m_uv;
		};

		struct Mesh {
			std::vector<Vertex> m_vertices;
			std::vector<uint32_t> m_indices;

			const std::vector<uint16_t>& getIndices16() {
				if (m_indices16.empty()) {
					m_indices16.reserve(m_indices.size());
					for (int i = 0; i < m_indices.size(); ++i) {
						m_indices16.push_back(static_cast<uint16_t>(m_indices[i]));
					}
				}
				return m_indices16;
			}

		private:
			std::vector<uint16_t> m_indices16;
		};

		Mesh getBox(const float width, const float height, const float depth, const uint32_t subDivisions) const;

	private:
		void subdivide(Mesh& subdividedMesh) const;
		Vertex bisect(const Vertex& v0, const Vertex& v1) const;
	};
}