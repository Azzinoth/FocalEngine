#include "FEGeometricTools.h"
using namespace FocalEngine;

FEAABB::FEAABB()
{

}

FEAABB::FEAABB(const glm::vec3 Min, const glm::vec3 Max)
{
	this->Min = Min;
	this->Max = Max;

	Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > Size)
		Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > Size)
		Size = abs(Max.z - Min.z);
}

FEAABB::FEAABB(std::vector<glm::vec3>& VertexPositions)
{
	Min.x = VertexPositions[0].x;
	Min.y = VertexPositions[0].y;
	Min.z = VertexPositions[0].z;

	Max.x = VertexPositions[0].x;
	Max.y = VertexPositions[0].y;
	Max.z = VertexPositions[0].z;

	for (size_t i = 1; i < VertexPositions.size(); i++)
	{
		if (Min.x > VertexPositions[i].x)
			Min.x = VertexPositions[i].x;

		if (Min.y > VertexPositions[i].y)
			Min.y = VertexPositions[i].y;

		if (Min.z > VertexPositions[i].z)
			Min.z = VertexPositions[i].z;

		if (Max.x < VertexPositions[i].x)
			Max.x = VertexPositions[i].x;

		if (Max.y < VertexPositions[i].y)
			Max.y = VertexPositions[i].y;

		if (Max.z < VertexPositions[i].z)
			Max.z = VertexPositions[i].z;
	}

	Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > Size)
		Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > Size)
		Size = abs(Max.z - Min.z);
}

FEAABB::FEAABB(std::vector<float>& VertexPositions)
{
	Min.x = VertexPositions[0];
	Min.y = VertexPositions[1];
	Min.z = VertexPositions[2];

	Max.x = VertexPositions[0];
	Max.y = VertexPositions[1];
	Max.z = VertexPositions[2];

	for (size_t i = 3; i < VertexPositions.size(); i += 3)
	{
		if (Min.x > VertexPositions[i])
			Min.x = VertexPositions[i];

		if (Min.y > VertexPositions[i + 1])
			Min.y = VertexPositions[i + 1];

		if (Min.z > VertexPositions[i + 2])
			Min.z = VertexPositions[i + 2];

		if (Max.x < VertexPositions[i])
			Max.x = VertexPositions[i];

		if (Max.y < VertexPositions[i + 1])
			Max.y = VertexPositions[i + 1];

		if (Max.z < VertexPositions[i + 2])
			Max.z = VertexPositions[i + 2];
	}

	Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > Size)
		Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > Size)
		Size = abs(Max.z - Min.z);
}

FEAABB::FEAABB(float* VertexPositions, const int VertexCount)
{
	Min.x = VertexPositions[0];
	Min.y = VertexPositions[1];
	Min.z = VertexPositions[2];

	Max.x = VertexPositions[0];
	Max.y = VertexPositions[1];
	Max.z = VertexPositions[2];

	for (size_t i = 3; i < static_cast<size_t>(VertexCount); i += 3)
	{
		if (Min.x > VertexPositions[i])
			Min.x = VertexPositions[i];

		if (Min.y > VertexPositions[i + 1])
			Min.y = VertexPositions[i + 1];

		if (Min.z > VertexPositions[i + 2])
			Min.z = VertexPositions[i + 2];

		if (Max.x < VertexPositions[i])
			Max.x = VertexPositions[i];

		if (Max.y < VertexPositions[i + 1])
			Max.y = VertexPositions[i + 1];

		if (Max.z < VertexPositions[i + 2])
			Max.z = VertexPositions[i + 2];
	}

	Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > Size)
		Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > Size)
		Size = abs(Max.z - Min.z);
}

FEAABB::~FEAABB()
{
}

glm::vec3 FEAABB::GetMin()
{
	return Min;
}

glm::vec3 FEAABB::GetMax()
{
	return Max;
}

bool FEAABB::RayIntersect(const glm::vec3 RayOrigin, const glm::vec3 RayDirection, float& Distance)
{
	float tmin = (Min.x - RayOrigin.x) / RayDirection.x;
	float tmax = (Max.x - RayOrigin.x) / RayDirection.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (Min.y - RayOrigin.y) / RayDirection.y;
	float tymax = (Max.y - RayOrigin.y) / RayDirection.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (Min.z - RayOrigin.z) / RayDirection.z;
	float tzmax = (Max.z - RayOrigin.z) / RayDirection.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	Distance = std::fmax(std::fmax(std::fmin(tmin, tmax), std::fmin(tymin, tymax)), std::fmin(tzmin, tzmax));
	return true;
}

// only for uniform sized AABB
FEAABB::FEAABB(glm::vec3 Center, const float Size)
{
	const float HalfSize = Size / 2.0f;
	Min[0] = Center[0] - HalfSize;
	Min[1] = Center[1] - HalfSize;
	Min[2] = Center[2] - HalfSize;

	Max[0] = Center[0] + HalfSize;
	Max[1] = Center[1] + HalfSize;
	Max[2] = Center[2] + HalfSize;

	this->Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > this->Size)
		this->Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > this->Size)
		this->Size = abs(Max.z - Min.z);
}

FEAABB::FEAABB(FEAABB Other, glm::mat4 TransformMatrix)
{
	// firstly we generate 8 points that represent AABBCube.
	// bottom 4 points
	glm::vec4 BottomLeftFront = glm::vec4(Other.Min.x, Other.Min.y, Other.Max.z, 1.0f);
	glm::vec4 BottomRightFront = glm::vec4(Other.Max.x, Other.Min.y, Other.Max.z, 1.0f);
	glm::vec4 BottomRightBack = glm::vec4(Other.Max.x, Other.Min.y, Other.Min.z, 1.0f);
	glm::vec4 BottomLeftBack = glm::vec4(Other.Min.x, Other.Min.y, Other.Min.z, 1.0f);
	// top 4 points
	glm::vec4 TopLeftFront = glm::vec4(Other.Min.x, Other.Max.y, Other.Max.z, 1.0f);
	glm::vec4 TopRightFront = glm::vec4(Other.Max.x, Other.Max.y, Other.Max.z, 1.0f);
	glm::vec4 TopRightBack = glm::vec4(Other.Max.x, Other.Max.y, Other.Min.z, 1.0f);
	glm::vec4 TopLeftBack = glm::vec4(Other.Min.x, Other.Max.y, Other.Min.z, 1.0f);

	// transform each point of this cube
	BottomLeftFront = TransformMatrix * BottomLeftFront;
	BottomRightFront = TransformMatrix * BottomRightFront;
	BottomRightBack = TransformMatrix * BottomRightBack;
	BottomLeftBack = TransformMatrix * BottomLeftBack;

	TopLeftFront = TransformMatrix * TopLeftFront;
	TopRightFront = TransformMatrix * TopRightFront;
	TopRightBack = TransformMatrix * TopRightBack;
	TopLeftBack = TransformMatrix * TopLeftBack;

	// for more convenient searching
	std::vector<glm::vec4> AllPoints;
	AllPoints.push_back(BottomLeftFront);
	AllPoints.push_back(BottomRightFront);
	AllPoints.push_back(BottomRightBack);
	AllPoints.push_back(BottomLeftBack);

	AllPoints.push_back(TopLeftFront);
	AllPoints.push_back(TopRightFront);
	AllPoints.push_back(TopRightBack);
	AllPoints.push_back(TopLeftBack);

	Min = glm::vec3(FLT_MAX);
	Max = glm::vec3(-FLT_MAX);
	for (auto Point : AllPoints)
	{
		if (Point.x < Min.x)
			Min.x = Point.x;

		if (Point.x > Max.x)
			Max.x = Point.x;

		if (Point.y < Min.y)
			Min.y = Point.y;

		if (Point.y > Max.y)
			Max.y = Point.y;

		if (Point.z < Min.z)
			Min.z = Point.z;

		if (Point.z > Max.z)
			Max.z = Point.z;
	}

	Size = abs(Max.x - Min.x);
	if (abs(Max.y - Min.y) > Size)
		Size = abs(Max.y - Min.y);

	if (abs(Max.z - Min.z) > Size)
		Size = abs(Max.z - Min.z);
}

float FEAABB::GetSize()
{
	if (Size == 0.0f)
	{
		Size = abs(Max.x - Min.x);
		if (abs(Max.y - Min.y) > Size)
			Size = abs(Max.y - Min.y);

		if (abs(Max.z - Min.z) > Size)
			Size = abs(Max.z - Min.z);
	}

	return Size;
}

glm::vec3 FEAABB::GetCenter()
{
	return Min + abs(Min - Max) / 2.0f;
}