#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FECameraSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FECameraSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();
		
		void DuplicateCameraComponent(FEEntity* EntityWithCameraComponent, FEEntity* NewEntity);

		void Update(const double DeltaTime);

		// FIX ME! This is temporary
		void SetCursorToCenter(FECameraComponent& Camera);
		glm::dvec3 PolarToCartesian(double PolarAngle, double AzimutAngle, const double R);

		std::unordered_map<std::string, std::vector<FEEntity*>> ViewPortToCameraEntities;
		static void OnViewportResize(std::string ViewportID);

		static Json::Value CameraComponentToJson(FEEntity* Entity);
		static void CameraComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FECameraSystem)

		void SetMainCamera(FEEntity* CameraEntity);
		FEEntity* GetMainCameraEntity(FEScene* Scene) const;
		FEViewport* GetMainCameraViewport(FEScene* Scene) const;

		bool SetCameraViewport(FEEntity* CameraEntity, std::string ViewportID);

		void SetIsIndividualInputActive(FEEntity* CameraEntity, const bool Active);
		void IndividualUpdate(FEEntity* CameraEntity, const double DeltaTime);
	};

#define CAMERA_SYSTEM FECameraSystem::getInstance()
}