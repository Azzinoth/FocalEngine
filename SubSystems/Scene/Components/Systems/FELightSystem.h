#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FELightSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FELightSystem)

		bool bInternalAdd = false;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void DirectionalLightInitialization(FELightComponent& LightComponent);
		static void DuplicateLightComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value LightComponentToJson(FEEntity* Entity);
		static void LightComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FELightSystem)

		glm::vec3 GetDirection(FEEntity* LightEntity);

		void UpdateCascades(FEEntity* LightEntity, float CameraFov, float AspectRatio, float NearPlane, float FarPlane, glm::mat4 ViewMatrix, glm::vec3 CameraForward, glm::vec3 CameraRight, glm::vec3 CameraUp);
	};

#define LIGHT_SYSTEM FELightSystem::GetInstance()
}