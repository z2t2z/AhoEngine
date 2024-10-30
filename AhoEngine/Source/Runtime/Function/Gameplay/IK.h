#pragma once
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/Core.h"
#include "Runtime/Function/SkeletonViewer.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include <glm/gtx/rotate_vector.hpp>

namespace Aho {
    class IKSolver {
    public:
        // TODO: Extra bones come in with assimp should be removed?
        static void FABRIK(SkeletonViewer* viewer, std::vector<glm::mat4>& globalMatrices,
            BoneNode* root, BoneNode* endEffector, const glm::vec3& target, 
            int MAX_ITER = 20, float eps = 1E-4) {

            // Step 1: Check if the target is within reach
            std::vector<BoneNode*> chains;
            std::vector<glm::vec3> positions;
            float sumLength = BuildChains(viewer->GetTransformMap(), root, endEffector, chains, positions);
            int n = positions.size();

            // Step 2 & 3: Forward and Backward Reaching
            bool reached = false;
            while (!reached && MAX_ITER--) {
                // Backward reaching
                positions.back() = target;
                for (int i = n - 2; i >= 0; i--) {
                    float dist = glm::distance(positions[i], positions[i + 1]);
                    float length = glm::distance(GetPosition(chains[i]->global), GetPosition(chains[i + 1]->global)); // The distance between joints
                    positions[i] = positions[i + 1] + (positions[i] - positions[i + 1]) * length / dist;
                }
                // Forward reaching
                positions[0] = GetPosition(root->global);
                for (int i = 1; i < n; i++) {
                    float dist = glm::distance(positions[i], positions[i - 1]);
                    float length = glm::distance(GetPosition(chains[i]->global), GetPosition(chains[i - 1]->global)); // The distance between joints
                    positions[i] = positions[i - 1] + (positions[i] - positions[i - 1]) * length / dist;
                }
                // Check if close enough to the target
                reached = glm::distance(positions.back(), target) <= eps;
            }

            // Step 4: Update Model Positions
            auto update = [&](auto self, BoneNode* curr, glm::mat4 globalMatrix4) -> void {
                globalMatrix4 = globalMatrix4 * curr->transform;
                curr->global = globalMatrix4;
                if (curr->hasInfluence) {
                    globalMatrices[curr->bone.id] = globalMatrix4;
                }
                for (auto child : curr->children) {
                    self(self, child, globalMatrix4);
                }
            };

            for (int i = 1; i < n; i++) {
                glm::vec3 O = GetPosition(chains[i - 1]->global);
                glm::vec3 A = GetPosition(chains[i]->global);
                glm::vec3 B = positions[i];
                glm::vec3 OA = glm::normalize(A - O);
                glm::vec3 OB = glm::normalize(B - O);
                float cosTheta = glm::dot(OA, OB);
                if (glm::abs(cosTheta - 1.0f) < 1e-6 || glm::abs(cosTheta + 1.0f) < 1e-6) {
                    continue;
                }
                auto rotationAxis = glm::cross(OA, OB);
                glm::quat quat = glm::angleAxis(glm::acos(cosTheta), glm::normalize(rotationAxis));
                glm::mat4 rotMat = glm::toMat4(quat);

                glm::mat4 globalMatrix4 = chains[i - 1]->global;
                glm::vec3 temp = glm::vec3(globalMatrix4[3].x, globalMatrix4[3].y, globalMatrix4[3].z);
                globalMatrix4[3].x = globalMatrix4[3].y = globalMatrix4[3].z = 0.0f;
                globalMatrix4 = rotMat * globalMatrix4;
                globalMatrix4[3].x = temp.x, globalMatrix4[3].y = temp.y, globalMatrix4[3].z = temp.z, globalMatrix4[3].w = 1.0f;
                //globalMatrix4 = globalMatrix4 * rotMat * curr->transform;
                globalMatrices[chains[i - 1]->bone.id] = globalMatrix4;
                chains[i - 1]->global = globalMatrix4;
                for (auto child : chains[i - 1]->children) {
                    update(update, child, globalMatrix4);
                }
                {
                    glm::vec3 currPos = GetPosition(chains[i]->global), parentPos = GetPosition(chains[i - 1]->global);
                    glm::vec3 PtoC = currPos - parentPos;
                    glm::vec3 d(0.0f, -1.0f, 0.0f);
                    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::length(PtoC)));
                    auto axis = glm::cross(d, PtoC);
                    float angle = glm::acos(glm::dot(d, glm::normalize(PtoC)));
                    glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
                    glm::mat4 rotation = glm::toMat4(q);
                    glm::mat4 translation = glm::translate(glm::mat4(1.0f), parentPos);
                    viewer->Update(chains[i], translation * rotation * scale);
                }
            }
        }

    private:
        glm::vec3 static GetPosition(const glm::mat4& transform) {
            return glm::vec3(transform[3]);
        }

        void static UpdateChain(const BoneNode*& node) {

        }

        // return chains total length
        float static BuildChains(const std::map<BoneNode*, glm::mat4>& pose, BoneNode* start, BoneNode* endEffector, std::vector<BoneNode*>& chains, std::vector<glm::vec3>& positions) {
            float totalLength = 0.0f;
            AHO_CORE_ASSERT(start != endEffector, "Ik chains has length equal to one");
            AHO_CORE_ASSERT(!endEffector->children.empty(), "Has not implemented yet");
            auto curr = endEffector;
            while (true) {
                positions.push_back(GetPosition(curr->global));
                chains.push_back(curr);
                if (curr == start) {
                    break;
                }
                glm::vec3 A = glm::vec3(GetPosition(curr->global));
                auto prev = curr->parent;
                if (!prev) {
                    AHO_CORE_ASSERT(false, "Wrong IK chains");
                }
                glm::vec3 B = glm::vec3(GetPosition(prev->global));
                totalLength += glm::length(A - B);
                curr = prev;
            }
            std::reverse(positions.begin(), positions.end());
            std::reverse(chains.begin(), chains.end());
            return totalLength;
        }
    };
}