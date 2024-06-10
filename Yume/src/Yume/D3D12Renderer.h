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

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	};
}
