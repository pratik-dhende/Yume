#pragma once
#include "Yume.h"
#include <DirectXMath.h>

class Box : public Yume::Application
{
public:
	void init() override;
	void update() override;
	void draw() override;

private:
	void buildCbvHeap();
	void buildConstantBuffer();
	void buildRootSignature();
	void buildShadersAndInputLayout();
	void buildBoxGeometry();
	void buildPipelineStateObject();

	void handleMouseMove(const Yume::Event& event);
	void handleWindowResize(const Yume::Event& event);

private:
	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 m_worldViewProjMatrix = Yume::identityMatrix4x4();
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	std::unique_ptr<Yume::UploadBuffer<ObjectConstants>> m_objectConstants = nullptr;
	std::unique_ptr<Yume::Mesh> m_boxMesh = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateObject;

	DirectX::XMFLOAT4X4 m_view = Yume::identityMatrix4x4();
	DirectX::XMFLOAT4X4 m_projection = Yume::identityMatrix4x4();
	DirectX::XMFLOAT4X4 m_world = Yume::identityMatrix4x4();

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
};