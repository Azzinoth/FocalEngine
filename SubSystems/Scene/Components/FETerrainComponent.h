#pragma once

#include "../Core/FEGeometricTools.h"
#include "../Renderer/FEMaterial.h"

namespace FocalEngine
{
#define FE_TERRAIN_MAX_LAYERS 8
#define FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER 9
#define FE_TERRAIN_LAYER_PER_TEXTURE 4
#define FE_TERRAIN_STANDARD_HIGHT_MAP_RESOLUTION 1024
#define FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION 512

	class FETerrainLayer
	{
		friend class FETerrainComponent;
		friend class FEResourceManager;

		std::string Name;
		FEMaterial* Material = nullptr;
		FETerrainLayer(std::string Name);
	public:
		void SetMaterial(FEMaterial* NewValue);
		FEMaterial* GetMaterial();

		std::string GetName() { return Name; }
		void SetName(std::string NewName) { Name = NewName; }
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

	// Idealy it not should be here
	class FEEntity;

	class FETerrainComponent
	{
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FETerrainSystem;
	public:
		FETerrainComponent();
		FETerrainComponent(FETerrainComponent& Other) = default;
		~FETerrainComponent();

		bool IsVisible();
		void SetVisibility(bool NewValue);

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

		float GetXSize();
		float GetZSize();

		bool GetNextEmptyLayerSlot(size_t& NextEmptyLayerIndex);
		FETerrainLayer* GetLayerInSlot(size_t LayerIndex);

		int LayersUsed();
	private:
		bool bWireframeMode = false;
		bool bVisible = true;
		bool bCastShadows = true;
		bool bReceiveShadows = true;
		bool bDirtyFlag = false;

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

		// **************************** TERRAIN EDITOR TOOLS ****************************
		FEFramebuffer* BrushOutputFB = nullptr;
		FEFramebuffer* BrushVisualFB = nullptr;
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		std::vector<FEEntity*> SnapedInstancedEntities;

		std::vector<FETerrainLayer*> Layers;
		std::vector<unsigned char*> LayerMapsRawData;
		FETerrainLayer* ActivateVacantLayerSlot(FEMaterial* Material);
		void DeleteLayerInSlot(size_t LayerIndex);
		
		GLuint GPULayersDataBuffer = 0;
		void LoadLayersDataToGPU();
		std::vector<float> GPULayersData;
		std::vector<float> OldGPULayersData;
	};
}