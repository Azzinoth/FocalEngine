#pragma once

#ifndef FEENTITYINSTANCED_H
#define FEENTITYINSTANCED_H

#include "FEEntity.h"

namespace FocalEngine
{
	enum FE_INSTANCED_ENTITY_CHANGES_TYPE
	{
		FE_CHANGE_NONE = 0,
		FE_CHANGE_DELETED = 1,
		FE_CHANGE_MODIFIED = 2,
		FE_CHANGE_ADDED = 3
	};

	struct FEInstanceModification
	{
		FE_INSTANCED_ENTITY_CHANGES_TYPE Type = FE_CHANGE_NONE;
		int Index = -1;
		glm::mat4 Modification;

		FEInstanceModification();
		FEInstanceModification(const FE_INSTANCED_ENTITY_CHANGES_TYPE Type, const int Index, const glm::mat4 Modification) : Type(Type), Index(Index), Modification(Modification) {};
	};

	struct FESpawnInfo
	{
		int Seed = 0;
		int ObjectCount = 1;
		float Radius = 1.0f;

		float GetMinScale();
		void SetMinScale(float NewValue);

		float GetMaxScale();
		void SetMaxScale(float NewValue);

		glm::vec3 RotationDeviation = glm::vec3(0.02f, 1.0f, 0.02f);

		float GetPositionDeviation();
		float GetScaleDeviation();
		int GetRotaionDeviation(glm::vec3 Axis);

	private:
		float MinScale = 1.0f;
		float MaxScale = 1.5f;
	};

	struct FEDrawElementsIndirectCommand
	{
		unsigned int Count;
		unsigned int PrimCount;
		unsigned int FirstIndex;
		unsigned int BaseVertex;
		unsigned int BaseInstance;
	};

	class FEEntityInstanced;
	class FEGameModelInstancedRenderer
	{
		friend FERenderer;
		friend FEEntityInstanced;

		std::vector<std::vector<glm::mat4>> InstancedMatricesLOD;

		std::vector<glm::vec3> InstancePositions;
		int* LODCounts;

		// GPU Culling
		GLenum InstancedBuffer = 0;
		GLenum* LODBuffers = nullptr;

		GLuint SourceDataBuffer = 0;
		GLuint PositionsBuffer = 0;
		GLuint AABBSizesBuffer = 0;
		GLuint LODInfoBuffer = 0;

		FEDrawElementsIndirectCommand* IndirectDrawsInfo;
		GLuint IndirectDrawInfoBuffer = 0;

		FEAABB AllInstancesAABB;
		FEPrefab* LastFramePrefab = nullptr;

		std::vector<glm::mat4> InstancedMatrices;
		std::vector<glm::mat4> TransformedInstancedMatrices;
		std::vector<float> InstancedAABBSizes;
	};

	class FEEntityInstanced : public FEEntity
	{
		friend FERenderer;
		friend FETerrain;
		friend FEScene;
	public:
		FEEntityInstanced(FEPrefab* Prefab, std::string Name);
		~FEEntityInstanced();

		bool Populate(FESpawnInfo SpawnInfo);
		void Render(int SubGameModel);
		void RenderOnlyBillbords(glm::vec3 CameraPosition);

		FEAABB GetAABB() final;
		void Clear();

		size_t InstanceCount = 0;

		int GetInstanceCount();

		FETerrain* GetSnappedToTerrain();
		int GetTerrainLayer();

		float GetMinimalLayerIntensity();
		void SetMinimalLayerIntensity(float NewValue);

		FESpawnInfo SpawnInfo;

		void UpdateSelectModeAABBData();

		// used only in editor select mode
		std::vector<FEAABB> InstancedAABB;
		bool IsSelectMode();
		void SetSelectMode(bool NewValue);

		void DeleteInstance(size_t InstanceIndex);
		glm::mat4 GetTransformedInstancedMatrix(size_t InstanceIndex);
		void ModifyInstance(size_t InstanceIndex, glm::mat4 NewMatrix);
		void AddInstance(glm::mat4 InstanceMatrix);

		bool TryToSnapInstance(size_t InstanceIndex);

		int GetSpawnModificationCount();
		std::vector<FEInstanceModification> GetSpawnModifications();

		std::vector<FEInstanceModification> Modifications;
	private:
		std::vector<FEGameModelInstancedRenderer*> Renderers;

		bool bSelectionMode = false;

		int CullingType = FE_CULLING_LODS;

		FETerrain* TerrainToSnap = nullptr;
		int TerrainLayer = -1;
		float MinLayerIntensity = 0.4f;
		float(FETerrain::* GetTerrainY)(glm::vec2);
		float(FETerrain::* GetTerrainLayerIntensity)(glm::vec2, int);

		void AddInstanceInternal(glm::mat4 InstanceMatrix);
		void AddInstances(const glm::mat4* InstanceMatrix, size_t Count);

		void ClearRenderers();
		void InitRender(int Index);

		void UpdateBuffers();
		void UpdateMatrices();

		void InitializeGPUCulling();

		void SnapToTerrain(FETerrain* Terrain, float(FETerrain::* GetTerrainY)(glm::vec2));
		void UnSnapFromTerrain();

		void ConnectToTerrainLayer(FETerrain* Terrain, int LayerIndex, float(FETerrain::* GetTerrainLayerIntensity)(glm::vec2, int));
		void UnConnectFromTerrainLayer();

		void CheckDirtyFlag(int SubGameModel);
	};
}

#endif FEENTITYINSTANCED_H