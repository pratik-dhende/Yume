#pragma once

#include <DirectXMath.h>
#include <memory>

class RenderItem {
public:
	DirectX::XMFLOAT4X4 m_world = Yume::identityMatrix4x4();

	int m_constantBufferOffset = 0;
	int m_objectConstantsUpdates = 0;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::unique_ptr<Yume::Mesh> m_mesh = nullptr;
};
