#include "ympch.h"

#include "D3D12Renderer.h"

namespace Yume
{	
	D3D12Renderer::D3D12Renderer()
	{
		init();
	}

	void D3D12Renderer::init()
	{
		// TODO: Research on different versions of D3D12 functions

#ifdef YM_DEBUG 
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}
#endif	
		YM_THROW_IF_FAILED_DX_EXCEPTION(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));

		// Use the primary adapter (TODO: Search for NVIDIA driver and use it)
		const HRESULT primaryAdapterResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));

		if (FAILED(primaryAdapterResult))
		{	
			// Fallback to WARP if no adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			YM_THROW_IF_FAILED_DX_EXCEPTION(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
		}

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

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
	}

	void D3D12Renderer::logAdapters()
	{	
		UINT adapterIndex = 0;
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;

		while (m_factory->EnumAdapters(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			adapter->GetDesc(&adapterDesc);

			YM_CORE_INFO("Adapter: {0}", wStringToAnsi(adapterDesc.Description));
			logAdapterOutputs(adapter);

			++adapterIndex;
		}
	}

	void D3D12Renderer::logAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter> adapter)
	{
		UINT adapterOutputIndex = 0;
		Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput = nullptr;

		while (adapter->EnumOutputs(adapterOutputIndex, &adapterOutput) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC adapterOutputDesc;
			adapterOutput->GetDesc(&adapterOutputDesc);

			YM_CORE_INFO("Output: {0}", wStringToAnsi(adapterOutputDesc.DeviceName));

			++adapterOutputIndex;
		}
	}
}