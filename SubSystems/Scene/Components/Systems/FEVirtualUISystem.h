#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEVirtualUISystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEVirtualUISystem)

		FEMaterial* CanvasMaterial = nullptr;
		FEGameModel* DummyGameModel = nullptr;
		FEGameModelComponent DummyGameModelComponent;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicateVirtualUIComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value VirtualUIComponentToJson(FEEntity* Entity);
		static void VirtualUIComponentFromJson(FEEntity* Entity, Json::Value Root);

		void Update();

		static void DummyRenderFunction(FEVirtualUI* VirtualUI);
	public:
		SINGLETON_PUBLIC_PART(FEVirtualUISystem)

		void RenderVirtualUIComponent(FEEntity* Entity);
		void RenderVirtualUIComponent(FEEntity* Entity, FEMaterial* ForceMaterial);
		FEEntity* GetParentEntity(FEVirtualUI* VirtualUI);
	};

#define VIRTUAL_UI_SYSTEM FEVirtualUISystem::GetInstance()
}