#pragma once

#include "FEFramebuffer.h"
#include "../SubSystems/FETransformComponent.h"

namespace FocalEngine
{
	enum FELightType
	{
		FE_DIRECTIONAL_LIGHT = 0,
		FE_POINT_LIGHT       = 1,
		FE_SPOT_LIGHT        = 2,
	};

	struct FELightShaderInfo
	{
		glm::vec3 typeAndAngles = glm::vec3(-1.0f);
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 direction;
		glm::mat4 lightSpace;
	};

	struct FEDirectionalLightShaderInfo
	{
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 direction;
		glm::mat4 CSM0;
		glm::mat4 CSM1;
		glm::mat4 CSM2;
		glm::mat4 CSM3;
		glm::vec4 CSMSizes;
		int activeCascades;
		float biasFixed;
		float biasVariableIntensity;
		float intensity;
	};

	class FEScene;
	class FERenderer;

	class FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FELight(FELightType Type);
		~FELight();

		glm::vec3 getColor();
		void setColor(glm::vec3 newColor);

		float getIntensity();
		void setIntensity(float newIntensity);

		FETransformComponent transform;

		bool isLightEnabled();
		void setLightEnabled(bool isLightEnabled);

		float getShadowBias();
		void setShadowBias(float newShadowBias);

		bool isStaticShadowBias();
		void setIsStaticShadowBias(bool isStaticShadowBias);

		float getShadowBiasVariableIntensity();
		void setShadowBiasVariableIntensity(float newShadowBiasVariableIntensity);

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		FELightType getType();

		std::string getName();
		void setName(std::string newName);
	protected:
		FELightType type;
		std::string name;

		glm::vec3 color = glm::vec3(1.0f);
		float intensity = 1.0f;

		bool enabled = true;

		bool staticShadowBias = false;
		float shadowBias = 0.001f;
		float shadowBiasVariableIntensity = 1.0f;
		bool castShadows = true;
	};

	struct FECascadeData
	{
		friend FEScene;
		friend FERenderer;
	public:
		FECascadeData();
		~FECascadeData();

		float size;
		glm::mat4 projectionMat;
		glm::mat4 viewMat;
		FEFramebuffer* frameBuffer = nullptr;
	};

	class FEDirectionalLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FEDirectionalLight();
		~FEDirectionalLight();

		glm::vec3 getDirection();
		void setDirection(glm::vec3 newDirection);

		int getActiveCascades();
		void setActiveCascades(int newActiveCascades);

		float getShadowCoverage();
		void setShadowCoverage(float newShadowCoverage);

		float getCSMZDepth();
		void setCSMZDepth(float newCSMZDepth);

		float getCSMXYDepth();
		void setCSMXYDepth(float newCSMXYDepth);
	protected:
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		bool useCascadeShadows = true;
		int activeCascades = 4;
		float shadowCoverage = 50.0f;
		float CSMZDepth = 3.0f;
		float CSMXYDepth = 1.0f;
		FECascadeData cascadeData[4];

		void updateCascades(float cameraFov, float aspectRatio, float nearPlane, float farPlane, glm::mat4 viewMatrix, glm::vec3 cameraForward, glm::vec3 cameraRight, glm::vec3 cameraUp);
	};

	class FESpotLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FESpotLight();
		~FESpotLight();

		float getRange();
		void setRange(float newRange);

		float getSpotAngle();
		void setSpotAngle(float newSpotAngle);

		float getSpotAngleOuter();
		void setSpotAngleOuter(float newSpotAngleOuter);

		glm::vec3 getDirection();
		void setDirection(glm::vec3 newDirection);

	protected:
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		float spotAngle = 30.0f;
		float spotAngleOuter = 45.0f;
		float range = 10;
	};

	class FEPointLight : public FELight
	{
		friend FEScene;
		friend FERenderer;
	public:
		FEPointLight();
		~FEPointLight();

		float getRange();
		void setRange(float newRange);
	protected:
		float range = 10;
	};
}