#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FESkyDomeSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FESkyDomeSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();
		void OnSceneClear();

		static FEGameModel* SkyDomeGameModel;
		bool bEnabled = true;

		static Json::Value SkyDomeComponentToJson(FEEntity* Entity);
		static void SkyDomeComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FESkyDomeSystem)

		bool IsEnabled();
		void SetEnabled(bool NewValue);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetSkyDomeSystem();
	#define SKY_DOME_SYSTEM (*static_cast<FESkyDomeSystem*>(GetSkyDomeSystem()))
#else
	#define SKY_DOME_SYSTEM FESkyDomeSystem::GetInstance()
#endif
}