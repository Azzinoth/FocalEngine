#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCALENGINE_API FESkyDomeSystem
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

#define SKY_DOME_SYSTEM FESkyDomeSystem::getInstance()
}