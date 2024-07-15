#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FELightSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FELightSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();
		void OnSceneClear();
	public:
		SINGLETON_PUBLIC_PART(FELightSystem)

		glm::vec3 GetDirection(FEEntity* LightEntity);
		//void SetDirection(FEEntity* LightEntity, glm::vec3 NewValue);

		void UpdateCascades(FEEntity* LightEntity, float CameraFov, float AspectRatio, float NearPlane, float FarPlane, glm::mat4 ViewMatrix, glm::vec3 CameraForward, glm::vec3 CameraRight, glm::vec3 CameraUp);
	};

#define LIGHT_SYSTEM FELightSystem::getInstance()
}