#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "FEFramebuffer.h"
#include "FEPrefab.h"

namespace FocalEngine
{
	class FEOctree;

	class FEEntity : public FEObject
	{
		friend FERenderer;
		friend FEOctree;
		FEEntity();
	public:
		FEEntity(FEPrefab* Prefab, std::string Name);
		~FEEntity();

		FEPrefab* prefab = nullptr;
		FETransformComponent transform;

		virtual void render();

		bool isVisible();
		void setVisibility(bool isVisible);

		virtual FEAABB getAABB();

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		bool isReceivingShadows();
		void setReceivingShadows(bool isReceivingShadows);

		bool isPostprocessApplied();
		void setIsPostprocessApplied(bool isPostprocessApplied);
	protected:
		bool visible = true;
		bool castShadows = true;
		bool receiveShadows = true;
		bool applyPostprocess = true;

		FEAABB entityAABB;
	};
}

#endif FEENTITY_H