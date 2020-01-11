#pragma once

#include "../SubSystems/FECoreIncludes.h"

namespace FocalEngine
{
	enum FELightType
	{
		FE_DIRECTIONAL_LIGHT = 0,
		FE_POINT_LIGHT       = 1,
		FE_SPOT_LIGHT        = 2,
	};

	class FELight
	{
	public:
		FELight(FELightType Type);
		~FELight();

		glm::vec3 getColor();
		void setColor(glm::vec3 newColor);

		float getIntensity();
		void setIntensity(float newIntensity);

		glm::vec3 getPosition();
		void setPosition(glm::vec3 newPosition);

		float getRange();
		void setRange(float newRange);

		bool isLightEnabled();
		void setLightEnabled(bool isLightEnabled);

		bool isLightProjectingShadow();
		void setLightProjectingShadow(bool isLightEnabled);

		float getShadowBias();
		void setShadowBias(float newShadowBias);

		bool isCastShadows();
		void setCastShadows(bool isCastShadows);
	private:
		FELightType type;

		glm::vec3 color = glm::vec3(1.0f);
		float intensity = 1.0f;
		glm::vec3 position = glm::vec3(0.0f);

		float range;
		bool enabled;
		bool projectShadow;
		float shadowBias;
		bool castShadows;
	};
}