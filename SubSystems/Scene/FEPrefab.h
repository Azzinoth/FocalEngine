#pragma once

#ifndef FEPREFAB_H
#define FEPREFAB_H
#include "../Renderer/FEGameModel.h"

namespace FocalEngine
{
#define PREFAB_SCENE_DESCRIPTION_TAG "PREFAB_DESCRIPTION"

	class FEPrefab : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
		friend FEEntity;

		FEScene* Scene = nullptr;
	public:
		FEPrefab(std::string Name = "", bool bSceneAllocation = true);
		~FEPrefab();

		FEAABB GetAABB();

		bool IsUsingMaterial(const FEUUID& MaterialID) const;
		bool IsUsingGameModel(const FEUUID& GameModelID) const;

		void SetScene(FEScene* Scene, bool bDeleteOldScene = true);
		FEScene* GetScene() const;
	};
}

#endif FEPREFAB_H