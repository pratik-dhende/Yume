#include "ympch.h"

#include "Core.h"
#include "Application.h"
#include "Window.h"
#include "D3D12Renderer.h"
#include "Event/MouseEvent.h"
#include "Event/ApplicationEvent.h"
#include "StepTimer.h"

#pragma comment(lib, "DirectXTK.lib")

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

			m_timer = std::make_unique<DX::StepTimer>();

			m_renderer = std::make_unique<D3D12Renderer>(m_window->m_d3d12Port);

			m_mouse = std::make_unique<DirectX::Mouse>();
			m_mouse->SetWindow(m_window->getHandle());

			DirectX::Mouse::ButtonStateTracker tracker;

			init();

			MSG msg = {};
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					auto state = m_mouse->GetState();
					tracker.Update(state);

					if (tracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED) {
						m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
					}
					else if (tracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
						m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
					}

					if (state.leftButton) {
						if (state.positionMode == DirectX::Mouse::MODE_RELATIVE) {
							MouseMovedEvent mouseMovedEvent(static_cast<float>(state.x), static_cast<float>(state.y), true);
							EventDispatcher::dispatchEvent(mouseMovedEvent);
						}
						else {
							MouseMovedEvent mouseMovedEvent(static_cast<float>(state.x), static_cast<float>(state.y), false);
							EventDispatcher::dispatchEvent(mouseMovedEvent);
						}
					}
					
					const double millisecondsPerFrame = 1000.0 / m_timer->GetFramesPerSecond();
					const std::wstring windowTitle = m_window->getTitle() + L"    FPS: " + std::to_wstring(m_timer->GetFramesPerSecond()) + L"    MSPF: " + std::to_wstring(millisecondsPerFrame);

					SetWindowText(m_window->getHandle(), windowTitle.c_str());

					m_timer->Tick([&]() {
						update(m_timer->GetElapsedSeconds());
					});
					
					draw();
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
