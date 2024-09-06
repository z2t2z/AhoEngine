#pragma once


#ifdef AHO_PLATFORM_WINDOWS


// Will be implemeneted in the client side
extern Aho::Application* Aho::CreateApplication();

int main(int argc, char** argv) {
	Aho::Log::Init();

	AHO_CORE_WARN("Initialized Log!");

	auto app = Aho::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif