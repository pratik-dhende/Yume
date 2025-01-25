#include "ympch.h"
#include "Mesh.h"
#include "Yume/Utility/Utility.h"

namespace Yume
{	
	Mesh::Mesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const void* vertexData, UINT vertexByteSize, UINT vertices, const void* indexData, UINT indexByteSize, UINT indices)
		: m_name(name)
	{
		UINT64 vertexDataByteSize = vertices * vertexByteSize;
		YM_THROW_IF_FAILED_DX_EXCEPTION(D3DCreateBlob(vertexDataByteSize, m_vertexBufferCPU.ReleaseAndGetAddressOf()));
		CopyMemory(m_vertexBufferCPU->GetBufferPointer(), vertexData, vertexDataByteSize);

		UINT64 indexDataByteSize = indices * indexByteSize;
		YM_THROW_IF_FAILED_DX_EXCEPTION(D3DCreateBlob(indexDataByteSize, m_indexBufferCPU.ReleaseAndGetAddressOf()));
		CopyMemory(m_indexBufferCPU->GetBufferPointer(), indexData, indexDataByteSize);

		createDefaultAndUploadBuffer(device, commandList, m_vertexBufferGPU, m_vertexUploadBuffer, m_vertexBufferCPU->GetBufferPointer(), m_vertexBufferCPU->GetBufferSize());

		createDefaultAndUploadBuffer(device, commandList, m_indexBufferGPU, m_indexUploadBuffer, m_indexBufferCPU->GetBufferPointer(), m_indexBufferCPU->GetBufferSize());

		m_vertexByteStride = vertexByteSize;
		
	}

	void Mesh::createDefaultAndUploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const void* data, UINT64 dataByteSize) const
	{	
		// Default buffer
		CD3DX12_HEAP_PROPERTIES bufferHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(dataByteSize);

		YM_THROW_IF_FAILED_DX_EXCEPTION(device->CreateCommittedResource(&bufferHeapProps, D3D12_HEAP_FLAG_NONE, &bufferResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

		// Upload buffer
		bufferHeapProps = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);

		YM_THROW_IF_FAILED_DX_EXCEPTION(device->CreateCommittedResource(&bufferHeapProps, D3D12_HEAP_FLAG_NONE, &bufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subResourcesData[1];
		subResourcesData[0].pData = data;
		subResourcesData[0].RowPitch = dataByteSize;
		subResourcesData[0].SlicePitch = subResourcesData[0].RowPitch;

		// Copy from upload to default buffer
		CD3DX12_RESOURCE_BARRIER resourceBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->ResourceBarrier(1, &resourceBarrierTransition);

		UpdateSubresources<1>(commandList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, subResourcesData);

		resourceBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

		commandList->ResourceBarrier(1, &resourceBarrierTransition);
	}
}
