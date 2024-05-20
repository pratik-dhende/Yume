#pragma once

extern Yume::Application* Yume::createApplication();

int main(int argc, char** argv) {

	auto app = Yume::createApplication();
	app->run();
	delete app;

	return 0;
}
