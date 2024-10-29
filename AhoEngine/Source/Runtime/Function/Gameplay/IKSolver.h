#pragma once
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/Core.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"

namespace Aho {
	class IKSolver {
	public:
		static void FABRIK(BoneNode* root, BoneNode* endEffector, const glm::vec3& target, int MAX_ITER = 20, float eps = 1E-3) {
            // Step 1: Check if the target is within reach
            std::vector<BoneNode*> chains;
            std::vector<glm::vec3> positions;
            float sumLength = BuildChains(root, endEffector, chains, positions);
            if (sumLength < glm::length(target)) {
                glm::vec3 alpha = glm::normalize(GetPosition(endEffector));
                glm::vec3 beta = glm::normalize(target);
                double cosTheta = glm::dot(alpha, beta);
                double theta = glm::acos(cosTheta);
                glm::vec3 rotateAxis = glm::cross(alpha, beta);
                //root.setRotation(theta, rotateAxis, 1);
                // reset root, then rotate this
                return;
            }

            // Step 2 & 3: Forward and Backward Reaching
            bool reached = false;
            while (!reached && MAX_ITER--) {
                // Backward reaching
                positions.back() = target;
                for (size_t i = positions.size() - 2; i >= 0; i--) {
                    float dist = glm::distance(positions[i], positions[i + 1]);
                    float length = glm::length(GetPosition(chains[i]) - GetPosition(chains[i + 1])); // The distance between joints
                    positions[i] = positions[i + 1] + (positions[i] - positions[i + 1]) * length / dist;
                }

                // Forward reaching
                positions[0] = GetPosition(root);
                for (size_t i = 1; i < positions.size(); i++) {
                    float dist = glm::distance(positions[i], positions[i - 1]);
                    float length = glm::length(GetPosition(chains[i]) - GetPosition(chains[i - 1]));; // The distance between joints
                    positions[i] = positions[i - 1] + (positions[i] - positions[i - 1]) * length / dist;
                }

                // Check if close enough to the target
                reached = glm::distance(positions.back(), target) <= eps;
            }

            // updatePosition :: only update position recursively
            // updateTransform:: only update Transform recursively

            // Step 4: Update Model Positions
            for (size_t i = 0; i < positions.size() - 1; i++) {
                auto O = GetPosition(chains[i]);
                auto A = GetPosition(chains[i + 1]);
                auto B = positions[i];
                auto alpha = glm::normalize(A - O);
                auto beta = glm::normalize(B - O);
                float cosTheta = glm::dot(alpha, beta);
                auto rotationAxis = glm::cross(alpha, beta);
                if (glm::abs(cosTheta - 1.0f) < 1e-6 || glm::abs(cosTheta + 1.0f) < 1e-6) {
                    continue;
                }
                double theta = glm::acos(cosTheta);
                double thetaA = glm::degrees(theta);

            }

            //size_t i = 1;
            //for (auto& cur = root; i < positions.size() and cur != &endEffector; cur = cur->childrens[0], i++) {
            //    auto O = cur->getPosition();
            //    auto A = cur->childrens[0]->getPosition();
            //    auto B = positions[i];
            //    auto alpha = glm::normalize(A - O);
            //    auto beta = glm::normalize(B - O);
            //    float cosTheta = glm::dot(alpha, beta);
            //    auto rotationAxis = glm::cross(alpha, beta);
            //    if (glm::abs(cosTheta - 1.0f) < 1e-6 || glm::abs(cosTheta + 1.0f) < 1e-6) {
            //        continue;
            //    }
            //    double theta = glm::acos(cosTheta);
            //    double thetaA = glm::degrees(theta);
            //    cur->setRotation(thetaA, rotationAxis, false);
            //    root.updatePosition(); // why?
            //}
		}

    private:
        glm::vec3 static GetPosition(BoneNode* node) {
            return glm::vec3(node->transform[3]);
        }

        void static UpdateChain(const BoneNode*& node) {

        }

        // return chains total length
        float static BuildChains(BoneNode* start, BoneNode* endEffector, std::vector<BoneNode*>& chains, std::vector<glm::vec3>& positions) {
            float totalLength = 0.0f;
            for (auto& node = endEffector; ; node = node->parent) {
                if (!node->parent) {
                    AHO_CORE_ASSERT(false, "BuildChains");
                }
                positions.push_back(GetPosition(node));
                chains.push_back(node);
                glm::vec3 A = glm::vec3(GetPosition(node));
                glm::vec3 B = glm::vec3(GetPosition(node->parent));
                totalLength += glm::length(A - B);
                if (node->parent == start) {
                    break;
                }
            }
            std::reverse(positions.begin(), positions.end());
            std::reverse(chains.begin(), chains.end());
            return totalLength;
        }
	};
}