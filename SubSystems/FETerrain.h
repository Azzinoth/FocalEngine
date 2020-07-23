#pragma once

#ifndef FETERRAIN_H
#define FETERRAIN_H

#include "../Renderer/FEEntity.h"

namespace FocalEngine
{
	class FETerrain
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

		std::string getName();
		void setName(std::string newName);

		int getNameHash();

		/*FEAABB getAABB();
		FEAABB getPureAABB();*/

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		bool isReceivingShadows();
		void setReceivingShadows(bool isReceivingShadows);

		FEShader* shader = nullptr;

		FETexture* heightMap = nullptr;

		FEMaterial* layer0 = nullptr;
		/*FETexture* albedoMap = nullptr;
		FETexture* normalMap = nullptr;
		FETexture* AOMap = nullptr;
		FETexture* roughtnessMap = nullptr;
		FETexture* metalnessMap = nullptr;
		FETexture* displacementMap = nullptr;*/

		bool isWireframeMode();
		void setWireframeMode(bool isActive);
		
		float getHightScale();
		void setHightScale(float newHightScale);

		glm::vec2 getTileMult();
		void setTileMult(glm::vec2 newTileMult);

		float getLODlevel();
		void setLODlevel(float newLODlevel);

		float getChunkPerSide();
		void setChunkPerSide(float newChunkPerSide);
	private:
		std::string name;
		int nameHash = 0;

		bool wireframeMode = false;
		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;

		float hightScale = 1.0;
		float scaleFactor = 1.0;
		glm::vec2 tileMult = glm::vec2(1.0);
		glm::vec2 hightMapShift = glm::vec2(0.0);
		float chunkPerSide = 4.0;

		float LODlevel = 64.0;
	};
}

#endif FETERRAIN_H