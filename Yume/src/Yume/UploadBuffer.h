#pragma once

#include "Yume/Utility/Utility.h"
#include "directx/d3dx12.h"

#include <dxgi1_6.h>

namespace Yume
{
	template<typename T>
	class UploadBuffer
	{
	public:
		UploadBuffer(const UploadBuffer& other) = delete;
		UploadBuffer& operator=(const UploadBuffer& other) = delete;

		UploadBuffer(ID3D12Device* device, const UINT elementCount, const bool isConstantBuffer)
			: m_isConstantBuffer(isConstantBuffer)
		{
			m_elementByteSize = sizeof(T);

			/*
				Constant buffer elements need to be multiples of 256 bytes.
				This is because the hardware can only view constant data
				at m*256 byte offsets and of n*256 byte lengths.
				typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
					UINT64 OffsetInBytes; // multiple of 256
					UINT   SizeInBytes;   // multiple of 256
				} D3D12_CONSTANT_BUFFER_VIEW_DESC;
			*/
			if (m_isConstantBuffer)
			{
				m_elementByteSize = Yume::nextMultiple256(m_elementByteSize);
			}

			UINT resourceByteSize = m_elementByteSize * elementCount;

			// TODO: Why passing address of temporary doesn't work?
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(resourceByteSize);
			YM_THROW_IF_FAILED_DX_EXCEPTION(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_uploadBuffer.ReleaseAndGetAddressOf())));

			YM_THROW_IF_FAILED_DX_EXCEPTION(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
		}

		~UploadBuffer()
		{
			if (m_uploadBuffer)
			{
				m_uploadBuffer->Unmap(0, nullptr);
			}
			m_mappedData = nullptr;
		}

		ID3D12Resource* getResource()
		{
			return m_uploadBuffer.Get();
		}

		void updateBuffer(const int offset, const T& data)
		{
			memcpy(&m_mappedData[offset * m_elementByteSize], &data, sizeof(T));
		}

	private:
		BYTE* m_mappedData = nullptr; // TODO: Is it possible to use smart pointer here?

		bool m_isConstantBuffer = false;
		UINT m_elementByteSize = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer = nullptr;
	};
}

