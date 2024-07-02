#pragma once

#include "Core.h"
#include "D3D12Renderer.h"

#include <string>

namespace Yume
{	
	class YM_API Window
	{
	public:
		// Used to implement methods of same signatures with different implementations.
		class YM_API D3D12Port : public Yume::ID3D12Window
		{	
		public:
			D3D12Port(const Window& window) 
				: m_window(window) 
			{ }

			int getWidth() const noexcept override;
			int getHeight() const noexcept override;
			HWND getHandle() const noexcept override;

		private:
			const Window& m_window;
		};

	public:
		static::LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		Window(const std::wstring& title);
		Window(const std::wstring& title, const int width, const int height);

		HWND getHandle() const noexcept;

		void show(const int nCmdShow) const;

	public:
		D3D12Port m_d3d12Port;

	private:
		HWND m_hwnd;

		int m_width = 1280;
		int m_height = 720;
		
	private:
		void createWindow(const std::wstring& title);
	};
}