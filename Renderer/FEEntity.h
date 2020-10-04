#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "../SubSystems/FETransformComponent.h"
#include "FEMesh.h"
#include "FEFramebuffer.h"
#include "FEGameModel.h"

namespace FocalEngine
{
	class FEEntity : public FEAsset
	{
		friend FERenderer;
	public:
		FEEntity(FEGameModel* gameModel, std::string Name);
		~FEEntity();

		FEGameModel* gameModel = nullptr;
		FETransformComponent transform;

		void render();

		bool isVisible();
		void setVisibility(bool isVisible);

		std::string getName();
		void setName(std::string newName);

		int getNameHash();

		FEAABB getAABB();
		FEAABB getPureAABB();

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		bool isReceivingShadows();
		void setReceivingShadows(bool isReceivingShadows);
	private:
		std::string name;
		int nameHash = 0;

		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;

		FEAABB entityAABB;
	};
}

#endif FEENTITY_H