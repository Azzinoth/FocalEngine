#pragma once
#include "../Renderer/FEFramebuffer.h"

namespace FocalEngine
{
	enum FE_LIGHT_TYPE
	{
		FE_NULL_LIGHT = 0,
		FE_DIRECTIONAL_LIGHT = 1,
		FE_POINT_LIGHT = 2,
		FE_SPOT_LIGHT = 3
	};

	struct FELightShaderInfo
	{
		glm::vec4 TypeAndAngles = glm::vec4(-1.0f);
		glm::vec4 Position = glm::vec4(0.0f);
		glm::vec4 Color = glm::vec4(0.0f);
		glm::vec4 Direction = glm::vec4(0.0f);
		glm::mat4 LightSpace = glm::mat4(0.0f);
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

	struct FECascadeData
	{
		friend FEScene;
		friend FERenderer;
	public:
		FECascadeData();
		FECascadeData(const FECascadeData& Other);
		void operator=(const FECascadeData& Other);
		~FECascadeData();

		float Size;
		glm::mat4 ProjectionMat;
		glm::mat4 ViewMat;
		FEFramebuffer* FrameBuffer = nullptr;
		float** Frustum = nullptr;
	};

	struct FELightComponent
	{
		friend class FELightSystem;
		friend class FERenderer;

		FELightComponent();
		FELightComponent(FE_LIGHT_TYPE Type);
		FELightComponent(FELightComponent& Other) = default;
		~FELightComponent();

		FE_LIGHT_TYPE GetType();

		glm::vec3 GetColor();
		void SetColor(glm::vec3 NewValue);

		float GetIntensity();
		void SetIntensity(float NewValue);

		float GetRange();
		void SetRange(float NewValue);

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

		int GetActiveCascades();
		void SetActiveCascades(int NewValue);

		float GetShadowCoverage();
		void SetShadowCoverage(float NewValue);

		float GetCSMZDepth();
		void SetCSMZDepth(float NewValue);

		float GetCSMXYDepth();
		void SetCSMXYDepth(float NewValue);

		float GetSpotAngle();
		void SetSpotAngle(float NewValue);

		float GetSpotAngleOuter();
		void SetSpotAngleOuter(float NewValue);
	private:
		FE_LIGHT_TYPE Type = FE_NULL_LIGHT;

		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;

		bool bEnabled = true;

		bool bStaticShadowBias = false;
		float ShadowBias = 0.001f;
		float ShadowBiasVariableIntensity = 1.0f;
		bool bCastShadows = true;
		float ShadowBlurFactor = 1.0f;
		float Range = 10.0f;

		// Directional Light Specific
		glm::vec3 Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 DefaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		bool bUseCascadeShadows = true;
		int ActiveCascades = 4;
		float ShadowCoverage = 50.0f;
		float CSMZDepth = 3.0f;
		float CSMXYDepth = 1.0f;
		FECascadeData CascadeData[4];

		// Spot Light Specific
		float SpotAngle = 30.0f;
		float SpotAngleOuter = 45.0f;
	};
}