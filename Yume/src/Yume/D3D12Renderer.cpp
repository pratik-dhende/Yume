#include "ympch.h"

#include "D3D12Renderer.h"
#include "Log.h"
#include "Event/ApplicationEvent.h"

namespace Yume
{	
	D3D12Renderer::D3D12Renderer(const ID3D12Window& window)
	{	
		EventDispatcher::registerEventHandler([&](const Event& event) {
			this->onEvent(event);
		});
		init(window);
	}

	void D3D12Renderer::init(const ID3D12Window& window)
	{
		// TODO: Research on different versions of D3D12 functions
		UINT dxgiFactoryFlags = 0;
#ifdef YM_DEBUG 
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif	
		YM_THROW_IF_FAILED_DX_EXCEPTION(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_factory.ReleaseAndGetAddressOf())));

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

		m_rtvDescriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvDescriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cbvSrvUavDescriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render target formats, so we only need to check quality support.
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = m_backBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

		// Set to use maximum 4X MSAA quality.
		m_4xMsaaQualityLevels = msQualityLevels.NumQualityLevels;

		// All Direct3D 11 capable devices support 4X MSAA for all render target formats
		YM_CORE_ASSERT(m_4xMsaaQualityLevels > 0, "Unexpected MSAA Quality Level");

#ifdef YM_DEBUG
		D3D12Renderer::logAdapters();
#endif	
		createCommandObjects();

		createSwapChain(window);
		m_currentBackBuffer = m_swapChain->GetCurrentBackBufferIndex();

		createRtvAndDsvDescriptorHeaps();

		resize(window.getWidth(), window.getHeight());
	}

	void D3D12Renderer::createCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));

		// Start off in a closed state. This is because the first time we refer to the command list we will Reset it, and it needs to be closed before calling Reset.
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
	}

	void D3D12Renderer::createSwapChain(const ID3D12Window& window)
	{
		// Release the previous swapchain we will be recreating
		// TODO: Remove it as we will be releasing the com ptr before passing to CreateSwapChain method.
		m_swapChain.Reset();

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = window.getWidth();
		swapChainDesc.Height = window.getHeight();
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = s_swapChainBufferCount;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), window.getHandle(), &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()));
		YM_THROW_IF_FAILED_DX_EXCEPTION(swapChain.As(&m_swapChain));
	}

	void D3D12Renderer::createRtvAndDsvDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
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

	void D3D12Renderer::flushCommandQueue()
	{	
		// Advance the fence value to mark commands up to this fence point.
		m_currentFence++;

		// Add an instruction to the command queue to set a new fence point.  Because we 
		// are on the GPU timeline, the new fence point won't be set until the GPU finishes
		// processing all the commands prior to this Signal().
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

		// Wait until the GPU has completed commands up to this fence point.
		if (m_fence->GetCompletedValue() < m_currentFence)
		{
			const HANDLE fenceEventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
			YM_THROW_IF_FAILED_WIN32_EXCEPTION(fenceEventHandle);

			YM_THROW_IF_FAILED_DX_EXCEPTION(m_fence->SetEventOnCompletion(m_currentFence, fenceEventHandle));

			WaitForSingleObject(fenceEventHandle, INFINITE);
			CloseHandle(fenceEventHandle);
		}
	}

	void D3D12Renderer::switchBackBuffer()
	{
		m_currentBackBuffer = (m_currentBackBuffer + 1) % s_swapChainBufferCount;
	}

	void D3D12Renderer::onEvent(const Event& event) {
		if (event.getEventType() == EventType::WindowResize) {
			const WindowResizeEvent& windowResizeEvent = static_cast<const WindowResizeEvent&>(event);
			resize(windowResizeEvent.getWidth(), windowResizeEvent.getHeight());
		}
	}

	void D3D12Renderer::resize(const int width, const int height) {
		YM_CORE_ASSERT(m_device, "No device for resizing");
		YM_CORE_ASSERT(m_commandAllocator, "No command allocator for resizing");
		YM_CORE_ASSERT(m_swapChain, "No swap chain for resizing");

		// Ensures that references held to swapchain buffers are released
		flushCommandQueue();

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		for (int i = 0; i < s_swapChainBufferCount; ++i) {
			m_swapChainBuffers[i].Reset();
		}
		m_depthStencilBuffer.Reset();

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_swapChain->ResizeBuffers(s_swapChainBufferCount, width, height, m_backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		m_currentBackBuffer = m_swapChain->GetCurrentBackBufferIndex();

		// Create Render Target View
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < s_swapChainBufferCount; i++)
		{
			YM_THROW_IF_FAILED_DX_EXCEPTION(m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainBuffers[i].ReleaseAndGetAddressOf())));
			m_device->CreateRenderTargetView(m_swapChainBuffers[i].Get(), nullptr, rtvDescriptorHandle);
			rtvDescriptorHandle.Offset(1, m_rtvDescriptorHandleIncrementSize);
		}

		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.Format = m_depthStencilFormat; // TODO: Change in future for new demo
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		auto depthStencilHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto depthStencilClearValue = CD3DX12_CLEAR_VALUE(m_depthStencilFormat, 1.0f, 0);

		// TODO: Research on compatiblity of D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES with CreateCommittedResource as it doesn't allow me to use it
		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateCommittedResource(&depthStencilHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthStencilClearValue, IID_PPV_ARGS(m_depthStencilBuffer.ReleaseAndGetAddressOf())));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvViewDesc;
		dsvViewDesc.Format = m_depthStencilFormat;
		dsvViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvViewDesc.Texture2D.MipSlice = 0;

		m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvViewDesc, m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_commandList->Close());
		ID3D12CommandList* commandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		flushCommandQueue();

		// Specify viewport and scissor rectangle
		m_screenViewport.TopLeftX = 0.0f;
		m_screenViewport.TopLeftY = 0.0f;
		m_screenViewport.Width = static_cast<FLOAT>(width);
		m_screenViewport.Height = static_cast<FLOAT>(height);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;

		m_scissorRect.top = 0;
		m_scissorRect.left = 0;
		m_scissorRect.right = width;
		m_scissorRect.bottom = height;
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

	Microsoft::WRL::ComPtr<ID3DBlob> D3D12Renderer::compileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target)
	{
		// TODO: Remove D3DCompile as for DirectX 12, Shader Model 5.1, the D3DCompile API, and FXC are all deprecated. Use Shader Model 6 via DXIL instead. See https://github.com/microsoft/DirectXShaderCompiler. (Kept it for simplicity for now)
		Microsoft::WRL::ComPtr<ID3DBlob> compiledByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> compileError = nullptr;

		UINT compileFlags = 0;
#ifdef YM_DEBUG
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT compileResult = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), target.c_str(), compileFlags, 0, compiledByteCode.GetAddressOf(), compileError.GetAddressOf());

		if (compileError)
		{
			YM_ERROR(static_cast<char*>(compileError->GetBufferPointer()));
		}
		YM_THROW_IF_FAILED_DX_EXCEPTION(compileResult);

		return compiledByteCode;
	}
}