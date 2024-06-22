#include "ympch.h"

#include "D3D12Renderer.h"

namespace Yume
{	
	D3D12Renderer::D3D12Renderer(const ID3D12Window& window)
	{
		init(window);
	}

	void D3D12Renderer::init(const ID3D12Window& window)
	{
		// TODO: Research on different versions of D3D12 functions

#ifdef YM_DEBUG 
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
			debugController->EnableDebugLayer();
		}
#endif	
		YM_THROW_IF_FAILED_DX_EXCEPTION(CreateDXGIFactory1(IID_PPV_ARGS(m_factory.ReleaseAndGetAddressOf())));

		// Use the primary adapter (TODO: Search for NVIDIA driver and use it)
		const HRESULT primaryAdapterResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()));

		if (FAILED(primaryAdapterResult))
		{	
			// Fallback to WARP if no adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			YM_THROW_IF_FAILED_DX_EXCEPTION(m_factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.ReleaseAndGetAddressOf())));
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())));
		}

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render target formats, so we only need to check quality support.
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = m_backBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

		// Set to use maximum 4X MSAA quality.
		m_4xMsaaQuality = msQualityLevels.NumQualityLevels;

		// All Direct3D 11 capable devices support 4X MSAA for all render target formats
		YM_CORE_ASSERT(m_4xMsaaQuality > 0, "Unexpected MSAA Quality Level");

#ifdef YM_DEBUG
		D3D12Renderer::logAdapters();
#endif	
		createCommandObjects();
		createSwapChain(window);
		createRtvAndDsvDescriptorHeaps();
	}

	void D3D12Renderer::createCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));

		// Start off in a closed state. This is because the first time we refer to the command list we will Reset it, and it needs to be closed before calling Reset.
		// TODO: Refer to documentation for creating command list without closing it.
		m_commandList->Close();
	}

	void D3D12Renderer::createSwapChain(const ID3D12Window& window)
	{
		// Release the previous swapchain we will be recreating
		// TODO: Remove it as we will be releasing the com ptr before passing to CreateSwapChain method.
		m_swapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = window.getWidth();
		swapChainDesc.BufferDesc.Height = window.getHeight();
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // TODO: Make it modifiable.
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = m_backBufferFormat;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = m_4xMsaaEnabled ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = m_4xMsaaEnabled ? m_4xMsaaQuality - 1 : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = s_swapChainBufferCount;
		swapChainDesc.OutputWindow = window.getHandle();
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_factory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
	}

	void D3D12Renderer::createRtvAndDsvDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = s_swapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())));
	}

	void D3D12Renderer::logAdapters() const
	{	
		UINT adapterIndex = 0;
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;

		while (m_factory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			adapter->GetDesc(&adapterDesc);

			YM_CORE_INFO("Adapter: {0}", wStringToAnsi(adapterDesc.Description));
			logAdapterOutputs(adapter);

			++adapterIndex;
		}
	}

	void D3D12Renderer::logAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter>& adapter) const
	{
		UINT adapterOutputIndex = 0;
		Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput = nullptr;

		while (adapter->EnumOutputs(adapterOutputIndex, adapterOutput.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC adapterOutputDesc;
			adapterOutput->GetDesc(&adapterOutputDesc);

			YM_CORE_INFO("Output: {0}", wStringToAnsi(adapterOutputDesc.DeviceName));

			logOutputDisplayModes(adapterOutput, m_backBufferFormat);

			++adapterOutputIndex;
		}
	}

	void D3D12Renderer::logOutputDisplayModes(const Microsoft::WRL::ComPtr<IDXGIOutput>& output, const DXGI_FORMAT& format) const
	{
		UINT displayModesCount = 0;
		UINT flags = 0;

		// Call with nullptr to get list count
		output->GetDisplayModeList(format, flags, &displayModesCount, nullptr);

		std::vector<DXGI_MODE_DESC> displayModes(displayModesCount);
		output->GetDisplayModeList(format, flags, &displayModesCount, displayModes.data());

		for (auto& displayMode : displayModes)
		{
			UINT refreshRateNumerator = displayMode.RefreshRate.Numerator;
			UINT refreshRateDenominator = displayMode.RefreshRate.Denominator;

			YM_CORE_INFO("DisplayMode: Width - {0}, Height - {1}, Refresh Rate - {2}/{3}", displayMode.Width, displayMode.Height, displayMode.RefreshRate.Numerator, displayMode.RefreshRate.Denominator);
		}
	}
}