#include "FEFrustum.h"
using namespace FocalEngine;

std::vector<float> FEFrustum::GetAllPlanesCoefficients()
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

std::vector<FELine> FEFrustum::GetFrustumLines(glm::vec3 Color, float LineWidth)
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