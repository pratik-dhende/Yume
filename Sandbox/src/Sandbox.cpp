#include "ympch.h"
#include "Yume.h""

class Sandbox : public Yume::Application {
public:
	Sandbox() {

	}
	
	~Sandbox() {

	}
};

Yume::Application* Yume::createApplication() {
	return new Sandbox();
}