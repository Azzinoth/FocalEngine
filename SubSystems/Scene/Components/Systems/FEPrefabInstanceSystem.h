#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEPrefabInstanceSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEPrefabInstanceSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void DuplicatePrefabInstanceComponent(FEEntity* EntityToWorkWith, FEEntity* NewEntity);
		
		static Json::Value PrefabInstanceComponentToJson(FEEntity* Entity);
		static void PrefabInstanceComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FEPrefabInstanceSystem)

		bool IsPartOfPrefabInstance(FEEntity* Entity);
		FEEntity* GetParentPrefabInstanceIfAny(FEEntity* Entity);

		bool IsPrefabInstanceUnmodified(FEEntity* Entity);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPrefabInstanceSystem();
	#define PREFAB_INSTANCE_SYSTEM (*static_cast<FEPrefabInstanceSystem*>(GetPrefabInstanceSystem()))
#else
	#define PREFAB_INSTANCE_SYSTEM FEPrefabInstanceSystem::GetInstance()
#endif
}