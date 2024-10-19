#pragma once

#include <memory>
#include "glm/glm.hpp"
#include "Buffer.h"
#include "VertexArrayr.h"

namespace Aho {
	enum class ClearFlags {
		None = 0,
		Color_Buffer = 0x00004000,
		Depth_Buffer = 0x00000100,
		Stencil_Buffer = 0x00000400,
	};
	inline ClearFlags operator|(ClearFlags lhs, ClearFlags rhs) {
		return static_cast<ClearFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	inline ClearFlags& operator|=(ClearFlags& lhs, ClearFlags rhs) {
		lhs = lhs | rhs;
		return lhs;
	}

	class RendererAPI {
	public:
		enum class API {
			None = 0, 
			OpenGL = 1
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear(ClearFlags flags) = 0;
		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;
		virtual void SetDepthTest(bool state) = 0;
		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

}