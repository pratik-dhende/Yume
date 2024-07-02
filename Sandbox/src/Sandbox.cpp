#include "ympch.h"
#include "Sandbox.h"

void Sandbox::init()
{
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList.Reset());

	buildCbvHeap();
	buildConstantBuffer();
	buildRootSignature();
	buildShadersAndInputLayout();
	buildBoxGeometry();
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

}