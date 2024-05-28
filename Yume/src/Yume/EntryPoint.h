#pragma once

extern Yume::Application* Yume::createApplication();

int main(int argc, char** argv) 
{	
	Yume::Log::init();

	YM_INFO("Yume Logger initialized.");
	YM_INFO("Yume Engine initialized.");

	auto app = Yume::createApplication();
	app->run();
	delete app;

	return 0;
}
