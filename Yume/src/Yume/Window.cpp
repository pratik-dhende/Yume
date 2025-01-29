#include "ympch.h"

#include "Core.h"
#include "Window.h"
#include "Event/ApplicationEvent.h"

namespace Yume
{	
	int Window::D3D12Port::getWidth() const noexcept
	{
		return m_window.m_width;
	}

	int Window::D3D12Port::getHeight() const noexcept
	{
		return m_window.m_height;
	}

	HWND Window::D3D12Port::getHandle() const noexcept
	{
		return m_window.m_hwnd;
	}

	Window::Window(const std::wstring& title, const int width, const int height)
		: m_width(width), m_height(height), m_d3d12Port(*this)
	{	
		// TODO: Add minimum window size constraint
		createWindow(title);
		EventDispatcher::registerEventHandler([&](const Event& event) {
			this->onEvent(event);
		});
	}

	LRESULT CALLBACK Window::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{	
		switch (uMsg)
		{	
			case WM_SIZE:
			{	
				if (wParam == SIZE_MAXIMIZED) {
					try {
						WindowResizeEvent windowResizeEvent(LOWORD(lParam), HIWORD(lParam));
						EventDispatcher::dispatchEvent(windowResizeEvent);
					}
					catch (const Exception& exception)
					{
						MessageBox(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
					}
				}
				else if (wParam == SIZE_RESTORED) {
					try {
						WindowResizeEvent windowResizeEvent(LOWORD(lParam), HIWORD(lParam));
						EventDispatcher::dispatchEvent(windowResizeEvent);
					}
					catch (const Exception& exception)
					{
						MessageBox(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
					}
				}
				return 0;
			}

			case WM_EXITSIZEMOVE:
			{	
				try {
					RECT windowRect;
					GetClientRect(hwnd, &windowRect);
					WindowResizeEvent windowResizeEvent(static_cast<int>(windowRect.right - windowRect.left), static_cast<int>(windowRect.bottom - windowRect.top));
					EventDispatcher::dispatchEvent(windowResizeEvent);
				}
				catch (const Exception& exception)
				{
					MessageBox(NULL, exception.toWString().c_str(), L"Exception", MB_OK);
				}
				return 0;
			}

			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	HWND Window::getHandle() const noexcept
	{
		return m_hwnd;
	}

	int Window::getWidth() const noexcept
	{
		return m_width;
	}

	int Window::getHeight() const noexcept
	{
		return m_height;
	}

	void Window::show(const int nCmdShow) const
	{
		ShowWindow(m_hwnd, nCmdShow);
	}

	void Window::createWindow(const std::wstring& title)
	{	
		const std::wstring className = L"Window";
		const HINSTANCE hInstance = GetModuleHandle(nullptr);

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(hInstance);

		WNDCLASS windowClass{};
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = handleMessage;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = hInstance;
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		windowClass.hCursor = LoadCursor(0, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = className.c_str();

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(RegisterClass(&windowClass));

		RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		m_hwnd = CreateWindow(
			className.c_str(),
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hInstance,
			nullptr
		);

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(m_hwnd);
	}

	void Window::onEvent(const Event& event) {
		if (event.getEventType() == EventType::WindowResize) {
			const WindowResizeEvent& windowResizeEvent = static_cast<const WindowResizeEvent&>(event);
			m_width = windowResizeEvent.getWidth();
			m_height = windowResizeEvent.getHeight();
		}
	}
}

