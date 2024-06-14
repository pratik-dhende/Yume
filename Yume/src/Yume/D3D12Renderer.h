#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

// TODO: Add to premake
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Yume
{	
	class ID3D12Window
	{
	public:
		virtual int getWidth() const noexcept = 0;
		virtual int getHeight() const noexcept = 0;
	};

	class D3D12Renderer
	{
	public:
		D3D12Renderer(const ID3D12Window& window);

	private:
		void init(const ID3D12Window& window);
		void createCommandObjects();
		void createSwapChain(const ID3D12Window& window);
		void createRtvAndDsvDescriptionHeaps();
		void logAdapters();
		void logAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter> adapter);
		void logOutputDisplayModes(const Microsoft::WRL::ComPtr<IDXGIOutput> output, const DXGI_FORMAT format);

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;
		UINT m_cbvSrvUavDescriptorSize = 0;
		UINT m_4xMsaaQuality = 0;

		DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		
	};
}
