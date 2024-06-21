#include "ympch.h"
#include "Yume.h""

class Sandbox : public Yume::Application {
public:
	Sandbox() {

	}
	
	~Sandbox() {

	}

	void init() override
	{
		YM_INFO("INIT");
	}

	void update() override
	{
		YM_INFO("UPDATE");
	}
};

std::unique_ptr<Yume::Application> Yume::createApplication() {
	return std::make_unique<Sandbox>();
}