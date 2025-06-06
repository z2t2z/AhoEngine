#include "Ahopch.h"
#include "IBL.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include <numeric>

namespace Aho {
	inline float RGBToLuminance(const glm::vec3& rgb) {
		return 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;
	}

	IBL::IBL(_Texture* texture) {
		m_EnvTexture = texture;
		ConstructCDF();
	}

	IBL::~IBL() {
		if (m_1DCDF) {
			glDeleteTextures(1, &m_1DCDF);
		}
		if (m_2DCDF) {
			glDeleteTextures(1, &m_2DCDF);
		}
		if (m_2DCDF_Reference) {
			glDeleteTextures(1, &m_2DCDF_Reference);
		}
	}

	void IBL::Bind(const Shader* shader) const {
		shader->SetInt("u_EnvMap.EnvLight", 0);
		shader->SetInt("u_EnvMap.Env1DCDF", 1);
		shader->SetInt("u_EnvMap.Env2DCDF", 2);
		//shader->SetInt("u_EnvMap.Env2DCDF_Reference", 3);
		shader->SetIvec2("u_EnvMap.EnvSize", glm::ivec2(GetSize()));
		shader->SetFloat("u_EnvMap.EnvTotalLum", m_EnvTotalLum);
		BindEnvMap(0);
		Bind1DCDF(1);
		Bind2DCDF(2);
		//Bind2DCDFReference(3);
	}

	void IBL::Bind1DCDF(uint32_t slot) const {
		glBindTextureUnit(slot, m_1DCDF);
	}

	void IBL::Bind2DCDF(uint32_t slot) const {
		glBindTextureUnit(slot, m_2DCDF);
	}

	void IBL::Bind2DCDFReference(uint32_t slot) const {
		glBindTextureUnit(slot, m_2DCDF_Reference);
	}

	void IBL::BindEnvMap(uint32_t slot) const {
		glBindTextureUnit(slot, m_EnvTexture->GetTextureID());
	}

	void IBL::ConstructCDF() {
		const uint32_t width = m_EnvTexture->GetWidth();
		const uint32_t height = m_EnvTexture->GetHeight();
		GLuint dataformat = m_EnvTexture->GetDataFmt();
		AHO_CORE_ASSERT(dataformat == GL_RGB || dataformat == GL_RGBA);
		const uint32_t channel = dataformat == GL_RGBA ? 4 : 3;

		GLfloat* pixels = new GLfloat[width * height * channel];
		glBindTexture(GL_TEXTURE_2D, m_EnvTexture->GetTextureID());
		glGetTexImage(GL_TEXTURE_2D, 0, dataformat, m_EnvTexture->GetDataType(), pixels); // rgba?

		std::vector<float> lum(width * height);
		for (int v = 0; v < height; v++) {
			for (int u = 0; u < width; u++) {
				int idx = v * width + u;
				lum[idx] = RGBToLuminance(glm::vec3(pixels[idx * channel], pixels[idx * channel + 1], pixels[idx * channel + 2]));
			}
		}

		// DELETE THIS SOME DAY
		{
			//std::vector<float> cdf(height * width);
			//cdf[0] = lum[0];
			//for (int i = 1; i < height * width; i++) {
			//	cdf[i] = cdf[i - 1] + lum[i];
			//}
			//m_EnvTotalLum = cdf[height * width - 1];
			//glGenTextures(1, &m_2DCDF_Reference);
			//glBindTexture(GL_TEXTURE_2D, m_2DCDF_Reference);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, cdf.data());
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//delete pixels;
			//return;
		}

		std::vector<float> col(width);
		for (int v = 0; v < height; v++) {
			for (int u = 0; u < width; u++) {
				col[u] += lum[v * width + u];
			}
		}
		
		m_EnvTotalLum = std::accumulate(col.begin(), col.end(), 0.0f);
		std::vector<float> marginalCDF(width);
		marginalCDF[0] = col[0] / m_EnvTotalLum;
		for (int u = 1; u < width; u++) {
			marginalCDF[u] = marginalCDF[u - 1] + col[u] / m_EnvTotalLum;
		}
		AHO_CORE_ASSERT((marginalCDF.back() - 1.0f) <= 0.00001);

		// TO FIX: division by zero
		// TO FIX: slow indexing
		std::vector<float> condCDF(width * height);
		for (int u = 0; u < width; u++) {
			condCDF[0 * width + u] = lum[0 * width + u] / col[u];
			for (int v = 1; v < height; v++) {
				condCDF[v * width + u] = condCDF[(v - 1) * width + u] + lum[v * width + u] / col[u];
			}
			AHO_CORE_ASSERT((condCDF[(width - 1) * (height - 1) + u] - 1.0f) <= 0.00001);
		}

		glGenTextures(1, &m_1DCDF);
		glBindTexture(GL_TEXTURE_1D, m_1DCDF);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, width, 0, GL_RED, GL_FLOAT, marginalCDF.data());
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_1D, 0);

		glGenTextures(1, &m_2DCDF);
		glBindTexture(GL_TEXTURE_2D, m_2DCDF);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, condCDF.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] pixels;
	}
	
}
