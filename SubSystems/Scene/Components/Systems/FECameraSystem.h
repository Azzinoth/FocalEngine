#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FECameraSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FECameraSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();
		
		static void DuplicateCameraComponent(FEEntity* EntityWithCameraComponent, FEEntity* NewEntity);

		void Update(const double DeltaTime);

		std::unordered_map<std::string, std::vector<FEEntity*>> ViewPortToCameraEntities;
		static void OnViewportResize(std::string ViewportID);

		static Json::Value CameraComponentToJson(FEEntity* Entity);
		static void CameraComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FECameraSystem)

		void SetMainCamera(FEEntity* CameraEntity);
		FEEntity* GetMainCamera(FEScene* Scene) const;
		FEViewport* GetMainCameraViewport(FEScene* Scene) const;

		bool SetCameraViewport(FEEntity* CameraEntity, std::string ViewportID);

		void IndividualUpdate(FEEntity* CameraEntity, const double DeltaTime);

		bool SetCameraRenderingPipeline(FEEntity* CameraEntity, FERenderingPipeline NewPipeline);

		bool SetCameraRenderScale(FEEntity* CameraEntity, float NewValue);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetCameraSystem();
	#define CAMERA_SYSTEM (*static_cast<FECameraSystem*>(GetCameraSystem()))
#else
	#define CAMERA_SYSTEM FECameraSystem::GetInstance()
#endif
}