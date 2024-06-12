#pragma once

extern std::unique_ptr<Yume::Application> Yume::createApplication();

int YM_MAIN()
{	
	Yume::Log::init();

	YM_CORE_INFO("Yume Logger initialized.");
	YM_CORE_INFO("Yume Engine initialized.");

	auto app = Yume::createApplication();
	app->run(YM_N_CMD_SHOW);

	return 0;
}
