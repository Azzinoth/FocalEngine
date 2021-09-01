#pragma once

#ifndef FEENTITYINSTANCED_H
#define FEENTITYINSTANCED_H

#include "FEEntity.h"

namespace FocalEngine
{
	enum FEChangesType
	{
		CHANGE_NONE = 0,
		CHANGE_DELETED = 1,
		CHANGE_MODIFIED = 2,
		CHANGE_ADDED = 3
	};

	struct FEInstanceModification
	{
		FEChangesType type = CHANGE_NONE;
		int index = -1;
		glm::mat4 modification;

		FEInstanceModification();
		FEInstanceModification(FEChangesType type, int index, glm::mat4 modification) : type(type), index(index), modification(modification) {};
	};

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

	struct FEDrawElementsIndirectCommand
	{
		unsigned int count;
		unsigned int primCount;
		unsigned int firstIndex;
		unsigned int baseVertex;
		unsigned int baseInstance;
	};

	class FEEntityInstanced : public FEEntity
	{
		friend FERenderer;
		friend FETerrain;
		friend FEScene;
	public:
		FEEntityInstanced(FEGameModel* gameModel, std::string Name);
		~FEEntityInstanced();

		bool populate(FESpawnInfo spawnInfo);
		float cullInstances(glm::vec3 cameraPosition, float** frustum, bool freezeCulling = false);
		void render();
		void renderOnlyBillbords(glm::vec3 cameraPosition);

		FEAABB getAABB() final;
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

		void updateSelectModeAABBData();

		// used only in editor select mode
		std::vector<FEAABB> instancedAABB;
		bool isSelectMode();
		void setSelectMode(bool newValue);

		void deleteInstance(size_t instanceIndex);
		glm::mat4 getTransformedInstancedMatrix(size_t instanceIndex);
		void modifyInstance(size_t instanceIndex, glm::mat4 newMatrix);
		void addInstance(glm::mat4 instanceMatrix);

		bool tryToSnapInstance(size_t instanceIndex);

		int getSpawnModificationCount();
		std::vector<FEInstanceModification> getSpawnModifications();

		std::vector<glm::mat4> instancedMatrices;
		std::vector<glm::mat4> transformedInstancedMatrices;
		std::vector<float> instancedAABBSizes;
	private:
		bool selectionMode = false;
	
		std::vector<std::vector<glm::mat4>> instancedMatricesLOD;
		std::vector<glm::mat4> instancedMatricesBillboard;

		void addInstanceInternal(glm::mat4 instanceMatrix);
		void addInstances(glm::mat4* instanceMatrix, size_t count);

		void updateBuffers();
		void updateMatrices();
#ifdef USE_GPU_CULLING
		GLenum instancedBuffer = 0;
		GLenum* LODBuffers = nullptr;
		int* testLODCount = nullptr;

		GLuint sourceDataBuffer = 0;
		GLuint positionsBuffer = 0;
		GLuint AABBSizesBuffer = 0;
		GLuint LODInfoBuffer = 0;

		void initializeGPUCulling();

		FEDrawElementsIndirectCommand* indirectDrawsInfo;
		GLuint indirectDrawInfoBuffer = 0;
#else
		GLenum instancedBuffer = 0;
#endif // USE_GPU_CULLING
		FEAABB allInstancesAABB;
		FEGameModel* lastFrameGameModel = nullptr;

		FETerrain* terrainToSnap = nullptr;
		float(FETerrain::* getTerrainY)(glm::vec2);

		std::vector<FEInstanceModification> modifications;
	};
}

#endif FEENTITYINSTANCED_H