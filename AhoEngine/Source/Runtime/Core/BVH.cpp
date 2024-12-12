#include "Ahopch.h"
#include "BVH.h"

namespace Aho {
	namespace Utils {
		glm::vec4 DecodeUint32ToVec4(uint32_t encodedColor) {
			// 提取每个通道的 8 位值
			float r = (encodedColor >> 24) & 0xFF; // 提取红色通道
			float g = (encodedColor >> 16) & 0xFF; // 提取绿色通道
			float b = (encodedColor >> 8) & 0xFF;  // 提取蓝色通道
			float a = encodedColor & 0xFF;         // 提取透明度通道

			// 将 [0, 255] 的值归一化到 [0, 1]
			return glm::vec4(r, g, b, a) / 255.0f;
		}
	}
	
	const std::unique_ptr<BVHNode>& BVH::AddPrimitives(std::vector<std::unique_ptr<Primitive>>& primitives) {
		m_Primitives.reserve(m_Primitives.size() + primitives.size());
		for (auto& primitive : primitives) {
			m_Primitives.push_back(std::move(primitive));
		}
		primitives.clear();
		return m_Root;
	}

	std::vector<std::unique_ptr<Primitive>> BVH::GeneratePrimitives(const std::shared_ptr<StaticMesh>& mesh, 
																	const std::unordered_map<std::string, std::shared_ptr<Texture2D>>& textureCached) {
		std::vector<std::unique_ptr<Primitive>> primitives;
		primitives.reserve(mesh->GetVerticesCount() / 3);
		// Assume triangle
		const auto& meshInfo = mesh->GetMeshInfo();
		for (const auto& info : meshInfo) {
			const auto& vertices = info->vertexBuffer;
			const auto& indices = info->indexBuffer;
			size_t siz = indices.size();
			AHO_CORE_ASSERT(siz % 3 == 0);
			for (size_t i = 0; i < siz; i += 3) {
				const Vertex& v0 = vertices[indices[i]];
				const Vertex& v1 = vertices[indices[i + 1]];
				const Vertex& v2 = vertices[indices[i + 2]];
				PrimitiveMaterial mat0, mat1, mat2;

				auto FillMaterial = [&](const Vertex& vertex, PrimitiveMaterial& mat) -> void {
					if (info->materialInfo.HasMaterial()) {
						for (const auto& [type, path] : info->materialInfo.materials) {
							const auto& tex = textureCached.at(path);
							switch (type) {
								case TexType::Albedo:
									mat.m_Albedo = Utils::DecodeUint32ToVec4(tex->ReadPixel(vertex.uv.x, vertex.uv.y));
									break;
								case TexType::Normal:
									mat.m_Normal = Utils::DecodeUint32ToVec4(tex->ReadPixel(vertex.uv.x, vertex.uv.y));
									break;
								case TexType::Metallic:
									mat.m_PBR.x = Utils::DecodeUint32ToVec4(tex->ReadPixel(vertex.uv.x, vertex.uv.y)).x;
									break;
								case TexType::Roughness:
									mat.m_PBR.y = Utils::DecodeUint32ToVec4(tex->ReadPixel(vertex.uv.x, vertex.uv.y)).x;
									break;
							}
						}
					}
				};
				//FillMaterial(v0, mat0);
				//FillMaterial(v1, mat1);
				//FillMaterial(v2, mat2);

				primitives.emplace_back(std::make_unique<Primitive>(v0, v1, v2, mat0, mat1, mat2));
			}
		}
		return primitives;
	}

	void BVH::BuildTree() {
		//for (auto& p : m_Primitives) {
		//	p->Update();
		//}
		BuildTreeRecursion(0, m_Primitives.size());
	}

	std::optional<IntersectResult> BVH::GetIntersectionRecursion(const Ray& ray, BVHNode* node) {
		//AHO_CORE_ASSERT(node);
		if (!node) {
			return std::nullopt;
		}

		if (!Intersect(ray, node->aabb)) {
			return std::nullopt;
		}
		if (!node->primitives.empty()) {
			const Primitive* p = node->primitives[0];
			auto intersectResult = Intersect(ray, p);
			if (intersectResult) {
				return intersectResult;
			}
		}
		auto intersectResult = GetIntersectionRecursion(ray, node->left.get());
		return intersectResult ? intersectResult : GetIntersectionRecursion(ray, node->right.get());
	}

	std::unique_ptr<BVHNode> BVH::BuildTreeRecursion(int indexL, int indexR) {
		std::unique_ptr<BVHNode> node = std::make_unique<BVHNode>();

		AABB aabb;
		for (int i = indexL; i < indexR; i++) {
			const auto& p = m_Primitives[i];
			AHO_CORE_ASSERT(p);
			aabb.Expand(p->GetAABB());
		}
		node->aabb = aabb;

		int length = indexR - indexL;
		if (length == 1) {
			node->primitives = { m_Primitives[indexL].get() };
		}
		else if (length == 2) {
			node->left = BuildTreeRecursion(indexL, indexL + 1);
			node->right = BuildTreeRecursion(indexL + 1, indexR);
		}
		else {
			int axis = (int)aabb.GetSplitAxis();

			std::sort(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR, 
				[axis](const std::unique_ptr<Primitive>& lhs, const std::unique_ptr<Primitive>& rhs) {
					return lhs->GetCentroid()[axis] < rhs->GetCentroid()[axis];
				});

			node->left = BuildTreeRecursion(indexL, indexL + length / 2);
			node->right = BuildTreeRecursion(indexL + length / 2, indexR);
		}

		return node;
	}
}

