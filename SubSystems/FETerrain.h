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
	private:
		std::string name;
		int nameHash = 0;

		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;

		FEShader* shader = nullptr;
	};
}

#endif FETERRAIN_H