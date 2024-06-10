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

#if defined(DEBUG) || defined(_DEBUG) 
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}
#endif	
		YM_THROW_IF_FAILED_DX_EXCEPTION(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

		// Use the primary adapter (TODO: Search for NVIDIA driver and use it)
		HRESULT primaryAdapterResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));

		if (FAILED(primaryAdapterResult))
		{	
			// Fallback to WARP if no adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			YM_THROW_IF_FAILED_DX_EXCEPTION(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			YM_THROW_IF_FAILED_DX_EXCEPTION(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
		}

		YM_THROW_IF_FAILED_DX_EXCEPTION(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	}
}