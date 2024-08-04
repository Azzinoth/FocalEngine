#pragma once
#include "../Renderer/FEPrefab.h"

namespace FocalEngine
{
	struct FEPrefabInstanceComponent
	{
		friend class FEPrefabInstanceSystem;
		
		FEPrefabInstanceComponent() {};
		FEPrefabInstanceComponent(FEPrefab* PrefabToSet);
		FEPrefabInstanceComponent(const FEPrefabInstanceComponent& Other);
		void operator=(const FEPrefabInstanceComponent& Other);

		FEPrefab* GetPrefab();
	private:
		FEPrefab* Prefab = nullptr;
	};
}