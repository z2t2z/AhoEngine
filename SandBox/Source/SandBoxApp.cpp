#include "IamAho.h"
#include "imgui.h"

#include "Runtime/Core/App/EntryPoint.h"

class ExampleLayer : public Aho::Layer {
public:
	ExampleLayer() : Layer("Example Layer") {}

	void OnUpdate(float deltaTime) override {

	}

	virtual void OnImGuiRender() override {

	}
	
	void OnEvent(Aho::Event& event) override {

	}
};


class SandBox : public Aho::Application {
public:

	SandBox() {
		//PushLayer(new ExampleLayer());
	}

	~SandBox() {

	}

private:


};


Aho::Application* Aho::CreateApplication() {
	return new SandBox();
}