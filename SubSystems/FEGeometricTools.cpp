#include "FEGeometricTools.h"
using namespace FocalEngine;

FEAABB::FEAABB()
{

}

FEAABB::FEAABB(glm::vec3 Min, glm::vec3 Max)
{
	min = Min;
	max = Max;
}

FEAABB::FEAABB(std::vector<glm::vec3>& VertexPositions)
{
	min.x = VertexPositions[0].x;
	min.y = VertexPositions[0].y;
	min.z = VertexPositions[0].z;

	max.x = VertexPositions[0].x;
	max.y = VertexPositions[0].y;
	max.z = VertexPositions[0].z;

	for (size_t i = 1; i < VertexPositions.size(); i++)
	{
		if (min.x > VertexPositions[i].x)
			min.x = VertexPositions[i].x;

		if (min.y > VertexPositions[i].y)
			min.y = VertexPositions[i].y;

		if (min.z > VertexPositions[i].z)
			min.z = VertexPositions[i].z;

		if (max.x < VertexPositions[i].x)
			max.x = VertexPositions[i].x;

		if (max.y < VertexPositions[i].y)
			max.y = VertexPositions[i].y;

		if (max.z < VertexPositions[i].z)
			max.z = VertexPositions[i].z;
	}
}

FEAABB::FEAABB(std::vector<float>& VertexPositions)
{
	min.x = VertexPositions[0];
	min.y = VertexPositions[1];
	min.z = VertexPositions[2];

	max.x = VertexPositions[0];
	max.y = VertexPositions[1];
	max.z = VertexPositions[2];

	for (size_t i = 3; i < VertexPositions.size(); i += 3)
	{
		if (min.x > VertexPositions[i])
			min.x = VertexPositions[i];

		if (min.y > VertexPositions[i + 1])
			min.y = VertexPositions[i + 1];

		if (min.z > VertexPositions[i + 2])
			min.z = VertexPositions[i + 2];

		if (max.x < VertexPositions[i])
			max.x = VertexPositions[i];

		if (max.y < VertexPositions[i + 1])
			max.y = VertexPositions[i + 1];

		if (max.z < VertexPositions[i + 2])
			max.z = VertexPositions[i + 2];
	}
}

FEAABB::FEAABB(float* VertexPositions, int VertexCount)
{
	min.x = VertexPositions[0];
	min.y = VertexPositions[1];
	min.z = VertexPositions[2];

	max.x = VertexPositions[0];
	max.y = VertexPositions[1];
	max.z = VertexPositions[2];

	for (size_t i = 3; i < size_t(VertexCount); i += 3)
	{
		if (min.x > VertexPositions[i])
			min.x = VertexPositions[i];

		if (min.y > VertexPositions[i + 1])
			min.y = VertexPositions[i + 1];

		if (min.z > VertexPositions[i + 2])
			min.z = VertexPositions[i + 2];

		if (max.x < VertexPositions[i])
			max.x = VertexPositions[i];

		if (max.y < VertexPositions[i + 1])
			max.y = VertexPositions[i + 1];

		if (max.z < VertexPositions[i + 2])
			max.z = VertexPositions[i + 2];
	}
}

FEAABB::~FEAABB()
{
}

glm::vec3 FEAABB::getMin()
{
	return min;
}

glm::vec3 FEAABB::getMax()
{
	return max;
}

bool FEAABB::rayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& distance)
{
	float tmin = (min.x - RayOrigin.x) / RayDirection.x;
	float tmax = (max.x - RayOrigin.x) / RayDirection.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (min.y - RayOrigin.y) / RayDirection.y;
	float tymax = (max.y - RayOrigin.y) / RayDirection.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (min.z - RayOrigin.z) / RayDirection.z;
	float tzmax = (max.z - RayOrigin.z) / RayDirection.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	distance = std::fmax(std::fmax(std::fmin(tmin, tmax), std::fmin(tymin, tymax)), std::fmin(tzmin, tzmax));
	return true;
}

// only for uniform sized AABB
FEAABB::FEAABB(glm::vec3 center, float size)
{
	float halfSize = size / 2.0f;
	min[0] = center[0] - halfSize;
	min[1] = center[1] - halfSize;
	min[2] = center[2] - halfSize;

	max[0] = center[0] + halfSize;
	max[1] = center[1] + halfSize;
	max[2] = center[2] + halfSize;
}

FEAABB::FEAABB(FEAABB other, glm::mat4 transformMatrix)
{
	// firstly we generate 8 points that represent AABBCube.
	// bottom 4 points
	glm::vec4 bottomLeftFront = glm::vec4(other.min.x, other.min.y, other.max.z, 1.0f);
	glm::vec4 bottomRightFront = glm::vec4(other.max.x, other.min.y, other.max.z, 1.0f);
	glm::vec4 bottomRightBack = glm::vec4(other.max.x, other.min.y, other.min.z, 1.0f);
	glm::vec4 bottomLeftBack = glm::vec4(other.min.x, other.min.y, other.min.z, 1.0f);
	// top 4 points
	glm::vec4 topLeftFront = glm::vec4(other.min.x, other.max.y, other.max.z, 1.0f);
	glm::vec4 topRightFront = glm::vec4(other.max.x, other.max.y, other.max.z, 1.0f);
	glm::vec4 topRightBack = glm::vec4(other.max.x, other.max.y, other.min.z, 1.0f);
	glm::vec4 topLeftBack = glm::vec4(other.min.x, other.max.y, other.min.z, 1.0f);

	// transform each point of this cube
	bottomLeftFront = transformMatrix * bottomLeftFront;
	bottomRightFront = transformMatrix * bottomRightFront;
	bottomRightBack = transformMatrix * bottomRightBack;
	bottomLeftBack = transformMatrix * bottomLeftBack;

	topLeftFront = transformMatrix * topLeftFront;
	topRightFront = transformMatrix * topRightFront;
	topRightBack = transformMatrix * topRightBack;
	topLeftBack = transformMatrix * topLeftBack;

	// for more convenient searching
	std::vector<glm::vec4> allPoints;
	allPoints.push_back(bottomLeftFront);
	allPoints.push_back(bottomRightFront);
	allPoints.push_back(bottomRightBack);
	allPoints.push_back(bottomLeftBack);

	allPoints.push_back(topLeftFront);
	allPoints.push_back(topRightFront);
	allPoints.push_back(topRightBack);
	allPoints.push_back(topLeftBack);

	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);
	for (auto point : allPoints)
	{
		if (point.x < min.x)
			min.x = point.x;

		if (point.x > max.x)
			max.x = point.x;

		if (point.y < min.y)
			min.y = point.y;

		if (point.y > max.y)
			max.y = point.y;

		if (point.z < min.z)
			min.z = point.z;

		if (point.z > max.z)
			max.z = point.z;
	}

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}