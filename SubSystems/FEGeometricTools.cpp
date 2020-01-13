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

FEAABB::~FEAABB()
{
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