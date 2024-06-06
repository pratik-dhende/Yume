#pragma once

#include "Core.h"

#include <string>

namespace Yume
{
	class YM_API Window
	{
	public:
		static::LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		Window(const std::wstring& title);

		HWND getHandle() const noexcept;

		void show(const int nCmdShow) const;

	private:
		HWND m_hwnd;

		int m_width = 1280;
		int m_height = 720;
		
	private:
		void createWindow(const std::wstring& title);
	};
}