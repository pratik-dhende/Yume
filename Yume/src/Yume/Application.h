#pragma once

#include "Core.h"
#include "TextureManager.h"
#include "ShaderManager.h"

namespace DirectX {
	class Mouse;
}

namespace Yume {
	class Window;
	class D3D12Renderer;
	class Event;
	class StepTimer;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void run(int nCmdShow);

		virtual void Init() = 0;
		virtual void Update(const StepTimer& coreTimer) = 0;
		virtual void Render() = 0;

	public:
		std::unique_ptr<D3D12Renderer> m_renderer;
		std::unique_ptr<Window> m_window;
		std::unique_ptr<DirectX::Mouse> m_mouse;
		std::unique_ptr<StepTimer> m_timer;

		std::unique_ptr<TextureManager> m_textureManager;
		std::unique_ptr<ShaderManager> m_shaderManager;
	};

	std::unique_ptr<Application> createApplication();
}

