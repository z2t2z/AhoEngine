#include "IamAho.h"

#include "imgui.h"
#include "Core/EntryPoint.h"

class ExampleLayer : public Aho::Layer {
public:
	ExampleLayer() : Layer("Example Layer") {}

	void OnUpdate(float deltaTime) override {
		AHO_INFO("Example Layer Update");
	}

	virtual void OnImGuiRender() override {
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}
	
	void OnEvent(Aho::Event& event) override {
		if (event.GetEventType() == Aho::EventType::KeyPressed) {
			Aho::KeyPressedEvent& e = (Aho::KeyPressedEvent&)event;
			if (e.GetKeyCode() == AHO_KEY_TAB)
				AHO_TRACE("Tab key is pressed (event)!");
			AHO_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
};


class SandBox : public Aho::Application {
public:

	SandBox() {
		PushLayer(new ExampleLayer());
	}

	~SandBox() {

	}

private:


};


Aho::Application* Aho::CreateApplication() {
	return new SandBox();
}