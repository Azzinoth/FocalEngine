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

		std::vector<float> GetAllPlanesCoefficients() const;
		std::vector<FELine> GetFrustumLines(glm::vec3 Color, float LineWidth) const;

		bool ContainsPoint(const glm::vec3& Point) const;

		// This function do not produce correct result in all circumstances!
		// Check if an AABB intersects or is inside the frustum
		// Returns: -1 = outside, 0 = intersecting, 1 = fully inside
		int TestAABB(const FEAABB& Box) const;
		// This function do not produce correct result in all circumstances!
		bool IntersectsAABB(const FEAABB& Box) const;
		// This function do not produce correct result in all circumstances!
		bool ContainsAABB(const FEAABB& Box) const;
	};
}