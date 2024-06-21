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
			m_window = std::make_unique<Window>(YM_ENGINE_NAME.c_str(), 1280, 720);
			m_window->show(nCmdShow);

			m_renderer = std::make_unique<D3D12Renderer>(m_window->m_d3d12Port);

			init();

			MSG msg{};
			while (GetMessageW(&msg, m_window->getHandle(), NULL, NULL) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				update();
			}
		}
		catch (const Exception& exception)
		{
			MessageBoxW(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
		}
	}
}
