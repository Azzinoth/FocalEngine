#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FEPrefabInstanceSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEPrefabInstanceSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void DuplicatePrefabInstanceComponent(FEEntity* EntityToWorkWith, FEEntity* NewEntity);
		bool IsPrefabInstanceValid(FEEntity* Entity);
	public:
		SINGLETON_PUBLIC_PART(FEPrefabInstanceSystem)


	};

#define PREFAB_INSTANCE_SYSTEM FEPrefabInstanceSystem::getInstance()
}