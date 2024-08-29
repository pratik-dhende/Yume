#include "ympch.h"
#include "Sandbox.h"

void Sandbox::init()
{	
	// Puts command list in the recording state
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList->Reset(m_renderer->m_commandAllocator.Get(), nullptr));

	buildCbvHeap();
	buildConstantBuffer();
	buildRootSignature();
	buildShadersAndInputLayout();
	buildBoxGeometry();
	buildPipelineStateObject();
}

void Sandbox::update()
{
	
}

std::unique_ptr<Yume::Application> Yume::createApplication() {
	return std::make_unique<Sandbox>();
}

void Sandbox::buildCbvHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.ReleaseAndGetAddressOf())));
}

void Sandbox::buildConstantBuffer()
{
	m_objectConstants = std::make_unique<Yume::UploadBuffer<ObjectConstants>>(m_renderer->m_device.Get(), 1, true);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_objectConstants->getResource()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = Yume::nextMultiple256(sizeof(ObjectConstants));

	m_renderer->m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Sandbox::buildRootSignature()
{	
	CD3DX12_DESCRIPTOR_RANGE descTableRanges[1];

	// TODO: Fix the "Using uninitialized memory"
	descTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, descTableRanges);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRoot;
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootError;

	HRESULT hrSerializedRoot = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, serializedRoot.GetAddressOf(), serializedRootError.GetAddressOf());

	if (serializedRootError)
	{
		YM_ERROR(static_cast<char*>(serializedRootError->GetBufferPointer()));
	}

	YM_THROW_IF_FAILED_DX_EXCEPTION(hrSerializedRoot);

	m_renderer->m_device->CreateRootSignature(0, serializedRoot->GetBufferPointer(), serializedRoot->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf()));
}

void Sandbox::buildShadersAndInputLayout()
{	
	m_vsByteCode = m_renderer->compileShader(L"Shaders/VS.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = m_renderer->compileShader(L"Shaders/PS.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void Sandbox::buildBoxGeometry()
{
	std::array<Vertex, 8> vertices = {
		Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::White) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Black) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Green) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	m_boxMesh = std::make_unique<Yume::Mesh>("BoxMesh", m_renderer->m_device.Get(), m_renderer->m_commandList.Get(), vertices.data(), sizeof(vertices[0]), vertices.size(), indices.data(), sizeof(indices[0]), indices.size());

	m_boxMesh->subMeshes["box"] = Yume::SubMesh { static_cast<UINT>(indices.size()), 0, 0 };
}

void Sandbox::buildPipelineStateObject()
{	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.pRootSignature = m_rootSignature.Get();
	pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsByteCode.Get());
	pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(m_psByteCode.Get());
	pipelineStateDesc.NodeMask = 0;
	pipelineStateDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
	pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
	pipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // TODO: Is this the right way?
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = m_renderer->m_backBufferFormat;
	pipelineStateDesc.DSVFormat = m_renderer->m_depthStencilFormat;
	pipelineStateDesc.SampleDesc.Count = m_renderer->m_4xMsaaEnabled ? 4 : 1;
	pipelineStateDesc.SampleDesc.Quality = m_renderer->m_4xMsaaEnabled ? m_renderer->m_4xMsaaQualityLevels - 1 : 0;
	pipelineStateDesc.CachedPSO.pCachedBlob = nullptr; // TODO: Is this the right way?
	pipelineStateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // TODO: Is this the right way?

	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_pipelineStateObject.ReleaseAndGetAddressOf())));
}