#pragma once

#include "../SubSystems/FECoreIncludes.h"

namespace FocalEngine
{
	class FELight
	{
	public:
		FELight();
		~FELight();

		glm::vec3 getColor();
		void setColor(glm::vec3 newColor);
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

	private:
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec3 position = glm::vec3(0.0f);
		float range;
		bool enabled;
		bool projectShadow;
		float shadowBias;
	};
}