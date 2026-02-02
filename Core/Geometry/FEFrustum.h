#pragma once

#include "FEAABB.h"

namespace FocalEngine
{
	struct FEFrustum
	{
		FEPlane<float> LeftPlane;
		FEPlane<float> RightPlane;
		FEPlane<float> BottomPlane;
		FEPlane<float> TopPlane;
		FEPlane<float> NearPlane;
		FEPlane<float> FarPlane;

		std::vector<float> GetAllPlanesCoefficients();
		std::vector<FELine> GetFrustumLines(glm::vec3 Color, float LineWidth);
	};
}