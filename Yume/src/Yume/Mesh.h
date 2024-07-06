#pragma once

#include <wrl/client.h>

namespace Yume
{	
	struct YM_API SubMesh
	{
		UINT indexCount = 0;
		int startIndex = 0;
		int baseVertexLocation = 0;
	};

	class YM_API Mesh
	{
	public:
		Mesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const void* vertexData, UINT64 vertexByteSize, UINT vertices, const void* indexData, UINT64 indexByteSize, UINT indices);

	public:
		std::unordered_map<std::string, SubMesh> subMeshes;

	private:
		void createDefaultAndUploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const void* data, UINT64 dataByteSize) const;

	private:
		std::string m_name = "";

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;

		// Need this to keep the upload buffer alive until the command list is executed.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUploadBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUploadBuffer = nullptr;

		UINT m_vertexByteStride = 0;
		
		DXGI_FORMAT m_indexFormat = DXGI_FORMAT_R16_UINT;
	};
}

