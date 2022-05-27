#pragma once

#ifndef FEPREFAB_H
#define FEPREFAB_H

#include "../SubSystems/FETransformComponent.h"
#include "FEGameModel.h"

namespace FocalEngine
{
	struct FEPrefabComponent
	{
		// Later it should be FEObject because we could use not only FEGameModel but something like FEAnimation or other classes as components.
		FEGameModel* gameModel = nullptr;
		FETransformComponent transform;
	};

	class FEPrefab : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
		friend FEEntity;
		friend FEEntityInstanced;

		std::vector<FEPrefabComponent*> components;

		FEAABB AABB;
		void updateAABB();
		
		FEPrefab();
	public:
		FEPrefab(FEGameModel* gameModel, std::string Name = "");
		~FEPrefab();

		int componentsCount();
		void addComponent(FEGameModel* gameModel, FETransformComponent transform = FETransformComponent());
		FEPrefabComponent* getComponent(int index);
		void removeComponent(FEGameModel* gameModel);
		void removeComponent(int index);

		FEAABB getAABB();

		bool usesMaterial(std::string materialID);
		bool usesGameModel(std::string gameModelID);
	};
}

#endif FEPREFAB_H