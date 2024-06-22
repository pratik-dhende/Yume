#include "ympch.h"
#include "Sandbox.h"

void Sandbox::init()
{
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList.Reset());

	buildCbvHeap();
	buildConstantBuffer();

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

}