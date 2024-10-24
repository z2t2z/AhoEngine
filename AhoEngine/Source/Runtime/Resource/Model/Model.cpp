#include "Ahopch.h"
#include "Model.h"

#include "Runtime/Function/Renderer/Buffer.h"
#include "Runtime/Function/Renderer/VertexArrayr.h"
#include <queue>


namespace Aho {
	void Model::LoadModel() {
        Assimp::Importer importer;
        auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
        const aiScene* scene = importer.ReadFile(m_Path, Flags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            AHO_CORE_ERROR(importer.GetErrorString());
            return;
        }
        // retrieve the directory path of the filepath
        m_Path = m_Path.substr(0, m_Path.find_last_of('/'));

        ProcessNode(scene->mRootNode, scene);
	}

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        std::queue<aiNode*> q;
        q.push(node);
        while (!q.empty()) {
            aiNode* cur_node = q.front();
            q.pop();
            for (uint32_t i = 0; i < cur_node->mNumMeshes; i++) {
                aiMesh* mesh = scene->mMeshes[cur_node->mMeshes[i]];
                ProcessMesh(mesh, scene);
            }
            for (uint32_t i = 0; i < cur_node->mNumChildren; i++) {
                q.push(cur_node->mChildren[i]);
            }
        }
    }

    // TODO : support mutiple texture coords
    void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<float> vertices;
        std::shared_ptr<VertexArray> vertexArray;
        vertexArray.reset(VertexArray::Create());

        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            // Position
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            // Normals
            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }

            // Texture coords, tangent and bitangent
            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);

                vertices.push_back(mesh->mTangents[i].x);
                vertices.push_back(mesh->mTangents[i].y);
                vertices.push_back(mesh->mTangents[i].z);

                vertices.push_back(mesh->mBitangents[i].x);
                vertices.push_back(mesh->mBitangents[i].y);
                vertices.push_back(mesh->mBitangents[i].z);
            }
        }

        std::shared_ptr<VertexBuffer> vertexBuffer;
        vertexBuffer.reset(VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float)));
        
        BufferLayout layout = {
            { ShaderDataType::Float3, "a_Position" }
        };

        if (mesh->HasNormals()) {
            layout.Push({ ShaderDataType::Float3, "a_Normal" });
        }

        if (mesh->HasTextureCoords(0)) {
            layout.Push({ ShaderDataType::Float2, "a_TexCoords" });
            layout.Push({ ShaderDataType::Float3, "a_Tangent" });
            layout.Push({ ShaderDataType::Float3, "a_Bitangent" });
        }

        vertexBuffer->SetLayout(layout);
        uint32_t offset = 0;
        vertexArray->AddVertexBuffer(vertexBuffer, offset);

        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::shared_ptr<IndexBuffer> indexBuffer;
        indexBuffer.reset(IndexBuffer::Create(indices.data(), indices.size()));
        vertexArray->SetIndexBuffer(indexBuffer);

        m_VAOs.push_back(vertexArray);
    }

}