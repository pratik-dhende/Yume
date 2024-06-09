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
		Window(const std::wstring& title, const unsigned int width, const unsigned int height);

		HWND getHandle() const noexcept;

		void show(const int nCmdShow) const;

	private:
		HWND m_hwnd;

		unsigned int m_width = 1280;
		unsigned int m_height = 720;
		
	private:
		void createWindow(const std::wstring& title);
	};
}