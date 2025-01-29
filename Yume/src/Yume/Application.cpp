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
#ifdef YM_DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		try
		{	
			m_window = std::make_unique<Window>(YM_ENGINE_NAME.c_str(), 1280, 720);
			m_window->show(nCmdShow);

			m_renderer = std::make_unique<D3D12Renderer>(m_window->m_d3d12Port);

			init();

			MSG msg = {};
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					update();
				}
			}
		}
		catch (const Exception& exception)
		{
			MessageBox(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
		}

		EventDispatcher::shutdown();
	}
}
