#pragma once
#include "../Renderer/FEGameModel.h"

namespace FocalEngine
{
	enum FE_INSTANCED_CHANGES_TYPE
	{
		FE_CHANGE_NONE = 0,
		FE_CHANGE_DELETED = 1,
		FE_CHANGE_MODIFIED = 2,
		FE_CHANGE_ADDED = 3
	};

	struct FEInstanceModification
	{
		FE_INSTANCED_CHANGES_TYPE Type = FE_CHANGE_NONE;
		int Index = -1;
		glm::mat4 Modification;

		FEInstanceModification();
		FEInstanceModification(const FE_INSTANCED_CHANGES_TYPE Type, const int Index, const glm::mat4 Modification) : Type(Type), Index(Index), Modification(Modification) {};
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

	struct FEInstancedElementData
	{
		std::vector<std::vector<glm::mat4>> InstancedMatricesLOD;
		std::vector<glm::vec3> InstancePositions;
		int* LODCounts = nullptr;

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
		FEGameModel* LastFrameGameModel = nullptr;

		std::vector<glm::mat4> InstancedMatrices;
		std::vector<glm::mat4> TransformedInstancedMatrices;
		std::vector<float> InstancedAABBSizes;

		std::string EntityIDWithGameModelComponent = "";
	};

	struct FEInstancedComponent
	{
		friend class FETerrainSystem;
		friend class FEInstancedSystem;
		friend class FERenderer;
		friend class FEScene;

		FEInstancedComponent();
		FEInstancedComponent(const FEInstancedComponent& Other) = default;
		~FEInstancedComponent();

		void Clear();

		size_t GetInstanceCount();

		FEEntity* GetSnappedToTerrain();
		int GetTerrainLayer();

		// Later there should be a component for this
		float GetMinimalLayerIntensityToSpawn();
		void SetMinimalLayerIntensityToSpawn(float NewValue);

		// Used only in editor select mode
		// editor functionality, should not be here.
		std::vector<FEAABB> IndividualInstancedAABB;

		glm::mat4 GetTransformedInstancedMatrix(size_t InstanceIndex);

		size_t GetSpawnModificationCount();
		std::vector<FEInstanceModification> GetSpawnModifications();
		FESpawnInfo SpawnInfo;

		std::vector<FEInstancedElementData*> InstancedElementsData;
	private:
		bool bDirtyFlag = false;
		size_t InstanceCount = 0;

		// Editor functionality, should not be here
		bool bSelectionMode = false;

		// Data for postponed spawning
		std::string PostponedTerrainToSnapID = "";
		int PostponedTerrainLayer = -1;
		Json::Value PostponedModificationsData;

		FEEntity* TerrainToSnap = nullptr;

		int TerrainLayer = -1;
		float MinLayerIntensityToSpawn = 0.4f;

		void SnapToTerrain(FEEntity* Terrain);
		void UnSnapFromTerrain();

		void ConnectToTerrainLayer(FEEntity* Terrain, int LayerIndex);
		void UnConnectFromTerrainLayer();

		FE_CULING_TYPE CullingType = FE_CULLING_LODS;
		// ******************* RENDERING *******************
		//std::vector<std::vector<glm::mat4>> InstancedMatricesLOD;
		//std::vector<glm::vec3> InstancePositions;
		//int* LODCounts = nullptr;

		//// GPU Culling
		//GLenum InstancedBuffer = 0;
		//GLenum* LODBuffers = nullptr;

		//GLuint SourceDataBuffer = 0;
		//GLuint PositionsBuffer = 0;
		//GLuint AABBSizesBuffer = 0;
		//GLuint LODInfoBuffer = 0;

		//FEDrawElementsIndirectCommand* IndirectDrawsInfo;
		//GLuint IndirectDrawInfoBuffer = 0;

		//FEAABB AllInstancesAABB;
		//FEGameModel* LastFrameGameModel = nullptr;

		//std::vector<glm::mat4> InstancedMatrices;
		//std::vector<glm::mat4> TransformedInstancedMatrices;
		//std::vector<float> InstancedAABBSizes;
		// ******************* RENDERING END *******************
		

		std::vector<FEInstanceModification> Modifications;
	};
}