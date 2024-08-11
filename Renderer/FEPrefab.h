#pragma once

#ifndef FEPREFAB_H
#define FEPREFAB_H
#include "FEGameModel.h"

namespace FocalEngine
{
	class FEPrefab : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
		friend FEEntity;
		friend FEEntityInstanced;

		FEScene* Scene = nullptr;
	public:
		FEPrefab(std::string Name = "");
		~FEPrefab();

		FEAABB GetAABB();

		bool IsUsingMaterial(std::string MaterialID) const;
		bool IsUsingGameModel(std::string GameModelID) const;

		void SetScene(FEScene* Scene, bool DeleteOldScene = true);
		FEScene* GetScene() const;
	};
}

#endif FEPREFAB_H