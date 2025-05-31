#include "Ahopch.h"
#include "ShaderAsset.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"

#include <fstream>
#include <glad/glad.h>

namespace Aho {
	namespace Utils {
		static GLenum ShaderTypeFromString(const std::string& type) {
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;
			if (type == "compute")
				return GL_COMPUTE_SHADER;
			AHO_CORE_ASSERT(false, "Unknown shader type: " + type);
			return 0;
		}
	}

	ShaderAsset::ShaderAsset(const std::string& path, const ShaderUsage& usage)
		: Asset(path), m_Usage(usage) {
		Load();
	}

	bool ShaderAsset::Load() {
		std::string source = ReadFile(m_Path);
		if (source.empty()) {
			return false;
		}
		std::unordered_map<uint32_t, std::string> srcMap;
		Preprocess(srcMap, source);
		ReplaceIncludes(srcMap);
		if (srcMap.empty()) {
			AHO_CORE_ERROR("ShaderAsset::No valid shader source found in file: {0}", m_Path);
			return false;
		}
		m_OpenGLSourceCode = std::move(srcMap);
		return true;
	}

	std::string ShaderAsset::ReadFile(const std::string& filepath) {
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else {
				AHO_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else {
			AHO_CORE_ERROR("Could not open file '{0}'", filepath);
		}
		if (result.empty()) {
			AHO_CORE_WARN("No content from file:" + filepath);
		}
		return result;
	}

	void ShaderAsset::Preprocess(std::unordered_map<uint32_t, std::string>& umap, const std::string& src) {
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = src.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos) {
			size_t eol = src.find_first_of("\r\n", pos); //End of shader type declaration line
			AHO_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = src.substr(begin, eol - begin);
			AHO_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			m_Type = Utils::ShaderTypeFromString(type) == GL_COMPUTE_SHADER ? ShaderType::Compute : ShaderType::Normal;
			
			size_t nextLinePos = src.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			AHO_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = src.find(typeToken, nextLinePos); //Start of next shader type declaration line
			umap[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? src.substr(nextLinePos) : src.substr(nextLinePos, pos - nextLinePos);
		}
	}

	void ShaderAsset::ReplaceIncludes(std::unordered_map<uint32_t, std::string>& umap) {
		namespace fs = std::filesystem;
		std::string basePath = fs::path(m_Path).parent_path().string() + '/';
		fs::path basePathfs = fs::path(m_Path).parent_path();

		// TODO: Don't hard code this!!
		auto rootPath = fs::current_path();
		rootPath = rootPath / "ShaderSrc";

		auto processIncludesRecursion = [&](auto&& self, const std::string& source, const std::string& currBasePath, std::unordered_set<std::string>& includedFiles) -> std::string {
			std::stringstream processed;
			std::istringstream sourceStream(source);
			std::string line;

			while (std::getline(sourceStream, line)) {
				auto commentStartPos = line.find("//");
				if (commentStartPos != std::string::npos) {
					line = line.substr(0, commentStartPos);
				}
				if (line.find("#include") != std::string::npos) {
					size_t start = line.find("\"");
					size_t end = line.rfind("\"");
					if (start != std::string::npos && end != std::string::npos && start < end) {
						std::string includeFilePath = line.substr(start + 1, end - start - 1);
						std::string fullPath = currBasePath + includeFilePath;

						if (includedFiles.find(fullPath) != includedFiles.end()) {
							//AHO_CORE_WARN("Warning: Circular include detected for file: {}", fullPath);
							continue;
						}

						includedFiles.insert(fullPath);
						try {
							std::string includeContent = ReadFile(fullPath);
							processed << self(self, includeContent, fs::path(fullPath).parent_path().string() + '/', includedFiles);
						}
						catch (const std::runtime_error& e) {
							AHO_CORE_ERROR(e.what());
						}

						continue;
					}
				}
				processed << line << '\n';
			}

			return processed.str();
		};

		for (auto& [shaderType, source] : umap) {
			std::unordered_set<std::string> includedFiles;
			source = processIncludesRecursion(processIncludesRecursion, source, basePath, includedFiles);
		}
	}

}