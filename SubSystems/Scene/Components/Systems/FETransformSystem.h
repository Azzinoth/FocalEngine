#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FETransformSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FETransformSystem)

		//bool bInternalAdd = false;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void Update();
		void UpdateInternal(FENaiveSceneGraphNode* SubTreeRoot);
	public:
		SINGLETON_PUBLIC_PART(FETransformSystem)
	};

#define TRANSFORM_SYSTEM FETransformSystem::getInstance()
}