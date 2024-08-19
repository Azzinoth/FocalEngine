#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FENativeScriptSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FENativeScriptSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void Update();

		static Json::Value NativeScriptComponentToJson(FEEntity* Entity);
		static void NativeScriptComponentFromJson(FEEntity* Entity, Json::Value Root);
		static void DuplicateNativeScriptComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);
	public:
		SINGLETON_PUBLIC_PART(FENativeScriptSystem)

		void Init(FEEntity* Entity);
	};

#define NATIVE_SCRIPT_SYSTEM FENativeScriptSystem::getInstance()
}