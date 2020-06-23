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
		glm::mat4 lightSpaceBig;
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
		int activeCascades;
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

		float shadowBias;
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
		
		float getCascadeDistributionExponent();
		void setCascadeDistributionExponent(float newCascadeDistributionExponent);

		float getFirstCascadeSize();
		void setFirstCascadeSize(float newFirstCascadeSize);
	protected:
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		bool useCascadeShadows = true;
		int activeCascades = 4;
		float firstCascadeSize = 50.0f;
		float cascadeDistributionExponent = 4.0f;
		FECascadeData cascadeData[4];

		void updateCascades(glm::vec3 cameraPosition, glm::vec3 cameraDirection);
		void updateProjectionMat();
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