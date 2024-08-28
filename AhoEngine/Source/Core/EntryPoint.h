#pragma once


#ifdef AHO_PLATFORM_WINDOWS


// Will be implemeneted in the client side
extern Aho::Application* Aho::CreateApplication();

int main(int argc, char** argv) {

	std::cout << "I am Aho!" << std::endl;

	Aho::Log::Init();

	AHO_CORE_WARN("Initialized Log!");
	int a = 5;
	AHO_INFO("Hello:) Var = {0}", a);

	auto app = Aho::CreateApplication();

	app->Run();

	delete app;

	return 0;
}

#endif