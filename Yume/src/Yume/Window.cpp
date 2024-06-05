#include "ympch.h"

#include "Core.h"
#include "Window.h"

namespace Yume
{	
	Window::Window(const std::wstring& title)
	{
		createWindow(title);
	}

	LRESULT CALLBACK Window::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	HWND Window::getHandle() const noexcept
	{
		return m_hwnd;
	}

	void Window::show(const int nCmdShow) const
	{
		ShowWindow(m_hwnd, nCmdShow);
	}

	void Window::createWindow(const std::wstring& title)
	{	
		const std::wstring className = L"Window";
		const HINSTANCE hDllInstance = GetModuleHandleW(YM_DLL_FILE_NAME.c_str());

		WNDCLASSW wc{};
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = handleMessage;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hDllInstance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = className.c_str();

		RegisterClassW(&wc);
		if (RegisterClassW(&wc))
		{
			int b = 2 + 3;
		}

		m_hwnd = CreateWindowW(
			className.c_str(),
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			m_width,
			m_height,
			NULL,
			NULL,
			hDllInstance,
			nullptr
		);
	}
}

