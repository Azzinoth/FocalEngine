#pragma once

#ifndef FETERRAIN_H
#define FETERRAIN_H

#include "../Renderer/FEEntityInstanced.h"
// FIX ME ! Temporary
#include "../SubSystems/Scene/FENewEntity.h"

namespace FocalEngine
{
#define FE_TERRAIN_MAX_LAYERS 8
#define FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER 9
#define FE_TERRAIN_LAYER_PER_TEXTURE 4
#define FE_TERRAIN_STANDARD_HIGHT_MAP_RESOLUTION 1024
#define FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION 512

	class FETerrain;

	class FETerrainLayer : public FEObject
	{
		friend FETerrain;
		friend FEResourceManager;

		FEMaterial* Material = nullptr;
		FETerrainLayer(std::string Name);
	public:
		void SetMaterial(FEMaterial* NewValue);
		FEMaterial* GetMaterial();
	};

	enum FE_TERRAIN_BRUSH_MODE
	{
		FE_TERRAIN_BRUSH_NONE = 0,
		FE_TERRAIN_BRUSH_SCULPT_DRAW = 1,
		FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED = 2,
		FE_TERRAIN_BRUSH_SCULPT_LEVEL = 3,
		FE_TERRAIN_BRUSH_SCULPT_SMOOTH = 4,
		FE_TERRAIN_BRUSH_LAYER_DRAW = 5
	};

	class FETerrain : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
	public:
		FETerrain(std::string Name);
		~FETerrain();

		FETransformComponent Transform;

		void Render();

		bool IsVisible();
		void SetVisibility(bool NewValue);

		FEAABB GetAABB();
		FEAABB GetPureAABB();

		bool IsCastingShadows();
		void SetCastingShadows(bool NewValue);

		bool IsReceivingShadows();
		void SetReceivingShadows(bool NewValue);

		FEShader* Shader = nullptr;

		FETexture* HeightMap = nullptr;
		std::vector<FETexture*> LayerMaps;
		FETexture* ProjectedMap = nullptr;

		bool IsWireframeMode();
		void SetWireframeMode(bool NewValue);
		
		float GetHightScale();
		void SetHightScale(float NewValue);

		float GetDisplacementScale();
		void SetDisplacementScale(float NewValue);

		glm::vec2 GetTileMult();
		void SetTileMult(glm::vec2 NewValue);

		float GetLODLevel();
		void SetLODLevel(float NewValue);

		float GetChunkPerSide();
		void SetChunkPerSide(float NewValue);

		float GetHeightAt(glm::vec2 XZWorldPosition);
		
		float GetXSize();
		float GetZSize();

		// ********************************** PointOnTerrain **********************************
		glm::dvec3 GetPointOnTerrain(glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection, float StartDistance = 0.0f, float EndDistance = 256.0f);
		// ********************************** PointOnTerrain END **********************************

		// **************************** TERRAIN EDITOR TOOLS ****************************
		float GetBrushSize();
		void SetBrushSize(float NewValue);

		float GetBrushIntensity();
		void SetBrushIntensity(float NewValue);

		bool IsBrushActive();
		void SetBrushActive(bool NewValue);

		FE_TERRAIN_BRUSH_MODE GetBrushMode();
		void SetBrushMode(FE_TERRAIN_BRUSH_MODE NewValue);

		size_t GetBrushLayerIndex();
		void SetBrushLayerIndex(size_t NewValue);
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		void SnapInstancedEntity(FEEntityInstanced* EntityToSnap);
		// FIX ME! Temporary
		void SnapInstancedEntity(FENewEntity* EntityToSnap);
		void UnSnapInstancedEntity(FEEntityInstanced* EntityToUnSnap);

		void ConnectInstancedEntityToLayer(FEEntityInstanced* Entity, int LayerIndex);
		void UnConnectInstancedEntityFromLayer(FEEntityInstanced* Entity);

		bool GetNextEmptyLayerSlot(size_t& NextEmptyLayerIndex);
		FETerrainLayer* GetLayerInSlot(size_t LayerIndex);

		int LayersUsed();

		float GetLayerIntensityAt(glm::vec2 XZWorldPosition, int LayerIndex);
	private:
		bool bWireframeMode = false;
		bool bVisible = true;
		bool bCastShadows = true;
		bool bReceiveShadows = true;

		float HightScale = 1.0f;
		float DisplacementScale = 0.2f;
		float ScaleFactor = 1.0f;
		glm::vec2 TileMult = glm::vec2(1.0);
		glm::vec2 HightMapShift = glm::vec2(0.0);
		float ChunkPerSide = 2.0f;

		float LODLevel = 64.0f;
		FEAABB AABB;
		FEAABB FinalAABB;
		float XSize = 0.0f;
		float ZSize = 0.0f;

		std::vector<float> HeightMapArray;

		// ********************************** PointOnTerrain **********************************
		glm::dvec3 BinarySearch(int Count, float Start, float Finish, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		bool IntersectionInRange(float Start, float Finish, glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		glm::dvec3 GetPointOnRay(glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection, float Distance);
		bool IsUnderGround(glm::dvec3 TestPoint);
		// ********************************** PointOnTerrain END **********************************

		// **************************** TERRAIN EDITOR TOOLS ****************************
		bool bBrushActive = false;
		FE_TERRAIN_BRUSH_MODE BrushMode = FE_TERRAIN_BRUSH_NONE;

		size_t BrushLayerIndex = 0;
		float BrushSize = 2.0f;
		float BrushIntensity = 0.01f;
		FEFramebuffer* BrushOutputFB = nullptr;
		FEShader* BrushOutputShader = nullptr;
		FEShader* LayersNormalizeShader = nullptr;
		FEFramebuffer* BrushVisualFB = nullptr;
		FEShader* BrushVisualShader = nullptr;
		FEMesh* PlaneMesh = nullptr;

		void UpdateBrush(glm::dvec3 MouseRayStart, glm::dvec3 MouseRayDirection);
		size_t WaitBeforeUpdateMs = 50;
		std::chrono::system_clock::time_point LastChangesTimeStamp;
		void UpdateCpuHeightInfo();
		void UpdateSnapedInstancedEntities();

		bool bCPUHeightInfoDirtyFlag = false;
		size_t FramesBeforeUpdate = 50;
		bool bBrushVisualFBCleared = false;
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		std::vector<FEEntityInstanced*> SnapedInstancedEntities;

		std::vector<FETerrainLayer*> Layers;
		std::vector<unsigned char*> LayerMapsRawData;
		bool UpdateLayerMapsRawData();
		FETerrainLayer* ActivateVacantLayerSlot(FEMaterial* Material);
		void DeleteLayerInSlot(size_t LayerIndex);
		
		GLuint GPULayersDataBuffer = 0;
		void LoadLayersDataToGPU();
		std::vector<float> GPULayersData;
		std::vector<float> OldGPULayersData;
	};
}

#endif FETERRAIN_H