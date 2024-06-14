#include "ympch.h"

#include "Core.h"
#include "Window.h"

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

	Window::Window(const std::wstring& title) : m_d3d12Port(*this)
	{
		createWindow(title);
	}

	Window::Window(const std::wstring& title, const int width, const int height)
		: m_width(width), m_height(height), m_d3d12Port(*this)
	{	
		// TODO: Add minimum window size constraint
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

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(hDllInstance);

		WNDCLASSW windowClass{};
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = handleMessage;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = hDllInstance;
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		windowClass.hCursor = LoadCursor(0, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = className.c_str();

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(RegisterClassW(&windowClass));

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

		YM_THROW_IF_FAILED_WIN32_EXCEPTION(m_hwnd);
	}
}

