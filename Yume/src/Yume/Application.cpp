#include "ympch.h"
#include "Core.h"
#include "Application.h"
#include "Window.h"

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
		Window window(YM_ENGINE_NAME.c_str());
		window.show(nCmdShow);

		MSG msg{};
		while (GetMessageW(&msg, window.getHandle(), NULL, NULL) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}
