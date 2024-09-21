#include "IamAho.h"

#include "AhoEditorLayer.h"
#include "Core/EntryPoint.h"

namespace Aho {
	class AhoEditor : public Application {
	public:
		AhoEditor() {
			PushLayer(new AhoEditorLayer());
		}

		~AhoEditor() {

		}

	private:

	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}