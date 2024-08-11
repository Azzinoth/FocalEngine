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

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void Update();
		void UpdateInternal(FENaiveSceneGraphNode* SubTreeRoot);

		static Json::Value TransfromComponentToJson(FEEntity* Entity);
		static void TransfromComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FETransformSystem)

		void DuplicateTransformComponent(FEEntity* SourceEntity, FEEntity* NewEntity);
	};

#define TRANSFORM_SYSTEM FETransformSystem::getInstance()
}