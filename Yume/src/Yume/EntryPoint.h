#pragma once

extern std::unique_ptr<Yume::Application> Yume::createApplication();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{	
	Yume::Log::init();

	YM_CORE_INFO("Yume Logger initialized.");
	YM_CORE_INFO("Yume Engine initialized.");

	auto app = Yume::createApplication();
	app->run(nCmdShow);

	return 0;
}
