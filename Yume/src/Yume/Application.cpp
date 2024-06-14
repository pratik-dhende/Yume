#include "ympch.h"

#include "Core.h"
#include "Application.h"
#include "Window.h"
#include "D3D12Renderer.h"

namespace Yume 
{

	Application::Application() 
	{

	}

	Application::~Application() 
	{

	}

	void Application::run(int nCmdShow) 
	{
		try
		{
			Window window(YM_ENGINE_NAME.c_str(), 1280, 720);
			window.show(nCmdShow);

			D3D12Renderer renderer(window.m_d3d12Port);

			MSG msg{};
			while (GetMessageW(&msg, window.getHandle(), NULL, NULL) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		catch (const Exception& exception)
		{
			MessageBoxW(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
		}
	}
}
