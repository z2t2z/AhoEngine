#include <IamAho.h>


class ExampleLayer : public Aho::Layer {
public:
	ExampleLayer() : Layer("Example Layer") {}

	void OnUpdate() override {
		AHO_INFO("Example Layer Update");
	}

	void OnEvent(Aho::Event& event) override {
		AHO_TRACE("{0}", event.ToString());
	}
};


class SandBox : public Aho::Application {
public:

	SandBox() {
		PushLayer(new ExampleLayer());
		PushOverlay(new Aho::ImGuiLayer());
	}

	~SandBox() {

	}

private:


};


Aho::Application* Aho::CreateApplication() {
	
	return new SandBox();

}