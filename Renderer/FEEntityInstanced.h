#pragma once

#ifndef FEENTITYINSTANCED_H
#define FEENTITYINSTANCED_H

#include "FEEntity.h"

namespace FocalEngine
{
	class FEEntityInstanced : public FEEntity
	{
		friend FERenderer;
	public:
		FEEntityInstanced(FEGameModel* gameModel, std::string Name);
		~FEEntityInstanced();
		float render(glm::vec3 cameraPosition);

		FEAABB getAABB() final;

		void addInstance(glm::mat4 instanceMatrix);
		void clear();

		int getInstanceCount();

		std::vector<glm::vec3> instancedPositions;

		FEMesh* testLOD0 = nullptr;
		FEMesh* testLOD1 = nullptr;
		float cullDistance = -1;
	private:
		std::vector<glm::mat4> instancedMatrices;

		std::vector<glm::mat4> testInstancedMatricesLOD0;
		std::vector<glm::mat4> testInstancedMatricesLOD1;
		
		GLenum instancedBuffer = 0;
		int instanceCount = 0;
		bool dirtyFlag = false;
		FEAABB allInstancesAABB;
		FEGameModel* lastFrameGameModel = nullptr;
	};
}

#endif FEENTITYINSTANCED_H