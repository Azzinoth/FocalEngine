#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEVolumeSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEVolumeSystem)

		bool bInternalAdd = false;
		// FE_TO_DO: Implement volume material ?
		std::vector<FEShader*> VolumetricShaders;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicateVolumeComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value VolumeComponentToJson(FEEntity* Entity);
		static void VolumeComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FEVolumeSystem)

		void Render(FEEntity* Entity, FEEntity* Camera);
		bool RenderVolumeComponent(FETransformComponent& TransformComponent, FEVolumeComponent& VolumeComponent, FEEntity* Camera);

		std::vector<FEShader*> GetVolumetricShaders();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetVolumeSystem();
	#define VOLUME_SYSTEM (*static_cast<FEVolumeSystem*>(GetVolumeSystem()))
#else
	#define VOLUME_SYSTEM FEVolumeSystem::GetInstance()
#endif
}