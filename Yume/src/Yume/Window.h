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

			int GetWidth() const noexcept override;
			int GetHeight() const noexcept override;
			HWND GetHandle() const noexcept override;

		private:
			const Window& m_window;
		};

	public:
		static LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		Window(const std::wstring& title, const int width, const int height);

		HWND GetHandle() const noexcept;
		int GetWidth() const noexcept;
		int GetHeight() const noexcept;
		const std::wstring& GetTitle() const noexcept;

		void show(const int nCmdShow) const;

	public:
		D3D12Port m_d3d12Port;

	private:
		HWND m_hwnd;

		int m_width = 1280;
		int m_height = 720;

		std::wstring m_title;
		
	private:
		void Create();
		void OnEvent(const Event& event);
	};
}