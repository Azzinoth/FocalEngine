#pragma once
#include "../../Scene/FEScene.h"

namespace FocalEngine
{
	class FESkyDomeSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FESkyDomeSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity);
		void RegisterOnComponentCallbacks();
		void OnSceneClear();

		static FEGameModel* SkyDomeGameModel;
		static FEGameModelComponent* SkyDomeGameModelComponent;
	public:
		SINGLETON_PUBLIC_PART(FESkyDomeSystem)

		bool IsEnabled();
		void SetEnabled(bool NewValue);

		void AddToEntity(FEEntity* EntityToAdd);
	};

#define SKY_DOME_SYSTEM FESkyDomeSystem::getInstance()
}