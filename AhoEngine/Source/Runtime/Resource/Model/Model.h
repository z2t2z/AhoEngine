#pragma once

#include "Runtime/Core/Core.h"

#include "Runtime/Platform/OpenGL/OpenGLBuffer.h"
#include "Runtime/Platform/OpenGL/OpenGLVertexArray.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>

namespace Aho {
	/* Deprecated! */
	class Model {
	public:
		Model() = default;
		Model(const std::string& path) : m_Path(path) { LoadModel(); }
		~Model() = default;
		std::vector<std::shared_ptr<VertexArray>>::iterator begin() { return m_VAOs.begin(); }
		std::vector<std::shared_ptr<VertexArray>>::iterator end() { return m_VAOs.end(); }
		void LoadModel();
		void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene);
	private:
		std::string m_Path;
		std::vector<std::shared_ptr<VertexArray>> m_VAOs;
	};
}
