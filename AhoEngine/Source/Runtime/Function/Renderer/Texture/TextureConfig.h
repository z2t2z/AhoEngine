#pragma once
#include "TextureUsage.h"
#include <string>
#include <glad/glad.h>

namespace Aho {
	enum TextureDim : GLenum {
		Texture2D = GL_TEXTURE_2D,
		CubeMap = GL_TEXTURE_CUBE_MAP
	};

	enum InternalFormat : GLint {
		R32F = GL_R32F,
		RGBA8 = GL_RGBA8,
		SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
		RGBA16F = GL_RGBA16F,
		RGB16F = GL_RGB16F,
		RG16F = GL_RG16F,
		RGBA32F = GL_RGBA32F,
		Depth24Stencil8 = GL_DEPTH24_STENCIL8,
		Depth24 = GL_DEPTH_COMPONENT24,
		Depth32F = GL_DEPTH_COMPONENT32F,
		// ...
	};

	enum DataFormat : GLenum {
		Red = GL_RED,
		RG = GL_RG,
		RGB = GL_RGB,
		BGR = GL_BGR,
		RGBA = GL_RGBA,
		BGRA = GL_BGRA,
		Depth = GL_DEPTH_COMPONENT,
		DepthStencil = GL_DEPTH_STENCIL,
		// ...
	};

	enum DataType : GLenum {
		UByte = GL_UNSIGNED_BYTE,
		UInt = GL_UNSIGNED_INT,
		UShort = GL_UNSIGNED_SHORT,
		Float = GL_FLOAT,
		UInt248 = GL_UNSIGNED_INT_24_8,
		// ...
	};

	struct TextureConfig {
		TextureUsage	Usage{ TextureUsage::Custom };
		std::string		Label;
		uint32_t		Width{ 1600 };
		uint32_t		Height{ 900 };
		bool			GenMips{ false };

		TextureDim		Dim = TextureDim::Texture2D;
		InternalFormat	InternalFmt = InternalFormat::RGBA8;
		DataFormat		DataFmt = DataFormat::BGRA;
		DataType		DataType = DataType::UByte;

		static TextureConfig GetDepthTextureConfig(const std::string& label = "Depth", bool GenMip = false) {
			TextureConfig depthConfig;
			depthConfig.Usage = TextureUsage::Depth;
			depthConfig.Label = label;
			depthConfig.Width = 1600;
			depthConfig.Height = 1600;
			depthConfig.GenMips = GenMip;
			depthConfig.Dim = TextureDim::Texture2D;
			depthConfig.InternalFmt = InternalFormat::Depth32F;
			depthConfig.DataFmt = DataFormat::Depth;
			depthConfig.DataType = DataType::Float;
			return depthConfig;
		}

		static TextureConfig GetColorTextureConfig(const std::string& label = "Shading") {
			TextureConfig colorConfig;
			colorConfig.Usage = TextureUsage::Custom;
			colorConfig.Label = label;
			colorConfig.Width = 1600;
			colorConfig.Height = 900;
			colorConfig.GenMips = false;
			colorConfig.Dim = TextureDim::Texture2D;
			colorConfig.InternalFmt = InternalFormat::RGBA8;
			colorConfig.DataFmt = DataFormat::RGBA;
			colorConfig.DataType = DataType::UByte;
			return colorConfig;
		}

		static TextureConfig GetCubeMapTextureConfig(const std::string& label = "Cubemap") {
			TextureConfig cubemapConfig;
			cubemapConfig.Usage = TextureUsage::IBL_CubeMap;
			cubemapConfig.Label = label;
			cubemapConfig.Width = 512;
			cubemapConfig.Height = 512;
			cubemapConfig.GenMips = false;
			cubemapConfig.Dim = TextureDim::CubeMap;
			cubemapConfig.InternalFmt = InternalFormat::RGBA16F;
			cubemapConfig.DataFmt = DataFormat::RGBA;
			cubemapConfig.DataType = DataType::Float;
			return cubemapConfig;
		}
	};
}
