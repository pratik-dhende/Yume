#pragma once

#include "Core.h"
#include "D3D12Renderer.h"
#include "Event/Event.h"

#include <string>

namespace Yume
{	
	class Window
	{
	public:
		// Used to implement methods of same signatures with different implementations.
		class D3D12Port : public Yume::ID3D12Window
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
		static LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		Window(const std::wstring& title, const int width, const int height);

		HWND getHandle() const noexcept;
		int getWidth() const noexcept;
		int getHeight() const noexcept;
		const std::wstring& getTitle() const noexcept;

		void show(const int nCmdShow) const;

	public:
		D3D12Port m_d3d12Port;

	private:
		HWND m_hwnd;

		int m_width = 1280;
		int m_height = 720;

		std::wstring m_title;
		
	private:
		void createWindow();
		void onEvent(const Event& event);
	};
}