#pragma once

#include "Yume/Utility/Utility.h"

#include <DirectXMath.h>
#include <memory>
#include <d3d12.h>
#include <wrl/client.h>

namespace Yume {
    template<typename T>
    class UploadBuffer;
}

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 m_world = Yume::identityMatrix4x4();
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 m_view = Yume::identityMatrix4x4();
    DirectX::XMFLOAT4X4 m_inverseView = Yume::identityMatrix4x4();

    DirectX::XMFLOAT4X4 m_projection = Yume::identityMatrix4x4();
    DirectX::XMFLOAT4X4 m_inverseProjection = Yume::identityMatrix4x4();

    DirectX::XMFLOAT4X4 m_viewProjection = Yume::identityMatrix4x4();
    DirectX::XMFLOAT4X4 m_inverseViewProjection = Yume::identityMatrix4x4();

    DirectX::XMFLOAT3 m_cameraPositionWorld = { 0.0f, 0.0f, 0.0f };
    float pad0 = 0.0f;

    DirectX::XMFLOAT2 m_renderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 m_inverseRenderTargetSize = { 0.0f, 0.0f };

    float m_nearZ = 0.0f;
    float m_farZ = 0.0f;
    float m_totalTime = 0.0f;
    float m_deltaTime = 0.0f;
};

class FrameResource {
public:
    FrameResource(ID3D12Device4* device, const int totalObjectConstants, const int totalPassConstants);

	std::unique_ptr<Yume::UploadBuffer<ObjectConstants>> m_objectConstants = nullptr;
	std::unique_ptr<Yume::UploadBuffer<PassConstants>> m_passConstants = nullptr;

    UINT64 m_fenceValue = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
};