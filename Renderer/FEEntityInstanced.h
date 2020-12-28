#pragma once

#ifndef FEENTITYINSTANCED_H
#define FEENTITYINSTANCED_H

#include "FEEntity.h"

namespace FocalEngine
{
	struct FESpawnInfo
	{
		int seed = 0;
		int objectCount = 1;
		float radius = 1.0f;
		// 0.1f == 10 % from base scale
		float scaleDeviation = 0.1f;
		glm::vec3 rotationDeviation = glm::vec3(0.02f, 1.0f, 0.02f);

		float getPositionDeviation();
		float getScaleDeviation();
		int getRotaionDeviation(glm::vec3 axis);
	};

	class FEEntityInstanced : public FEEntity
	{
		friend FERenderer;
		friend FETerrain;
	public:
		FEEntityInstanced(FEGameModel* gameModel, std::string Name);
		~FEEntityInstanced();

		bool populate(FESpawnInfo spawInfo);
		float cullInstances(glm::vec3 cameraPosition, float** frustum, bool freezeCulling = false);
		void render();
		void renderOnlyBillbords(glm::vec3 cameraPosition);

		FEAABB getAABB() final;

		void addInstance(glm::mat4 instanceMatrix);
		void addInstances(glm::mat4* instanceMatrix, size_t count);
		void clear();

		int getInstanceCount();

		size_t instancedXYZCount = 0;
		float* instancedX;
		float* instancedY;
		float* instancedZ;

		int* LODCounts;
		size_t instanceCount = 0;

		int cullingType = FE_CULLING_LODS;
		FETerrain* getSnappedToTerrain();

		FESpawnInfo spawnInfo;
	private:
		std::vector<glm::mat4> instancedMatrices;
		std::vector<glm::mat4> transformedInstancedMatrices;
		std::vector<float> instancedAABBSizes;

		std::vector<std::vector<glm::mat4>> instancedMatricesLOD;
		std::vector<glm::mat4> instancedMatricesBillboard;

		void updateBuffers();
		void updateMatrices();

		GLenum instancedBuffer = 0;
		
		bool dirtyFlag = false;
		FEAABB allInstancesAABB;
		FEGameModel* lastFrameGameModel = nullptr;

		FETerrain* terrainToSnap = nullptr;
		float(FETerrain::* getTerrainY)(glm::vec2);
	};
}

#endif FEENTITYINSTANCED_H