#pragma once

#include "Yume.h"
#include <DirectXMath.h>

class FrameResource;
class RenderItem;

class Shapes : public Yume::Application
{
public:
	void init() override;
	void update(const Yume::StepTimer& coreTimer) override;
	void draw() override;

private:
	void buildCbvHeap();
	void buildConstantBuffers();
	void buildRootSignature();
	void buildShadersAndInputLayout();
	void buildPipelineStateObject();
	void buildRenderItems();
	void buildBufferFrameResources();

	void updateCamera();
	void updateObjectConstants();
	void updatePassConstants(const Yume::StepTimer& timer);

	void drawRenderItems();

	void handleMouseMove(const Yume::Event& event);
	void handleWindowResize(const Yume::Event& event);

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateObject;

	DirectX::XMFLOAT4X4 m_view = Yume::identityMatrix4x4();
	DirectX::XMFLOAT4X4 m_projection = Yume::identityMatrix4x4();
	DirectX::XMFLOAT4X4 m_world = Yume::identityMatrix4x4();

	DirectX::XMFLOAT3 m_cameraPositionWorld = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);

	float m_cameraRadius = 5.0f;
	float m_cameraTheta = DirectX::XM_PIDIV4;
	float m_cameraPhi = DirectX::XM_PIDIV4;
	float m_nearZ = 1.0f;
	float m_farZ = 1000.0f;

	float m_mouseXDelta = 0.0f;
	float m_mouseYDelta = 0.0f;

	std::vector<std::unique_ptr<FrameResource>> m_bufferFrameResources;
	std::vector<std::unique_ptr<RenderItem>> m_renderItems;

	int m_currentFrameResourceIndex = 0;
};