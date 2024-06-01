#pragma once

#ifndef FEPREFAB_H
#define FEPREFAB_H

#include "../SubSystems/Scene/FETransformComponent.h"
#include "FEGameModel.h"

namespace FocalEngine
{
	struct FEPrefabComponent
	{
		// Later it should be FEObject because we could use not only FEGameModel but something like FEAnimation or other classes as components.
		FEGameModel* GameModel = nullptr;
		FETransformComponent Transform;
	};

	class FEPrefab : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
		friend FEEntity;
		friend FEEntityInstanced;

		std::vector<FEPrefabComponent*> Components;

		FEAABB AABB;
		void UpdateAABB();
		
		FEPrefab();
	public:
		FEPrefab(FEGameModel* GameModel, std::string Name = "");
		~FEPrefab();

		int ComponentsCount() const;
		void AddComponent(FEGameModel* GameModel, FETransformComponent Transform = FETransformComponent());
		FEPrefabComponent* GetComponent(int Index) const;
		void RemoveComponent(const FEGameModel* GameModel);
		void RemoveComponent(int Index);

		FEAABB GetAABB();

		bool UsesMaterial(std::string MaterialID) const;
		bool UsesGameModel(std::string GameModelID) const;
	};
}

#endif FEPREFAB_H