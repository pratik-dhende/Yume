#include "ympch.h"
#include "Yume.h""

class Sandbox : public Yume::Application {
public:
	Sandbox() {

	}
	
	~Sandbox() {

	}
};

std::unique_ptr<Yume::Application> Yume::createApplication() {
	return std::make_unique<Sandbox>();
}