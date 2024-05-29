#pragma once

#include "FEFramebuffer.h"
#include "../SubSystems/Scene/FETransformComponent.h"

namespace FocalEngine
{
	struct FELightShaderInfo
	{
		glm::vec3 TypeAndAngles = glm::vec3(-1.0f);
		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec4 Direction;
		glm::mat4 LightSpace;
	};

	struct FEDirectionalLightShaderInfo
	{
		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec4 Direction;
		glm::mat4 CSM0;
		glm::mat4 CSM1;
		glm::mat4 CSM2;
		glm::mat4 CSM3;
		glm::vec4 CSMSizes;
		int ActiveCascades;
		float BiasFixed;
		float BiasVariableIntensity;
		float Intensity;
	};

	class FEScene;
	class FERenderer;

	class FELight : public FEObject
	{
		friend FEScene;
		friend FERenderer;
	public:
		FELight(FE_OBJECT_TYPE LightType);
		~FELight();

		glm::vec3 GetColor();
		void SetColor(glm::vec3 NewValue);

		float GetIntensity();
		void SetIntensity(float NewValue);

		FETransformComponent Transform;

		bool IsLightEnabled();
		void SetLightEnabled(bool NewValue);

		float GetShadowBias();
		void SetShadowBias(float NewValue);

		bool IsStaticShadowBias();
		void SetIsStaticShadowBias(bool NewValue);

		float GetShadowBiasVariableIntensity();
		void SetShadowBiasVariableIntensity(float NewValue);

		float GetShadowBlurFactor();
		void SetShadowBlurFactor(float NewValue);

		bool IsCastShadows();
		void SetCastShadows(bool NewValue);
	protected:
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;

		bool bEnabled = true;

		bool bStaticShadowBias = false;
		float ShadowBias = 0.001f;
		float ShadowBiasVariableIntensity = 1.0f;
		bool bCastShadows = true;
		float ShadowBlurFactor = 1.0f;
	};

	struct FECascadeData
	{
		friend FEScene;
		friend FERenderer;
	public:
		FECascadeData();
		~FECascadeData();

		float Size;
		glm::mat4 ProjectionMat;
		glm::mat4 ViewMat;
		FEFramebuffer* FrameBuffer = nullptr;
		float** Frustum;
	};

	class FEDirectionalLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FEDirectionalLight();
		~FEDirectionalLight();

		glm::vec3 GetDirection();
		void SetDirection(glm::vec3 NewValue);

		int GetActiveCascades();
		void SetActiveCascades(int NewValue);

		float GetShadowCoverage();
		void SetShadowCoverage(float NewValue);

		float GetCSMZDepth();
		void SetCSMZDepth(float NewValue);

		float GetCSMXYDepth();
		void SetCSMXYDepth(float NewValue);

		void SetCastShadows(bool NewValue);
	protected:
		glm::vec3 Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 DefaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		bool bUseCascadeShadows = true;
		int ActiveCascades = 4;
		float ShadowCoverage = 50.0f;
		float CSMZDepth = 3.0f;
		float CSMXYDepth = 1.0f;
		FECascadeData CascadeData[4];

		void UpdateCascades(float CameraFov, float AspectRatio, float NearPlane, float FarPlane, glm::mat4 ViewMatrix, glm::vec3 CameraForward, glm::vec3 CameraRight, glm::vec3 CameraUp);
	};

	class FESpotLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FESpotLight();
		~FESpotLight();

		float GetRange();
		void SetRange(float NewValue);

		float GetSpotAngle();
		void SetSpotAngle(float NewValue);

		float GetSpotAngleOuter();
		void SetSpotAngleOuter(float NewValue);

		glm::vec3 GetDirection();
		void SetDirection(glm::vec3 NewValue);

	protected:
		glm::vec3 Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 DefaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		float SpotAngle = 30.0f;
		float SpotAngleOuter = 45.0f;
		float Range = 10;
	};

	class FEPointLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FEPointLight();
		~FEPointLight();

		float GetRange();
		void SetRange(float NewValue);
	protected:
		float Range = 10;
	};
}