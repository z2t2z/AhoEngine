#pragma once

#include "Runtime/Function/Renderer/Texture.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"

namespace Aho {
	class IBL {
	public:
		IBL(Texture* texture);
		void Bind1DCDF(uint32_t slot) const;
		void Bind2DCDF(uint32_t slot) const;
		void Bind2DCDFReference(uint32_t slot) const;

		void BindEnvMap(uint32_t slot) const;
		inline const glm::ivec2 GetSize() const {
			return glm::ivec2(m_EnvTexture->GetWidth(), m_EnvTexture->GetHeight());
		}
		inline float GetTotLuminance() const { 
			return m_EnvTotalLum; 
		}
		uint32_t Get2DCDFReference() const { return m_2DCDF_Reference; }
		uint32_t Get2DCDF() const { return m_2DCDF; }
		uint32_t Get1DCDF() const { return m_1DCDF; }

		Texture* GetEnvMap() const { return m_EnvTexture; }
	private:
		void ConstructCDF();
	private:
		Texture* m_EnvTexture{ nullptr };
		uint32_t m_2DCDF{ 0u };
		uint32_t m_1DCDF{ 0u };
		uint32_t m_2DCDF_Reference{ 0u };
		float m_EnvTotalLum{ 0.0f };
	};
}