#pragma once

#include "Core.h"

namespace Yume {
	class Window;
	class D3D12Renderer;
	class Mouse;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void run(int nCmdShow);

		virtual void init() = 0;
		virtual void update() = 0;
		virtual void draw() = 0;

	public:
		std::unique_ptr<D3D12Renderer> m_renderer;
		std::unique_ptr<Window> m_window;
		std::unique_ptr<DirectX::Mouse> m_mouse;
	};

	std::unique_ptr<Application> createApplication();
}

