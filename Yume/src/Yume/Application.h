#pragma once

#include "Core.h"

namespace Yume {

	class YM_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();
	};

	Application* createApplication();

}

