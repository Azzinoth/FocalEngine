#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEVirtualUISystem
	{
		friend struct FEVirtualUIComponent;
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEVirtualUISystem)

		FEMaterial* CanvasMaterial = nullptr;
		FEGameModel* DummyGameModel = nullptr;
		FEGameModelComponent DummyGameModelComponent;

		// FIXME: Not sure that using one game model for all virtual UIs is a good idea
		// An unordered map will be used to control visibility
		std::unordered_map<std::string, bool> VirtualUIIDToVisibilityMap;
		bool IsVirtualUIVisible(const std::string& VirtualUIID) const;
		void SetVirtualUIVisible(const std::string& VirtualUIID, bool bVisible);

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

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetVirtualUISystem();
	#define VIRTUAL_UI_SYSTEM (*static_cast<FEVirtualUISystem*>(GetVirtualUISystem()))
#else
	#define VIRTUAL_UI_SYSTEM FEVirtualUISystem::GetInstance()
#endif
}