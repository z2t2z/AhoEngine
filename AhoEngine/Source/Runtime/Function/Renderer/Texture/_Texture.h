#pragma once

#include "TextureUsage.h"
#include "TextureConfig.h"
#include <vector>

namespace Aho {
	class TextureAsset;
	class _Texture {
	public:
		_Texture(const TextureConfig& cfg);
		_Texture(const std::shared_ptr<TextureAsset>& texAsset);
		~_Texture();
		//_Texture(_Texture&&) noexcept;
		//_Texture& operator=(_Texture&&) noexcept;
		_Texture(const _Texture&) = delete;
		_Texture& operator=(const _Texture&) = delete;
		void BindTextureImage(uint32_t pos) const; // For compute shader
		void ClearTextureData(const void* data) const {
			glClearTexImage(m_TextureID, 0, m_DataFmt, m_DataType, data);
		}
		bool Resize(uint32_t width, uint32_t height);
		void SetUsage(const TextureUsage usage) { m_Usage = usage; }
		uint64_t GetTextureHandle()	 { 
			if (m_Handle == 0) {
				m_Handle = glGetTextureHandleARB(m_TextureID);
				glMakeTextureHandleResidentARB(m_Handle);
			}
			return m_Handle; 
		}

		TextureDim GetDim()				const { return m_Dim; }
		InternalFormat GetInternalFmt() const { return m_InternalFmt; }
		DataFormat GetDataFmt()			const { return m_DataFmt; }
		DataType GetDataType()			const { return m_DataType; }
		uint32_t GetTextureID()			const { return m_TextureID; }
		uint32_t GetWidth()				const { return m_Width; }
		uint32_t GetHeight()			const { return m_Height; }
		TextureUsage GetUsage()			const { return m_Usage; }
	private:
		void GliLoader(const std::shared_ptr<TextureAsset>& texAsset);
		void STBImageLoader(const std::shared_ptr<TextureAsset>& texAsset);
	private:
		std::string m_Label;
		TextureUsage m_Usage;

		uint32_t		m_Width{ 0 };
		uint32_t		m_Height{ 0 };
		int				m_MipLevels{ -1 };
		uint32_t		m_Handle{ 0 };
		uint32_t		m_TextureID{ 0 };

		TextureDim		m_Dim;
		InternalFormat	m_InternalFmt;
		DataFormat		m_DataFmt;
		DataType		m_DataType;
	};
}
