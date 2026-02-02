#pragma once

#include "../Core/FECoreIncludes.h"

namespace FocalEngine
{
	struct FELine
	{
		glm::vec3 StartPoint;
		glm::vec3 EndPoint;
		glm::vec3 Color = glm::vec3(1.0f);
		float Width = 1.0f;

		FELine() = default;
		FELine(glm::vec3 StartPoint, glm::vec3 EndPoint, glm::vec3 LineColor = glm::vec3(1.0f), float LineWidth = 1.0f);
	};
}