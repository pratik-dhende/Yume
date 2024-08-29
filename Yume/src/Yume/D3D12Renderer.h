#pragma once

#include "directx/d3dx12.h"
#include <dxgi1_6.h>

// TODO: Add to premake
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Yume
{	
	class YM_API ID3D12Window
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
		Microsoft::WRL::ComPtr<ID3DBlob> compileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target);

	public:
		Microsoft::WRL::ComPtr<ID3D12Device4> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		bool m_4xMsaaEnabled = false;
		UINT m_4xMsaaQualityLevels = 0;

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

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;

		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;
		UINT m_cbvSrvUavDescriptorSize = 0;


	};
}
