#pragma once

#include "Core.h"
#include <string>
#include <glm/glm.hpp>

namespace Aho {
	void printMat4(const glm::mat4& mat) {
		AHO_CORE_TRACE(FormatMat4(mat));
	}
	std::string FormatMat4(const glm::mat4& mat) {
		std::string res = "\n";
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				float value = mat[row][col];
				res += std::to_string(value);
				res += " \n"[col == 3];
			}
		}
		return res;
	}
}
