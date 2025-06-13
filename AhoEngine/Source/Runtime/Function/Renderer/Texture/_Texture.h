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
		void BindUnit(uint32_t slot) const { glBindTextureUnit(slot, m_TextureID); }
		void BindTextureImage(uint32_t pos, uint32_t mipLevel = 0, uint32_t access = GL_WRITE_ONLY) const; // For compute shader
		void ClearTextureData() const;
		void GenMipMap();
		void GetTextureWdithHeight(int& width, int& height, int mipLevel = 0) const;
		bool Resize(uint32_t width, uint32_t height);
		void SetUsage(const TextureUsage usage) { m_Usage = usage; }
		uint64_t GetTextureHandle();
		std::string GetLabel()			const { return m_Label; }
		bool IsHDR()					const { return m_IsHDR; }
		int GetMipLevels()				const { return m_MipLevels; }
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

		bool			m_IsHDR{ false };
		bool			m_GenMips{ false };
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
