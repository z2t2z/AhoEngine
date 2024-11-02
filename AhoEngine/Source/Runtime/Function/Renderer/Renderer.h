#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include <memory>
#include <typeindex>
#include <glm/glm.hpp>

namespace Aho {
	// TODO; Seems like a big bad way
	class GlobalState {
	public:
		static uint32_t g_SelectedEntityID;
		static bool g_ShowDebug;
	};

	class Renderer {
	public:
		Renderer();
		~Renderer() {
			for (auto& pipeline : m_Pipelines) {
				delete pipeline;
			}
		}

		void Render() {
			m_CurrentPipeline->Execute();
		}
		
		void AddRenderPipeline(RenderPipeline* pl) { 
			if (std::find(m_Pipelines.begin(), m_Pipelines.end(), pl) == m_Pipelines.end()) {
				m_Pipelines.push_back(pl); 
			}
		}
		
		void SetCurrentRenderPipeline(RenderPipeline* pl) { 
			if (std::find(m_Pipelines.begin(), m_Pipelines.end(), pl) == m_Pipelines.end()) {
				AddRenderPipeline(pl); 
			}
			m_CurrentPipeline = pl; 
		}
		
		RenderPipeline* GetPipeline(RenderPipelineType type) { 
			auto it = std::find_if(m_Pipelines.begin(), m_Pipelines.end(), [type](RenderPipeline* p) {
				return p->GetType() == type;
			});
			return it != m_Pipelines.end() ? *it : nullptr;
		}

		RenderPipeline* GetCurrentRenderPipeline() { return m_CurrentPipeline; }

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	public:
		std::vector<RenderPipeline*>::iterator begin() { return m_Pipelines.begin(); }
		std::vector<RenderPipeline*>::iterator end() { return m_Pipelines.end(); }
		std::vector<RenderPipeline*>::const_iterator begin() const { return m_Pipelines.begin(); }
		std::vector<RenderPipeline*>::const_iterator end() const { return m_Pipelines.end(); }
	private:
		RenderPipeline* m_CurrentPipeline{ nullptr };
		std::vector<RenderPipeline*> m_Pipelines;
	};
}

