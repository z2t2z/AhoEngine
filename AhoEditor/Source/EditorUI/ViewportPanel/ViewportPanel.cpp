#include "ViewportPanel.h"
#include <imgui.h>

namespace Aho {
	bool ViewportPanel::DrawPanel() {

		return m_CursorInViewport;
	}
}