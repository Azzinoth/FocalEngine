#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "../SubSystems/FETransformComponent.h"
#include "FEMesh.h"
#include "FEFramebuffer.h"
#include "FEGameModel.h"

namespace FocalEngine
{
	class FEEntity
	{
		friend FERenderer;
	public:
		FEEntity(FEGameModel* gameModel, std::string Name);
		~FEEntity();

		/*FEMesh* mesh = nullptr;
		FEMaterial* material = nullptr;*/
		FEGameModel* gameModel = nullptr;

		FETransformComponent transform;

		void render();

		bool isVisible();
		void setVisibility(bool isVisible);

		std::string getName();
		void setName(std::string newName);

		FEAABB getAABB();

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		bool isReceivingShadows();
		void setReceivingShadows(bool isReceivingShadows);
	private:
		std::string name;

		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;
	};
}

#endif FEENTITY_H