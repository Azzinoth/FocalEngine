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

		glm::vec3 getDirection();
		void setDirection(glm::vec3 newDirection);

		float getRange();
		void setRange(float newRange);

		float getSpotAngle();
		void setSpotAngle(float newSpotAngle);

		float getSpotAngleOuter();
		void setSpotAngleOuter(float newSpotAngleOuter);

		bool isLightEnabled();
		void setLightEnabled(bool isLightEnabled);

		float getShadowBias();
		void setShadowBias(float newShadowBias);

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);

		FELightType getType();

		std::string getName();
		void setName(std::string newName);


		FEFramebuffer* shadowMap = nullptr;
	private:
		FELightType type;
		std::string name;

		glm::vec3 color = glm::vec3(1.0f);
		float intensity = 1.0f;

		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		float spotAngle = 30.0f;
		float spotAngleOuter = 45.0f;

		float range = 10;
		bool enabled = true;

		float shadowBias;
		bool castShadows = true;
		float shadowMapProjectionSize;
		glm::mat4 shadowProjectionMatrix;
		glm::mat4 getViewMatrixForShadowMap();
	};
}