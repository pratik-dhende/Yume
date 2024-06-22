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
		UploadBuffer(const ID3D12Device& device, const UINT elementCount, const bool isConstantBuffer)
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
				m_elementByteSize = nextMultiple256(m_elementByteSize);
			}

			YM_THROW_IF_FAILED_DX_EXCEPTION(device.CreateCommittedResource(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_uploadBuffer->ReleaseAndGetAddressOf())));
		}

	private:
		bool m_isConstantBuffer = false;
		UINT m_elementByteSize = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	};
}

