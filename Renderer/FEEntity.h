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

		FEPrefab* Prefab = nullptr;
		FETransformComponent Transform;

		virtual void Render();

		bool IsVisible() const;
		void SetVisibility(bool NewValue);

		virtual FEAABB GetAABB();

		bool IsCastShadows() const;
		void SetCastShadows(bool NewValue);

		bool IsReceivingShadows() const;
		void SetReceivingShadows(bool NewValue);

		bool IsPostprocessApplied() const;
		void SetIsPostprocessApplied(bool NewValue);

		bool IsWireframeMode() const;
		void SetWireframeMode(bool NewValue);
	protected:
		bool bVisible = true;
		bool bCastShadows = true;
		bool bReceiveShadows = true;
		bool bApplyPostprocess = true;
		bool bWireframeMode = false;

		FEAABB EntityAABB;
	};
}

#endif FEENTITY_H