#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FETerrainSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FETerrainSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void DuplicateTerrainComponent(FEEntity* EntityWithTerrainComponent, FEEntity* NewEntity);

		// ********************************** PointOnTerrain **********************************
		glm::dvec3 BinarySearch(FEEntity* TerrainEntity, int Count, float Start, float Finish, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		bool IntersectionInRange(FEEntity* TerrainEntity, float Start, float Finish, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		glm::dvec3 GetPointOnRay(FEEntity* TerrainEntity, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection, float Distance);
		bool IsUnderGround(FEEntity* TerrainEntity, glm::dvec3 TestPoint);
		// ********************************** PointOnTerrain END **********************************

		// **************************** TERRAIN EDITOR TOOLS ****************************
		FEEntity* TerrainEntityWithBrushModeOn = nullptr;

		bool bBrushActive = false;
		FE_TERRAIN_BRUSH_MODE BrushMode = FE_TERRAIN_BRUSH_NONE;

		size_t BrushLayerIndex = 0;
		float BrushSize = 2.0f;
		float BrushIntensity = 0.01f;
		
		FEShader* BrushOutputShader = nullptr;
		FEShader* LayersNormalizeShader = nullptr;
		FEShader* BrushVisualShader = nullptr;
		FEMesh* PlaneMesh = nullptr;

		void UpdateBrush(glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		size_t WaitBeforeUpdateMs = 50;
		std::chrono::system_clock::time_point LastChangesTimeStamp;
		void UpdateCPUHeightInfo(FEEntity* TerrainEntity);
		void UpdateSnapedInstancedEntities(FEEntity* TerrainEntity);

		bool bCPUHeightInfoDirtyFlag = false;
		size_t FramesBeforeUpdate = 50;
		bool bBrushVisualFBCleared = false;
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		void InitTerrainEditTools(FEEntity* TerrainEntity);
		bool UpdateLayerMapsRawData(FEEntity* TerrainEntity);
	public:
		SINGLETON_PUBLIC_PART(FETerrainSystem)

		FEAABB GetAABB(FEEntity* TerrainEntity);
		FEAABB GetPureAABB(FEEntity* TerrainEntity);

		float GetHeightAt(FEEntity* TerrainEntity, glm::vec2 XZWorldPosition);
		float GetLayerIntensityAt(FEEntity* TerrainEntity, glm::vec2 XZWorldPosition, int LayerIndex);
		void ActivateVacantLayerSlot(FEEntity* TerrainEntity, FEMaterial* Material);
		void FillTerrainLayerMask(FEEntity* TerrainEntity, const size_t LayerIndex);
		void FillTerrainLayerMaskWithRawData(FEEntity* TerrainEntity, const unsigned char* RawData, const size_t LayerIndex);
		void ClearTerrainLayerMask(FEEntity* TerrainEntity, size_t LayerIndex);
		void DeleteTerrainLayerMask(FEEntity* TerrainEntity, size_t LayerIndex);
		void LoadTerrainLayerMask(FEEntity* TerrainEntity, std::string FileName, const size_t LayerIndex);
		void SaveTerrainLayerMask(FEEntity* TerrainEntity, std::string FileName, const size_t LayerIndex);
		// ********************************** PointOnTerrain **********************************
		glm::dvec3 GetPointOnTerrain(FEEntity* TerrainEntity, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection, float StartDistance = 0.0f, float EndDistance = 256.0f);
		// ********************************** PointOnTerrain END **********************************

		void SnapInstancedEntity(FEEntity* TerrainEntity, FEEntity* EntityToSnap);
		void UnSnapInstancedEntity(FEEntity* TerrainEntity, FEEntity* EntityToUnSnap);

		void ConnectInstancedEntityToLayer(FEEntity* TerrainEntity, FEEntity* EntityWithInstancedComponent, int LayerIndex);
		void UnConnectInstancedEntityFromLayer(FEEntity* Entity);

		// **************************** TERRAIN EDITOR TOOLS ****************************
		float GetBrushSize();
		void SetBrushSize(float NewValue);

		float GetBrushIntensity();
		void SetBrushIntensity(float NewValue);

		bool IsBrushActive();
		void SetBrushActive(bool NewValue);

		FE_TERRAIN_BRUSH_MODE GetBrushMode();
		void SetBrushMode(FEEntity* TerrainEntity, FE_TERRAIN_BRUSH_MODE NewValue);

		size_t GetBrushLayerIndex();
		void SetBrushLayerIndex(size_t NewValue);
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		void LoadHeightMap(std::string FileName, FEEntity* TerrainEntity);
	};

#define TERRAIN_SYSTEM FETerrainSystem::getInstance()
}