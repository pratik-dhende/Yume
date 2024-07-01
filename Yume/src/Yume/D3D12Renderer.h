#pragma once

#include "directx/d3dx12.h"
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
		virtual HWND getHandle() const noexcept = 0;
	};

	class YM_API D3D12Renderer
	{	
	private:
		static const int s_swapChainBufferCount = 2;

	public:
		D3D12Renderer(const ID3D12Window& window);

	public:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;

	private:
		void init(const ID3D12Window& window);
		void createCommandObjects();
		void createSwapChain(const ID3D12Window& window);
		void createRtvAndDsvDescriptorHeaps();
		void logAdapters() const;
		void logAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter>& adapter) const;
		void logOutputDisplayModes(const Microsoft::WRL::ComPtr<IDXGIOutput>& output, const DXGI_FORMAT& format) const;

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;

		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;
		UINT m_cbvSrvUavDescriptorSize = 0;
		UINT m_4xMsaaQuality = 0;

		DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		bool m_4xMsaaEnabled = false;
	};
}
