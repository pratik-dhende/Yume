#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

// TODO: Add to premake
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Yume
{
	class D3D12Renderer
	{
	public:
		D3D12Renderer();

	private:
		void init();
		void logAdapters();
		void logAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter> adapter);

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;
		UINT m_cbvSrvUavDescriptorSize = 0;
		UINT m_4xMsaaQuality = 0;

		DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		
	};
}
