#pragma once

#ifndef FETERRAIN_H
#define FETERRAIN_H

#include "../Renderer/FEEntityInstanced.h"

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

		FEMaterial* material = nullptr;
		FETerrainLayer(std::string name);
	public:
		void setMaterial(FEMaterial* newValue);
		FEMaterial* getMaterial();
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

		FETransformComponent transform;

		void render();

		bool isVisible();
		void setVisibility(bool isVisible);

		FEAABB getAABB();
		FEAABB getPureAABB();

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		bool isReceivingShadows();
		void setReceivingShadows(bool isReceivingShadows);

		FEShader* shader = nullptr;

		FETexture* heightMap = nullptr;
		std::vector<FETexture*> layerMaps;
		FETexture* projectedMap = nullptr;

		bool isWireframeMode();
		void setWireframeMode(bool isActive);
		
		float getHightScale();
		void setHightScale(float newHightScale);

		float getDisplacementScale();
		void setDisplacementScale(float newDisplacementScale);

		glm::vec2 getTileMult();
		void setTileMult(glm::vec2 newTileMult);

		float getLODlevel();
		void setLODlevel(float newLODlevel);

		float getChunkPerSide();
		void setChunkPerSide(float newChunkPerSide);

		float getHeightAt(glm::vec2 XZWorldPosition);
		
		float getXSize();
		float getZSize();

		// ********************************** PointOnTerrain **********************************
		glm::dvec3 getPointOnTerrain(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection, float startDistance = 0.0f, float endDistance = 256.0f);
		// ********************************** PointOnTerrain END **********************************

		// **************************** TERRAIN EDITOR TOOLS ****************************
		float getBrushSize();
		void setBrushSize(float newBrushSize);

		float getBrushIntensity();
		void setBrushIntensity(float newBrushIntensity);

		bool isBrushActive();
		void setBrushActive(bool newBrushActive);

		FE_TERRAIN_BRUSH_MODE getBrushMode();
		void setBrushMode(FE_TERRAIN_BRUSH_MODE newBrushMode);

		size_t getBrushLayerIndex();
		void setBrushLayerIndex(size_t newBrushLayerIndex);
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		void snapInstancedEntity(FEEntityInstanced* entityToSnap);
		void unSnapInstancedEntity(FEEntityInstanced* entityToUnSnap);

		bool getNextEmptyLayerSlot(size_t& nextEmptyLayerIndex);
		FETerrainLayer* getLayerInSlot(size_t layerIndex);

		int layersUsed();
	private:
		bool wireframeMode = false;
		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;

		float hightScale = 1.0f;
		float displacementScale = 0.2f;
		float scaleFactor = 1.0f;
		glm::vec2 tileMult = glm::vec2(1.0);
		glm::vec2 hightMapShift = glm::vec2(0.0);
		float chunkPerSide = 2.0f;

		float LODlevel = 64.0f;
		FEAABB AABB;
		FEAABB finalAABB;
		float xSize = 0.0f;
		float zSize = 0.0f;

		std::vector<float> heightMapArray;

		// ********************************** PointOnTerrain **********************************
		glm::dvec3 binarySearch(int count, float start, float finish, glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection);
		bool intersectionInRange(float start, float finish, glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection);
		glm::dvec3 getPointOnRay(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection, float distance);
		bool isUnderGround(glm::dvec3 testPoint);
		// ********************************** PointOnTerrain END **********************************

		// **************************** TERRAIN EDITOR TOOLS ****************************
		bool brushActive = false;
		FE_TERRAIN_BRUSH_MODE brushMode = FE_TERRAIN_BRUSH_NONE;

		size_t brushLayerIndex = 0;
		float brushSize = 2.0f;
		float brushIntensity = 0.01f;
		FEFramebuffer* brushOutputFB = nullptr;
		FEShader* brushOutputShader = nullptr;
		FEShader* layersNormalizeShader = nullptr;
		FEFramebuffer* brushVisualFB = nullptr;
		FEShader* brushVisualShader = nullptr;
		FEMesh* planeMesh = nullptr;

		void updateBrush(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection);
		size_t waitBeforeUpdateMS = 50;
		std::chrono::system_clock::time_point lastChangesTimeStamp;
		void updateCPUHeightInfo();
		void updateSnapedInstancedEntities();

		bool CPUHeightInfoDirtyFlag = false;
		char* pixelBuffer = nullptr;
		size_t pixelBufferCount = 0;
		size_t framesBeforeUpdate = 50;
		bool brushVisualFBCleared = false;
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		std::vector<FEEntityInstanced*> snapedInstancedEntities;

		std::vector<FETerrainLayer*> layers;
		FETerrainLayer* activateVacantLayerSlot(FEMaterial* material);
		void deleteLayerInSlot(size_t layerIndex);
		
		GLuint GPULayersDataBuffer = 0;
		void loadLayersDataToGPU();
		std::vector<float> GPULayersData;
		std::vector<float> OldGPULayersData;
	};
}

#endif FETERRAIN_H