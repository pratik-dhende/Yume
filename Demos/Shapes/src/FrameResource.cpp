#include "ympch.h"
#include "FrameResource.h"
#include "Yume/UploadBuffer.h"

FrameResource::FrameResource(ID3D12Device4* device, const int totalObjectConstants, const int totalPassConstants) {
	m_objectConstants = std::make_unique<Yume::UploadBuffer<ObjectConstants>>(device, totalObjectConstants, true);
	m_passConstants = std::make_unique<Yume::UploadBuffer<PassConstants>>(device, totalPassConstants, true);

	YM_THROW_IF_FAILED_DX_EXCEPTION(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));
}