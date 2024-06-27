#pragma once
#include "Yume.h"
#include <DirectXMath.h>
#include <Yume/Utility/Utility.h>
#include <Yume/UploadBuffer.h>

class Sandbox : public Yume::Application
{
public:
	void init() override;
	void update() override;

private:
	void buildCbvHeap();
	void buildConstantBuffer();
	void buildRootSignature();

private:
	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 m_worldViewProjMatrix = Yume::identityMatrix4x4();
	};

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	std::unique_ptr<Yume::UploadBuffer<ObjectConstants>> m_objectConstants = nullptr;
};