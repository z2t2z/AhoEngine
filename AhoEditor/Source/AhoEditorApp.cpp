#include "IamAho.h"

#include "AhoEditorLayer.h"
#include "Core/EntryPoint.h"

namespace Aho {

}

class AhoEditor : public Aho::Application {
public:

	AhoEditor() {
		PushLayer(new Aho::AhoEditorLayer());
	}

	~AhoEditor() {

	}

private:

};


Aho::Application* Aho::CreateApplication() {
	return new AhoEditor();
}