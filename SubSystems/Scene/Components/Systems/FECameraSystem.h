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

		void RenderTargetResize(int Width, int Height);
		void Update(const double DeltaTime);

		// FIX ME! This is temporary
		void SetCursorToCenter(FECameraComponent& Camera);
		int RenderTargetCenterX = 0;
		int RenderTargetCenterY = 0;
		int RenderTargetShiftX = 0;
		int RenderTargetShiftY = 0;
		glm::dvec3 PolarToCartesian(double PolarAngle, double AzimutAngle, const double R);
	public:
		SINGLETON_PUBLIC_PART(FECameraSystem)

		void SetMainCamera(FEEntity* CameraEntity);
		FEEntity* GetMainCameraEntity(FEScene* Scene) const;

		void SetIsIndividualInputActive(FEEntity* CameraEntity, const bool Active);
		void IndividualUpdate(FEEntity* CameraEntity, const double DeltaTime);
	};

#define CAMERA_SYSTEM FECameraSystem::getInstance()
}