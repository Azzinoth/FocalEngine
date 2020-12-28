#pragma once

#ifndef FETERRAIN_H
#define FETERRAIN_H

#include "../Renderer/FEEntityInstanced.h"

namespace FocalEngine
{
	class FETerrain : public FEAsset
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

		FEMaterial* layer0 = nullptr;
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

		bool isBrushInversed();
		void setBrushInversed(bool newBrushInversed);

		bool isBrushSculptMode();
		void setBrushSculptMode(bool newBrushSculptMode);

		bool isBrushLevelMode();
		void setBrushLevelMode(bool newBrushLevelMode);

		bool isBrushSmoothMode();
		void setBrushSmoothMode(bool newBrushSmoothMode);
		// **************************** TERRAIN EDITOR TOOLS END ****************************

		void snapInstancedEntity(FEEntityInstanced* entityToSnap);
		void unSnapInstancedEntity(FEEntityInstanced* entityToUnSnap);
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
		bool brushInversed = false;
		bool brushSculptMode = false;
		bool brushLevelMode = false;
		bool brushSmoothMode = false;

		float brushSize = 2.0f;
		float brushIntensity = 0.01f;
		FEFramebuffer* brushOutputFB = nullptr;
		FEShader* brushOutputShader = nullptr;
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
	};
}

#endif FETERRAIN_H