#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEPointCloudSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEPointCloudSystem)

		bool bInternalAdd = false;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicatePointCloudComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		//static Json::Value LightComponentToJson(FEEntity* Entity);
		//static void LightComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FEPointCloudSystem)

		
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPointCloudSystem();
	#define POINT_CLOUD_SYSTEM (*static_cast<FEPointCloudSystem*>(GetPointCloudSystem()))
#else
	#define POINT_CLOUD_SYSTEM FEPointCloudSystem::GetInstance()
#endif
}