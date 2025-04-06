#include "Ahopch.h"
#include "IBL.h"

#include <numeric>

namespace Aho {
	inline float RGBToLuminance(const glm::vec3& rgb) {
		return 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;
	}

	IBL::IBL(Texture* texture) {
		m_EnvTexture = texture;
		ConstructCDF();
	}

	void IBL::Bind1DCDF(uint32_t slot) const {
		glBindTextureUnit(slot, m_1DCDF);
	}

	void IBL::Bind2DCDF(uint32_t slot) const {
		glBindTextureUnit(slot, m_2DCDF);
	}

	void IBL::BindEnvMap(uint32_t slot) const {
		m_EnvTexture->Bind(slot);
	}

	void IBL::ConstructCDF() {
		const uint32_t width = m_EnvTexture->GetWidth();
		const uint32_t height = m_EnvTexture->GetHeight();
		GLuint dataformat = Utils::GetGLParam(m_EnvTexture->GetTextureSpec().dataFormat);
		AHO_CORE_ASSERT(dataformat == GL_RGB || dataformat == GL_RGBA);
		const uint32_t channel = dataformat == GL_RGBA ? 4 : 3;

		GLfloat* pixels = new GLfloat[width * height * channel];
		glGetTexImage(GL_TEXTURE_2D, 0, dataformat, GL_FLOAT, pixels); // rgba?

		std::vector<float> lum(width * height);
		for (int v = 0; v < height; v++) {
			for (int u = 0; u < width; u++) {
				int idx = v * width + u;
				lum[idx] = RGBToLuminance(glm::vec3(pixels[idx * channel], pixels[idx * channel + 1], pixels[idx * channel + 2]));
			}
		}

		std::vector<float> col(width);
		for (int v = 0; v < height; v++) {
			for (int u = 0; u < width; u++) {
				col[u] += lum[v * width + u];
			}
		}
		
		float sum = std::accumulate(col.begin(), col.end(), 0.0f);
		m_EnvTotalLum = sum;
		std::vector<float> marginalCDF(width);
		marginalCDF[0] = col[0] / sum;
		for (int u = 1; u < width; u++) {
			marginalCDF[u] = marginalCDF[u - 1] + col[u] / sum;
		}

		// TO FIX: division by zero
		// TO FIX: slow indexing
		std::vector<float> condCDF(width * height);
		for (int u = 0; u < width; u++) {
			condCDF[0 * width + u] = lum[0 * width + u] / col[u];
			for (int v = 1; v < height; v++) {
				condCDF[v * width + u] = condCDF[(v - 1) * width + u] + lum[v * width + u] / col[u];
			}
		}

		glGenTextures(1, &m_1DCDF);
		glBindTexture(GL_TEXTURE_1D, m_1DCDF);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, width, 0, GL_RED, GL_FLOAT, marginalCDF.data());
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenTextures(1, &m_2DCDF);
		glBindTexture(GL_TEXTURE_2D, m_2DCDF);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, condCDF.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		delete[] pixels;
	}
	
}
