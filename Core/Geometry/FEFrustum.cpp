#include "FEFrustum.h"
using namespace FocalEngine;

std::vector<float> FEFrustum::GetAllPlanesCoefficients() const
{
	std::vector<float> Coefficients;

	glm::vec4 RightPlaneCoefficients = RightPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(RightPlaneCoefficients.x);
	Coefficients.push_back(RightPlaneCoefficients.y);
	Coefficients.push_back(RightPlaneCoefficients.z);
	Coefficients.push_back(RightPlaneCoefficients.w);

	glm::vec4 LeftPlaneCoefficients = LeftPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(LeftPlaneCoefficients.x);
	Coefficients.push_back(LeftPlaneCoefficients.y);
	Coefficients.push_back(LeftPlaneCoefficients.z);
	Coefficients.push_back(LeftPlaneCoefficients.w);

	glm::vec4 BottomPlaneCoefficients = BottomPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(BottomPlaneCoefficients.x);
	Coefficients.push_back(BottomPlaneCoefficients.y);
	Coefficients.push_back(BottomPlaneCoefficients.z);
	Coefficients.push_back(BottomPlaneCoefficients.w);

	glm::vec4 TopPlaneCoefficients = TopPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(TopPlaneCoefficients.x);
	Coefficients.push_back(TopPlaneCoefficients.y);
	Coefficients.push_back(TopPlaneCoefficients.z);
	Coefficients.push_back(TopPlaneCoefficients.w);

	glm::vec4 FarPlaneCoefficients = FarPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(FarPlaneCoefficients.x);
	Coefficients.push_back(FarPlaneCoefficients.y);
	Coefficients.push_back(FarPlaneCoefficients.z);
	Coefficients.push_back(FarPlaneCoefficients.w);

	glm::vec4 NearPlaneCoefficients = NearPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(NearPlaneCoefficients.x);
	Coefficients.push_back(NearPlaneCoefficients.y);
	Coefficients.push_back(NearPlaneCoefficients.z);
	Coefficients.push_back(NearPlaneCoefficients.w);

	return Coefficients;
}

std::vector<FELine> FEFrustum::GetFrustumLines(glm::vec3 Color, float LineWidth) const
{
	std::vector<FELine> Result;

	// Frustum could be drawn in different ways, this particular way is not most efficient.
	// But I wanted to try out geometric approach to draw frustum.

	// In frustum, we have 6 planes. We can find intersection of 3 planes to get 8 corners of frustum.
	// Then we can draw lines between these corners to get frustum.

	// Draw near plane
	std::optional<glm::vec3> IntersectionPoint = NearPlane.DoesIntersectPlanes(TopPlane, RightPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 NearTopRight = IntersectionPoint.value();

	IntersectionPoint = NearPlane.DoesIntersectPlanes(BottomPlane, RightPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 NearBottomRight = IntersectionPoint.value();

	IntersectionPoint = NearPlane.DoesIntersectPlanes(TopPlane, LeftPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 NearTopLeft = IntersectionPoint.value();

	IntersectionPoint = NearPlane.DoesIntersectPlanes(BottomPlane, LeftPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 NearBottomLeft = IntersectionPoint.value();

	// After we have 4 corners of near plane, we can add lines between them.
	Result.push_back(FELine(NearTopRight, NearTopLeft, Color, LineWidth));
	Result.push_back(FELine(NearTopLeft, NearBottomLeft, Color, LineWidth));
	Result.push_back(FELine(NearBottomLeft, NearBottomRight, Color, LineWidth));
	Result.push_back(FELine(NearBottomRight, NearTopRight, Color, LineWidth));

	// Find far plane corners
	IntersectionPoint = FarPlane.DoesIntersectPlanes(TopPlane, RightPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 FarTopRight = IntersectionPoint.value();

	IntersectionPoint = FarPlane.DoesIntersectPlanes(BottomPlane, RightPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 FarBottomRight = IntersectionPoint.value();

	IntersectionPoint = FarPlane.DoesIntersectPlanes(TopPlane, LeftPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 FarTopLeft = IntersectionPoint.value();

	IntersectionPoint = FarPlane.DoesIntersectPlanes(BottomPlane, LeftPlane);
	if (!IntersectionPoint.has_value())
	{
		// With proper frustum, this should never happen.
		LOG.Add("Incorrect frustum planes in FEFrustum::GetFrustumLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}
	glm::vec3 FarBottomLeft = IntersectionPoint.value();

	// Add lines between far plane corners
	Result.push_back(FELine(FarTopRight, FarTopLeft, Color, LineWidth));
	Result.push_back(FELine(FarTopLeft, FarBottomLeft, Color, LineWidth));
	Result.push_back(FELine(FarBottomLeft, FarBottomRight, Color, LineWidth));
	Result.push_back(FELine(FarBottomRight, FarTopRight, Color, LineWidth));

	// Add lines between near and far plane corners
	Result.push_back(FELine(NearTopRight, FarTopRight, Color, LineWidth));
	Result.push_back(FELine(NearTopLeft, FarTopLeft, Color, LineWidth));
	Result.push_back(FELine(NearBottomLeft, FarBottomLeft, Color, LineWidth));
	Result.push_back(FELine(NearBottomRight, FarBottomRight, Color, LineWidth));

	return Result;
}

bool FEFrustum::ContainsPoint(const glm::vec3& Point) const
{
	// A point is inside the frustum if it is on the negative side of all planes
	if (LeftPlane.SignedDistanceTo(Point) < 0) return false;
	if (RightPlane.SignedDistanceTo(Point) < 0) return false;
	if (BottomPlane.SignedDistanceTo(Point) < 0) return false;
	if (TopPlane.SignedDistanceTo(Point) < 0) return false;
	if (NearPlane.SignedDistanceTo(Point) < 0) return false;
	if (FarPlane.SignedDistanceTo(Point) < 0) return false;

	return true;
}

int FEFrustum::TestAABB(const FEAABB& Box) const
{
	int Result = 1;

	// First check all points of the AABB against the frustum planes.
	std::vector<glm::vec3> Corners = Box.GetCorners();
	int InsideCornersCount = 0;
	for (size_t i = 0; i < Corners.size(); i++)
	{
		if (ContainsPoint(Corners[i]))
			InsideCornersCount++;
	}

	if (InsideCornersCount == Corners.size())
		return 1; // Fully inside

	if (InsideCornersCount > 0)
		return 0; // Intersecting

	// All corners are outside but we can not be sure yet.
	if (InsideCornersCount == 0)
		Result = -1; 

	// FE_FIX_ME: That portion of code is not always working correctly.
	// If we reach here, we need to do a more thorough test.
	// Test against each plane.
	const FEPlane<float>* Planes[6] = { &LeftPlane, &RightPlane, &BottomPlane, &TopPlane, &NearPlane, &FarPlane };

	glm::vec3 Min = Box.GetMin();
	glm::vec3 Max = Box.GetMax();

	for (int i = 0; i < 6; i++)
	{
		const glm::vec3& Normal = Planes[i]->GetNormal();

		// Find the positive vertex (furthest along plane normal)
		glm::vec3 PositiveVertex;
		PositiveVertex.x = (Normal.x >= 0) ? Max.x : Min.x;
		PositiveVertex.y = (Normal.y >= 0) ? Max.y : Min.y;
		PositiveVertex.z = (Normal.z >= 0) ? Max.z : Min.z;

		// Find the negative vertex (furthest against plane normal)
		glm::vec3 NegativeVertex;
		NegativeVertex.x = (Normal.x >= 0) ? Min.x : Max.x;
		NegativeVertex.y = (Normal.y >= 0) ? Min.y : Max.y;
		NegativeVertex.z = (Normal.z >= 0) ? Min.z : Max.z;

		// If positive vertex is outside, the entire box is outside
		if (Planes[i]->SignedDistanceTo(PositiveVertex) < 0)
			return -1; // Outside

		// If negative vertex is outside, the box is intersecting
		if (Planes[i]->SignedDistanceTo(NegativeVertex) < 0)
			Result = 0; // Intersecting
	}

	return Result;
}

bool FEFrustum::IntersectsAABB(const FEAABB& Box) const
{
	return TestAABB(Box) != -1;
}

bool FEFrustum::ContainsAABB(const FEAABB& Box) const
{
	return TestAABB(Box) == 1;
}