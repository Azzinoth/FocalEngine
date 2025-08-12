#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FELineSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FELineSystem)

		bool bInternalAdd = false;
		FEShader* InstancedLineShader = nullptr;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicateLineComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value LineComponentToJson(FEEntity* Entity);
		static void LineComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FELineSystem)

		void Render(FEEntity* Entity, FEEntity* Camera);
		bool RenderLineComponent(FETransformComponent& TransformComponent, FELineComponent& LineComponent, FEEntity* Camera);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetLineSystem();
	#define LINE_SYSTEM (*static_cast<FELineSystem*>(GetLineSystem()))
#else
	#define LINE_SYSTEM FELineSystem::GetInstance()
#endif
}