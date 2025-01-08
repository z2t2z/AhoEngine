#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Framebuffer.h"
#include <vector>
#include <unordered_map>

namespace Aho {
	enum class RenderPassType {
		None = 0,
		Debug,
		Shading,
		Depth,
		DrawSelected,
		SSAOGeo,
		SSAO,
		SSAOLighting,
		BlurR,
		BlurRGB,
		SSRvs,
		HiZ,
		DrawLine,
		PostProcessing,
		PrecomputeIrradiance,
		GenCubemap,
		Prefilter,
		GenLUT,
		FXAA,
		Atmospheric,
		TransmittanceLUT,
		MutiScattLUT,
		SkyViewLUT,
		PathTracing,
		Test,
		/* TODO */
	};
	

	// TODO, think of a better way
	class TextureBuffer {
	public:
		TextureBuffer(Texture* tex, TexType type) : m_Texture(tex), m_Type(type) {}
		Texture* m_Texture;
		TexType m_Type;

	public:
		static void Init() {
			s_TexBufferMap[TexType::Albedo]			= "u_gAlbedo";
			s_TexBufferMap[TexType::Position]		= "u_gPosition";
			s_TexBufferMap[TexType::Normal]			= "u_gNormal";
			s_TexBufferMap[TexType::PBR]			= "u_gPBR";
			s_TexBufferMap[TexType::AO]				= "u_gAO";
			s_TexBufferMap[TexType::Noise]			= "u_gNoise";
			s_TexBufferMap[TexType::Depth]			= "u_gDepth";
			s_TexBufferMap[TexType::LightDepth]		= "u_gLightDepth";
			s_TexBufferMap[TexType::Irradiance]		= "u_gIrradiance";
			s_TexBufferMap[TexType::Prefiltering]	= "u_gPrefilter";
			s_TexBufferMap[TexType::BRDFLUT]		= "u_gLUT";
			s_TexBufferMap[TexType::Entity]			= "u_gEntity";
			s_TexBufferMap[TexType::Result]			= "u_gImage";
			s_TexBufferMap[TexType::HDR]			= "u_gHDR";
			s_TexBufferMap[TexType::CubeMap]		= "u_gCubeMap";

			// Atmosphere rendering
			s_TexBufferMap[TexType::TransmittanceLUT] = "u_TransmittanceLUT";
			s_TexBufferMap[TexType::SkyViewLUT]		  = "u_SkyviewLUT";
			s_TexBufferMap[TexType::MultiScattLUT]	  = "u_MultiScattLUT";
		}
		static const std::string GetTexBufferUniformName(TexType type) { return s_TexBufferMap.at(type); }
	private:
		static std::unordered_map<TexType, std::string> s_TexBufferMap;
	};

	class RenderPass {
	public:
		RenderPass() { AHO_CORE_ASSERT(false); }
		RenderPass(const std::string& name) : m_Name(name) {}
		~RenderPass() {
			m_RenderCommandBuffer.reset();
		}
		virtual void SetRenderTarget(const std::shared_ptr<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		virtual void SetRenderCommand(std::unique_ptr<RenderCommandBuffer> renderCommandBuffer) { m_RenderCommandBuffer = std::move(renderCommandBuffer); }
		virtual void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		virtual RenderPassType GetRenderPassType() { return m_RenderPassType; }
		virtual void SetRenderPassType(RenderPassType type) { m_RenderPassType = type; }
		
		virtual void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData) {
			m_RenderCommandBuffer->Execute(renderData, m_Shader, m_Framebuffer, m_TextureBuffers);
		}

		virtual Texture* GetTextureBuffer(TexType type) {
			if (type == TexType::Depth) {
				return m_Framebuffer->GetDepthTexture();
			}

			const auto& textureBuffer = m_Framebuffer->GetTextureAttachments();
			auto it = std::find_if(textureBuffer.begin(), textureBuffer.end(), 
				[&](const Texture* texBuffer) {
					return texBuffer->GetTexType() == type;
				});
			AHO_CORE_ASSERT(it != textureBuffer.end(), "Did not have this texture buffer: {}");
			return *it;
		}

		virtual void RegisterTextureBuffer(const TextureBuffer& buffer) { 
			auto it = std::find_if(m_TextureBuffers.begin(), m_TextureBuffers.end(), 
				[&](const TextureBuffer& texBuffer) {
					return texBuffer.m_Type == buffer.m_Type;
				});

			if (it != m_TextureBuffers.end()) {
				AHO_CORE_INFO("Erasing texture buffer: `{}` at render pass: `{}`", it->m_Texture->GetDebugName(), GetPassDebugName());
				m_TextureBuffers.erase(it);
			}
			m_TextureBuffers.push_back(buffer); 
		}

		virtual std::shared_ptr<Shader> GetShader() { return m_Shader; }
		std::shared_ptr<Framebuffer> GetRenderTarget() { return m_Framebuffer; }
		std::string GetPassDebugName() { return m_Name; }

	private:
		std::string m_Name;
		std::vector<TextureBuffer> m_TextureBuffers;
		RenderPassType m_RenderPassType{ RenderPassType::None };
		std::shared_ptr<Framebuffer> m_Framebuffer{ nullptr };  // This is the render target of this pass
		std::shared_ptr<Shader> m_Shader{ nullptr };			// Currently each render pass uses a single shader
		std::unique_ptr<RenderCommandBuffer> m_RenderCommandBuffer{ nullptr };
	};

};