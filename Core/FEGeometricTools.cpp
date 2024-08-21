#include "FEGeometricTools.h"
using namespace FocalEngine;

FEGeometry::FEGeometry() {}
FEGeometry::~FEGeometry() {}

FEAABB::FEAABB()
{

}

FEAABB::FEAABB(const glm::vec3 Min, const glm::vec3 Max)
{
	this->Min = Min;
	this->Max = Max;

	Size = this->Max - this->Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(glm::dvec3 Min, glm::dvec3 Max)
{
	this->Min = Min;
	this->Max = Max;

	Size = this->Max - this->Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
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

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(std::vector<glm::dvec3>& VertexPositions)
{
	Min.x = static_cast<float>(VertexPositions[0].x);
	Min.y = static_cast<float>(VertexPositions[0].y);
	Min.z = static_cast<float>(VertexPositions[0].z);

	Max.x = static_cast<float>(VertexPositions[0].x);
	Max.y = static_cast<float>(VertexPositions[0].y);
	Max.z = static_cast<float>(VertexPositions[0].z);

	for (size_t i = 1; i < VertexPositions.size(); i++)
	{
		if (Min.x > static_cast<float>(VertexPositions[i].x))
			Min.x = static_cast<float>(VertexPositions[i].x);

		if (Min.y > static_cast<float>(VertexPositions[i].y))
			Min.y = static_cast<float>(VertexPositions[i].y);

		if (Min.z > static_cast<float>(VertexPositions[i].z))
			Min.z = static_cast<float>(VertexPositions[i].z);

		if (Max.x < static_cast<float>(VertexPositions[i].x))
			Max.x = static_cast<float>(VertexPositions[i].x);

		if (Max.y < static_cast<float>(VertexPositions[i].y))
			Max.y = static_cast<float>(VertexPositions[i].y);

		if (Max.z < static_cast<float>(VertexPositions[i].z))
			Max.z = static_cast<float>(VertexPositions[i].z);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
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

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(std::vector<double>& VertexPositions)
{
	Min.x = static_cast<float>(VertexPositions[0]);
	Min.y = static_cast<float>(VertexPositions[1]);
	Min.z = static_cast<float>(VertexPositions[2]);

	Max.x = static_cast<float>(VertexPositions[0]);
	Max.y = static_cast<float>(VertexPositions[1]);
	Max.z = static_cast<float>(VertexPositions[2]);

	for (size_t i = 3; i < VertexPositions.size(); i += 3)
	{
		if (Min.x > static_cast<float>(VertexPositions[i]))
			Min.x = static_cast<float>(VertexPositions[i]);

		if (Min.y > static_cast<float>(VertexPositions[i + 1]))
			Min.y = static_cast<float>(VertexPositions[i + 1]);

		if (Min.z > static_cast<float>(VertexPositions[i + 2]))
			Min.z = static_cast<float>(VertexPositions[i + 2]);

		if (Max.x < static_cast<float>(VertexPositions[i]))
			Max.x = static_cast<float>(VertexPositions[i]);

		if (Max.y < static_cast<float>(VertexPositions[i + 1]))
			Max.y = static_cast<float>(VertexPositions[i + 1]);

		if (Max.z < static_cast<float>(VertexPositions[i + 2]))
			Max.z = static_cast<float>(VertexPositions[i + 2]);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
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

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(double* VertexPositions, const int VertexCount)
{
	Min.x = static_cast<float>(VertexPositions[0]);
	Min.y = static_cast<float>(VertexPositions[1]);
	Min.z = static_cast<float>(VertexPositions[2]);

	Max.x = static_cast<float>(VertexPositions[0]);
	Max.y = static_cast<float>(VertexPositions[1]);
	Max.z = static_cast<float>(VertexPositions[2]);

	for (size_t i = 3; i < static_cast<size_t>(VertexCount); i += 3)
	{
		if (Min.x > static_cast<float>(VertexPositions[i]))
			Min.x = static_cast<float>(VertexPositions[i]);

		if (Min.y > static_cast<float>(VertexPositions[i + 1]))
			Min.y = static_cast<float>(VertexPositions[i + 1]);

		if (Min.z > static_cast<float>(VertexPositions[i + 2]))
			Min.z = static_cast<float>(VertexPositions[i + 2]);

		if (Max.x < static_cast<float>(VertexPositions[i]))
			Max.x = static_cast<float>(VertexPositions[i]);

		if (Max.y < static_cast<float>(VertexPositions[i + 1]))
			Max.y = static_cast<float>(VertexPositions[i + 1]);

		if (Max.z < static_cast<float>(VertexPositions[i + 2]))
			Max.z = static_cast<float>(VertexPositions[i + 2]);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
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

bool FEAABB::RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, std::vector<glm::dvec3>& HitPoints)
{
	double tmin = (Min.x - RayOrigin.x) / RayDirection.x;
	double tmax = (Max.x - RayOrigin.x) / RayDirection.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	double tymin = (Min.y - RayOrigin.y) / RayDirection.y;
	double tymax = (Max.y - RayOrigin.y) / RayDirection.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	double tzmin = (Min.z - RayOrigin.z) / RayDirection.z;
	double tzmax = (Max.z - RayOrigin.z) / RayDirection.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	// If tmax < 0, the object is behind the ray, so we do not consider these cases as hits.
	if (tmax < 0.0)
		return false;

	// Clear previous hit points
	HitPoints.clear();

	// If tmin < 0, the origin is inside the AABB, so we only consider the exit point.
	if (tmin >= 0.0)
		HitPoints.push_back(RayOrigin + tmin * RayDirection);

	HitPoints.push_back(RayOrigin + tmax * RayDirection);

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

	this->Size = Max - Min;
	LongestAxisLength = std::max({ this->Size.x, this->Size.y, this->Size.z });
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

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB FEAABB::Transform(const glm::mat4 TransformMatrix)
{
	FEAABB Result;

	// firstly we generate 8 points that represent AABBCube.
	// bottom 4 points
	glm::vec4 BottomLeftFront = glm::vec4(Min.x, Min.y, Max.z, 1.0f);
	glm::vec4 BottomRightFront = glm::vec4(Max.x, Min.y, Max.z, 1.0f);
	glm::vec4 BottomRightBack = glm::vec4(Max.x, Min.y, Min.z, 1.0f);
	glm::vec4 BottomLeftBack = glm::vec4(Min.x, Min.y, Min.z, 1.0f);
	// top 4 points
	glm::vec4 TopLeftFront = glm::vec4(Min.x, Max.y, Max.z, 1.0f);
	glm::vec4 TopRightFront = glm::vec4(Max.x, Max.y, Max.z, 1.0f);
	glm::vec4 TopRightBack = glm::vec4(Max.x, Max.y, Min.z, 1.0f);
	glm::vec4 TopLeftBack = glm::vec4(Min.x, Max.y, Min.z, 1.0f);

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

	Result.Min = glm::vec3(FLT_MAX);
	Result.Max = glm::vec3(-FLT_MAX);
	for (const auto Point : AllPoints)
	{
		if (Point.x < Result.Min.x)
			Result.Min.x = Point.x;

		if (Point.x > Result.Max.x)
			Result.Max.x = Point.x;

		if (Point.y < Result.Min.y)
			Result.Min.y = Point.y;

		if (Point.y > Result.Max.y)
			Result.Max.y = Point.y;

		if (Point.z < Result.Min.z)
			Result.Min.z = Point.z;

		if (Point.z > Result.Max.z)
			Result.Max.z = Point.z;
	}

	Result.Size = Result.Max - Result.Min;
	Result.LongestAxisLength = std::max({ Result.Size.x, Result.Size.y, Result.Size.z });

	return Result;
}

FEAABB FEAABB::Merge(FEAABB& Other)
{
	if (this->LongestAxisLength == 0)
		return Other;

	FEAABB Result;

	Result.Min[0] = Min[0] < Other.Min[0] ? Min[0] : Other.Min[0];
	Result.Min[1] = Min[1] < Other.Min[1] ? Min[1] : Other.Min[1];
	Result.Min[2] = Min[2] < Other.Min[2] ? Min[2] : Other.Min[2];

	Result.Max[0] = Max[0] > Other.Max[0] ? Max[0] : Other.Max[0];
	Result.Max[1] = Max[1] > Other.Max[1] ? Max[1] : Other.Max[1];
	Result.Max[2] = Max[2] > Other.Max[2] ? Max[2] : Other.Max[2];

	Result.Size = Result.Max - Result.Min;
	Result.LongestAxisLength = std::max({ Result.Size.x, Result.Size.y, Result.Size.z });

	return Result;
}

float FEAABB::GetLongestAxisLength()
{
	if (Size == glm::vec3(0.0f))
		Size = Max - Min;

	if (LongestAxisLength == 0.0f)
		LongestAxisLength = std::max({ Size.x, Size.y, Size.z });

	return LongestAxisLength;
}

glm::vec3 FEAABB::GetCenter()
{
	return Min + abs(Min - Max) / 2.0f;
}

glm::vec3 FEAABB::GetSize()
{
	return Size;
}

bool FEAABB::ContainsPoint(const glm::vec3& Point) const
{
	return (Point.x >= Min.x && Point.x <= Max.x) &&
		   (Point.y >= Min.y && Point.y <= Max.y) &&
		   (Point.z >= Min.z && Point.z <= Max.z);
}

FEAABB FEAABB::GetIntersectionAABB(FEAABB& Other)
{
	if (!this->AABBIntersect(Other))
	{
		return FEAABB(glm::vec3(0.0f), glm::vec3(0.0f));
	}

	glm::vec3 IntersectionMin = glm::max(Min, Other.Min);
	glm::vec3 IntersectionMax = glm::min(Max, Other.Max);

	return FEAABB(IntersectionMin, IntersectionMax);
}

float FEAABB::GetVolume()
{
	return Size.x * Size.y * Size.z;
}

bool FEGeometry::IsEpsilonEqual(const glm::dvec3& FirstVector, const glm::dvec3& SecondVector, double Epsilon)
{
	return std::abs(FirstVector.x - SecondVector.x) < Epsilon &&
		   std::abs(FirstVector.y - SecondVector.y) < Epsilon &&
		   std::abs(FirstVector.z - SecondVector.z) < Epsilon;
}

bool FEGeometry::IsEpsilonEqual(const glm::dquat& FirstQuaternion, const glm::dquat& SecondQuaternion, double Epsilon)
{
	return glm::abs(glm::dot(FirstQuaternion, SecondQuaternion)) > (1.0 - Epsilon);
}

bool FEGeometry::IsEpsilonEqual(const glm::dmat4& FirstMatrix, const glm::dmat4& SecondMatrix, double Epsilon)
{
	return IsEpsilonEqual(FirstMatrix[0], SecondMatrix[0], Epsilon) &&
		   IsEpsilonEqual(FirstMatrix[1], SecondMatrix[1], Epsilon) &&
		   IsEpsilonEqual(FirstMatrix[2], SecondMatrix[2], Epsilon) &&
		   IsEpsilonEqual(FirstMatrix[3], SecondMatrix[3], Epsilon);
}

bool FEGeometry::DecomposeMatrixToTranslationRotationScale(const glm::dmat4& Matrix, glm::dvec3& OutTranslation, glm::dquat& OutRotationQuaternion, glm::dvec3& OutScale)
{
	// In rare cases glm::decompose can fail because of precision issues
	// So we will use double precision version of glm::decompose
	glm::dvec3 DoubleSkew;
	glm::dvec4 DoublePerspective;
	return glm::decompose(Matrix, OutScale, OutRotationQuaternion, OutTranslation, DoubleSkew, DoublePerspective);
}

glm::vec3 FEGeometry::CalculateNormal(glm::dvec3 FirstVertex, glm::dvec3 SecondVertex, glm::dvec3 ThirdVertex)
{
	glm::dvec3 Edge_0 = ThirdVertex - SecondVertex;
	glm::dvec3 Edge_1 = ThirdVertex - FirstVertex;

	glm::dvec3 Normal = glm::normalize(glm::cross(Edge_1, Edge_0));

	if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
		Normal = glm::dvec3();

	return Normal;
}

void FEGeometry::CalculateNormals(const std::vector<int>& Indices, const std::vector<float>& Vertices, std::vector<float>& NormalsToFill)
{
	std::vector<double> VerticesDouble;
	VerticesDouble.resize(Vertices.size());
	for (size_t i = 0; i < Vertices.size(); i++)
		VerticesDouble[i] = static_cast<double>(Vertices[i]);

	CalculateNormals(Indices, VerticesDouble, NormalsToFill);
}

void FEGeometry::CalculateNormals(const std::vector<int>& Indices, const std::vector<double>& Vertices, std::vector<float>& NormalsToFill)
{
	std::vector<double> TrianglesArea;
	std::vector<glm::dvec3> TrianglePoints;
	TrianglePoints.resize(3);

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		TrianglePoints[0] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		TrianglePoints[1] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		TrianglePoints[2] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		TrianglesArea.push_back(CalculateTriangleArea(TrianglePoints[0], TrianglePoints[1], TrianglePoints[2]));
	}

	struct VertexNormalsInfo
	{
		std::vector<glm::dvec3> Normals;
		std::vector<double> Areas;
		double AreaSum = 0.0;
	};

	std::vector<VertexNormalsInfo> DataForWeightedNormals;
	DataForWeightedNormals.resize(Indices.size());

	int IndexShift = 3;
	// We assume that there were no normals info read.
	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		glm::dvec3 V0 = { Vertices[Indices[i] * IndexShift], Vertices[Indices[i] * IndexShift + 1], Vertices[Indices[i] * IndexShift + 2] };
		glm::dvec3 V1 = { Vertices[Indices[i + 1] * IndexShift], Vertices[Indices[i + 1] * IndexShift + 1], Vertices[Indices[i + 1] * IndexShift + 2] };
		glm::dvec3 V2 = { Vertices[Indices[i + 2] * IndexShift], Vertices[Indices[i + 2] * IndexShift + 1], Vertices[Indices[i + 2] * IndexShift + 2] };

		glm::vec3 Normal = CalculateNormal(V0, V1, V2);

		DataForWeightedNormals[Indices[i]].Normals.push_back(Normal);
		DataForWeightedNormals[Indices[i]].Areas.push_back(TrianglesArea[i / 3]);
		DataForWeightedNormals[Indices[i]].AreaSum += TrianglesArea[i / 3];

		DataForWeightedNormals[Indices[i + 1]].Normals.push_back(Normal);
		DataForWeightedNormals[Indices[i + 1]].Areas.push_back(TrianglesArea[i / 3]);
		DataForWeightedNormals[Indices[i + 1]].AreaSum += TrianglesArea[i / 3];

		DataForWeightedNormals[Indices[i + 2]].Normals.push_back(Normal);
		DataForWeightedNormals[Indices[i + 2]].Areas.push_back(TrianglesArea[i / 3]);
		DataForWeightedNormals[Indices[i + 2]].AreaSum += TrianglesArea[i / 3];
	}

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		glm::vec3 Normal = glm::vec3(0.0f);
		for (size_t j = 0; j < DataForWeightedNormals[Indices[i]].Normals.size(); j++)
		{
			Normal += DataForWeightedNormals[Indices[i]].Normals[j] * DataForWeightedNormals[Indices[i]].Areas[j] / DataForWeightedNormals[Indices[i]].AreaSum;
		}
		Normal = glm::normalize(Normal);
		if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
			Normal = glm::vec3();

		NormalsToFill[Indices[i] * IndexShift] = Normal.x;
		NormalsToFill[Indices[i] * IndexShift + 1] = Normal.y;
		NormalsToFill[Indices[i] * IndexShift + 2] = Normal.z;

		Normal = glm::vec3(0.0f);
		for (size_t j = 0; j < DataForWeightedNormals[Indices[i + 1]].Normals.size(); j++)
		{
			Normal += DataForWeightedNormals[Indices[i + 1]].Normals[j] * DataForWeightedNormals[Indices[i + 1]].Areas[j] / DataForWeightedNormals[Indices[i + 1]].AreaSum;
		}
		Normal = glm::normalize(Normal);
		if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
			Normal = glm::vec3();

		NormalsToFill[Indices[i + 1] * IndexShift] = Normal.x;
		NormalsToFill[Indices[i + 1] * IndexShift + 1] = Normal.y;
		NormalsToFill[Indices[i + 1] * IndexShift + 2] = Normal.z;

		Normal = glm::vec3(0.0f);
		for (size_t j = 0; j < DataForWeightedNormals[Indices[i + 2]].Normals.size(); j++)
		{
			Normal += DataForWeightedNormals[Indices[i + 2]].Normals[j] * DataForWeightedNormals[Indices[i + 2]].Areas[j] / DataForWeightedNormals[Indices[i + 2]].AreaSum;
		}
		Normal = glm::normalize(Normal);
		if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
			Normal = glm::vec3();

		NormalsToFill[Indices[i + 2] * IndexShift] = Normal.x;
		NormalsToFill[Indices[i + 2] * IndexShift + 1] = Normal.y;
		NormalsToFill[Indices[i + 2] * IndexShift + 2] = Normal.z;
	}
}

glm::vec3 FEGeometry::CalculateTangent(const glm::vec3 FirstVertex, const glm::vec3 SecondVertex, const glm::vec3 ThirdVertex, std::vector<glm::vec2>&& TextureCoordinates)
{
	const glm::vec3 Q1 = SecondVertex - FirstVertex;
	const glm::vec3 Q2 = ThirdVertex - FirstVertex;
	const glm::vec2 UV0 = TextureCoordinates[0];
	const glm::vec2 UV1 = TextureCoordinates[1];
	const glm::vec2 UV2 = TextureCoordinates[2];

	const float T1 = UV1.y - UV0.y;
	const float T2 = UV2.y - UV0.y;

	const glm::vec3 Tangent = T1 * Q2 - T2 * Q1;
	return Tangent;
}

void FEGeometry::CalculateTangents(const std::vector<int>& Indices, const std::vector<float>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill)
{
	std::vector<double> VerticesDouble;
	VerticesDouble.resize(Vertices.size());
	for (size_t i = 0; i < Vertices.size(); i++)
		VerticesDouble[i] = static_cast<double>(Vertices[i]);

	CalculateTangents(Indices, VerticesDouble, TextureCoordinates, Normals, TangentsToFill);
}

void FEGeometry::CalculateTangents(const std::vector<int>& Indices, const std::vector<double>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill)
{
	for (size_t i = 0; i < Indices.size() - 1; i += 3)
	{
		const glm::vec3 V0 = { Vertices[Indices[i] * 3], Vertices[Indices[i] * 3 + 1], Vertices[Indices[i] * 3 + 2] };
		const glm::vec3 V1 = { Vertices[Indices[i + 1] * 3], Vertices[Indices[i + 1] * 3 + 1], Vertices[Indices[i + 1] * 3 + 2] };
		const glm::vec3 V2 = { Vertices[Indices[i + 2] * 3], Vertices[Indices[i + 2] * 3 + 1], Vertices[Indices[i + 2] * 3 + 2] };

		glm::vec2 T0 = { TextureCoordinates[Indices[i] * 2], TextureCoordinates[Indices[i] * 2 + 1] };
		glm::vec2 T1 = { TextureCoordinates[Indices[i + 1] * 2], TextureCoordinates[Indices[i + 1] * 2 + 1] };
		glm::vec2 T2 = { TextureCoordinates[Indices[i + 2] * 2], TextureCoordinates[Indices[i + 2] * 2 + 1] };

		glm::vec3 Tangent = GEOMETRY.CalculateTangent(V0, V1, V2, { T0, T1, T2 });
		// To eliminate NaN values after normalization.
		// I encounter this problem if triangle has same texture coordinates.
		if (Tangent.x != 0 || Tangent.y != 0 || Tangent.z != 0)
		{
			Tangent = glm::normalize(Tangent);
		}
		else
		{
			glm::vec3 Normal = { Normals[Indices[i] * 3], Normals[Indices[i] * 3 + 1], Normals[Indices[i] * 3 + 2] };
			glm::vec3 TangentOne = glm::cross(Normal, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 TangentTwo = glm::cross(Normal, glm::vec3(0.0f, 1.0f, 0.0f));
			// Choosing candidate with bigger length/magnitude.
			// Length/magnitude of cross product depend on sine of angle between vectors
			// and sine of 90 degrees is 1.0(max value), so basically we are choosing cross product in which vectors was closer to perpendicular(assuming both vectors are unit vectors).
			Tangent = glm::length(TangentOne) > glm::length(TangentTwo) ? TangentOne : TangentTwo;
			Tangent = glm::normalize(Tangent);
		}

		TangentsToFill[Indices[i] * 3] = Tangent.x;
		TangentsToFill[Indices[i] * 3 + 1] = Tangent.y;
		TangentsToFill[Indices[i] * 3 + 2] = Tangent.z;

		TangentsToFill[Indices[i + 1] * 3] = Tangent.x;
		TangentsToFill[Indices[i + 1] * 3 + 1] = Tangent.y;
		TangentsToFill[Indices[i + 1] * 3 + 2] = Tangent.z;

		TangentsToFill[Indices[i + 2] * 3] = Tangent.x;
		TangentsToFill[Indices[i + 2] * 3 + 1] = Tangent.y;
		TangentsToFill[Indices[i + 2] * 3 + 2] = Tangent.z;
	}
}

bool FEGeometry::RaysIntersection(const glm::vec3& FirstRayOrigin, const glm::vec3& FirstRayDirection, const glm::vec3& SecondRayOrigin, const glm::vec3& SecondRayDirection, float& FirstRayParametricIntersection, float& SecondRayParametricIntersection) const
{
	const glm::vec3 DirectionsCrossProduct = glm::cross(FirstRayDirection, SecondRayDirection);

	// Two rays are parallel.
	if (DirectionsCrossProduct == glm::vec3(0.0f))
		return false;

	FirstRayParametricIntersection = glm::dot(glm::cross((SecondRayOrigin - FirstRayOrigin), SecondRayDirection), DirectionsCrossProduct);
	FirstRayParametricIntersection /= glm::length(DirectionsCrossProduct) * glm::length(DirectionsCrossProduct);

	SecondRayParametricIntersection = glm::dot(glm::cross((SecondRayOrigin - FirstRayOrigin), FirstRayDirection), DirectionsCrossProduct);
	SecondRayParametricIntersection /= glm::length(DirectionsCrossProduct) * glm::length(DirectionsCrossProduct);

	return true;
}

bool FEGeometry::IsRayIntersectingTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& TriangleVertices, float& Distance, glm::vec3* HitPoint, float* U, float* V)
{
	// Ensure the triangle is defined by exactly three vertices
	if (TriangleVertices.size() != 3)
		return false;

	// Decompose RayDirection and triangle edges into components
	const float a = RayDirection[0];
	const float b = TriangleVertices[0][0] - TriangleVertices[1][0];
	const float c = TriangleVertices[0][0] - TriangleVertices[2][0];

	const float d = RayDirection[1];
	const float e = TriangleVertices[0][1] - TriangleVertices[1][1];
	const float f = TriangleVertices[0][1] - TriangleVertices[2][1];

	const float g = RayDirection[2];
	const float h = TriangleVertices[0][2] - TriangleVertices[1][2];
	const float j = TriangleVertices[0][2] - TriangleVertices[2][2];

	// Calculate vectors from RayOrigin to the first vertex of the triangle
	const float k = TriangleVertices[0][0] - RayOrigin[0];
	const float l = TriangleVertices[0][1] - RayOrigin[1];
	const float m = TriangleVertices[0][2] - RayOrigin[2];

	const glm::mat3 temp0 = glm::mat3 (a, b, c, d, e, f, g, h, j);
	const float determinant0 = glm::determinant(temp0);

	const glm::mat3 temp1 = glm::mat3(k, b, c, l, e, f, m, h, j);
	const float determinant1 = glm::determinant(temp1);

	// Calculate t from the first determinant and check if intersection is in the correct direction
	const float t = determinant1 / determinant0;
	const glm::mat3 temp2 = glm::mat3(a, k, c, d, l, f, g, m, j);
	const float determinant2 = glm::determinant(temp2);
	const float u = determinant2 / determinant0;

	const float determinant3 = glm::determinant(glm::mat3(a, b, k, d, e, l, g, h, m));
	const float v = determinant3 / determinant0;

	if (U != nullptr)
		*U = u;

	if (V != nullptr)
		*V = v;

	// Check if the intersection point lies within the triangle using barycentric coordinates
	if (t >= 0.00001 &&
		u >= 0.00001 && v >= 0.00001 &&
		u <= 1 && v <= 1 &&
		u + v >= 0.00001 &&
		u + v <= 1 && t > 0.00001)
	{
		// Calculate the exact intersection point if required
		if (HitPoint != nullptr)
			*HitPoint = TriangleVertices[0] + u * (TriangleVertices[1] - TriangleVertices[0]) + v * (TriangleVertices[2] - TriangleVertices[0]);

		Distance = t; // Set the distance to the intersection point
		return true; // Intersection detected
	}

	return false; // No valid intersection was found
}

bool FEGeometry::IsRayIntersectingTriangle(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, std::vector<glm::dvec3>& TriangleVertices, double& Distance, glm::dvec3* HitPoint, double* U, double* V)
{
	// Ensure the triangle is defined by exactly three vertices
	if (TriangleVertices.size() != 3)
		return false;

	// Decompose RayDirection and triangle edges into components
	const double a = RayDirection[0];
	const double b = TriangleVertices[0][0] - TriangleVertices[1][0];
	const double c = TriangleVertices[0][0] - TriangleVertices[2][0];

	const double d = RayDirection[1];
	const double e = TriangleVertices[0][1] - TriangleVertices[1][1];
	const double f = TriangleVertices[0][1] - TriangleVertices[2][1];

	const double g = RayDirection[2];
	const double h = TriangleVertices[0][2] - TriangleVertices[1][2];
	const double j = TriangleVertices[0][2] - TriangleVertices[2][2];

	// Calculate vectors from RayOrigin to the first vertex of the triangle
	const double k = TriangleVertices[0][0] - RayOrigin[0];
	const double l = TriangleVertices[0][1] - RayOrigin[1];
	const double m = TriangleVertices[0][2] - RayOrigin[2];

	const glm::dmat3 temp0 = glm::dmat3(a, b, c, d, e, f, g, h, j);
	const double determinant0 = glm::determinant(temp0);

	const glm::dmat3 temp1 = glm::dmat3(k, b, c, l, e, f, m, h, j);
	const double determinant1 = glm::determinant(temp1);

	// Calculate t from the first determinant and check if intersection is in the correct direction
	const double t = determinant1 / determinant0;

	const glm::dmat3 temp2 = glm::dmat3(a, k, c, d, l, f, g, m, j);
	const double determinant2 = glm::determinant(temp2);
	const double u = determinant2 / determinant0;

	const double determinant3 = glm::determinant(glm::dmat3(a, b, k, d, e, l, g, h, m));
	const double v = determinant3 / determinant0;

	if (U != nullptr)
		*U = u;

	if (V != nullptr)
		*V = v;

	// Check if the intersection point lies within the triangle using barycentric coordinates
	if (t >= 0.00001 &&
		u >= 0.00001 && v >= 0.00001 &&
		u <= 1 && v <= 1 &&
		u + v >= 0.00001 &&
		u + v <= 1 && t > 0.00001)
	{
		// Calculate the exact intersection point if required
		if (HitPoint != nullptr)
			*HitPoint = TriangleVertices[0] + u * (TriangleVertices[1] - TriangleVertices[0]) + v * (TriangleVertices[2] - TriangleVertices[0]);

		Distance = t; // Set the distance to the intersection point
		return true; // Intersection detected
	}

	return false; // No valid intersection was found
}

float FEGeometry::CalculateTriangleArea(std::vector<glm::vec3>& TriangleVertices)
{
	return CalculateTriangleArea(TriangleVertices[0], TriangleVertices[1], TriangleVertices[2]);
}

double FEGeometry::CalculateTriangleArea(std::vector<glm::dvec3>& TriangleVertices)
{
	return CalculateTriangleArea(TriangleVertices[0], TriangleVertices[1], TriangleVertices[2]);
}

float FEGeometry::CalculateTriangleArea(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 PointC)
{
	const float x1 = PointA.x;
	const float x2 = PointB.x;
	const float x3 = PointC.x;

	const float y1 = PointA.y;
	const float y2 = PointB.y;
	const float y3 = PointC.y;

	const float z1 = PointA.z;
	const float z2 = PointB.z;
	const float z3 = PointC.z;

	return 0.5f * static_cast<float>(sqrt(pow(x2 * y1 - x3 * y1 - x1 * y2 + x3 * y2 + x1 * y3 - x2 * y3, 2.0) +
										  pow((x2 * z1) - (x3 * z1) - (x1 * z2) + (x3 * z2) + (x1 * z3) - (x2 * z3), 2.0) +
										  pow((y2 * z1) - (y3 * z1) - (y1 * z2) + (y3 * z2) + (y1 * z3) - (y2 * z3), 2.0)));
}

double FEGeometry::CalculateTriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
{
	const double x1 = PointA.x;
	const double x2 = PointB.x;
	const double x3 = PointC.x;

	const double y1 = PointA.y;
	const double y2 = PointB.y;
	const double y3 = PointC.y;

	const double z1 = PointA.z;
	const double z2 = PointB.z;
	const double z3 = PointC.z;

	return 0.5 * sqrt(pow(x2 * y1 - x3 * y1 - x1 * y2 + x3 * y2 + x1 * y3 - x2 * y3, 2.0) +
					  pow((x2 * z1) - (x3 * z1) - (x1 * z2) + (x3 * z2) + (x1 * z3) - (x2 * z3), 2.0) +
					  pow((y2 * z1) - (y3 * z1) - (y1 * z2) + (y3 * z2) + (y1 * z3) - (y2 * z3), 2.0));
}

bool FEGeometry::IsAABBIntersectTriangle(FEAABB& AABB, std::vector<glm::vec3>& TriangleVertices)
{
	// Check if any of the triangle's vertices are inside the AABB, providing an early exit if true.
	if (AABB.ContainsPoint(TriangleVertices[0]) ||
		AABB.ContainsPoint(TriangleVertices[1]) ||
		AABB.ContainsPoint(TriangleVertices[2]))
		return true;

	// We will define 6 rays that represent 3 sides of triangle, for each side we will define 2 rays.
	std::vector<std::pair<glm::dvec3, glm::dvec3>> TriangleSidesRays = {
		// Edge 1: Vertex 0 to Vertex 1 and Vertex 1 to Vertex 0
		{TriangleVertices[0], TriangleVertices[1] - TriangleVertices[0]},
		{TriangleVertices[1], TriangleVertices[0] - TriangleVertices[1]},
		// Edge 2: Vertex 1 to Vertex 2 and Vertex 2 to Vertex 1
		{TriangleVertices[1], TriangleVertices[2] - TriangleVertices[1]},
		{TriangleVertices[2], TriangleVertices[1] - TriangleVertices[2]},
		// Edge 3: Vertex 2 to Vertex 0 and Vertex 0 to Vertex 2
		{TriangleVertices[2], TriangleVertices[0] - TriangleVertices[2]},
		{TriangleVertices[0], TriangleVertices[2] - TriangleVertices[0]},
	};

	for (int i = 0; i < TriangleSidesRays.size(); i += 2)
	{
		glm::dvec3 Origin = TriangleSidesRays[i].first;
		glm::dvec3 Direction = glm::normalize(TriangleSidesRays[i].second);

		std::vector<glm::dvec3> LocalResult;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (AABB.RayIntersect(Origin, Direction, LocalResult))
		{
			for (auto HitPoint : LocalResult)
				if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
					return true;
		}
		else
		{
			Origin = TriangleSidesRays[i + 1].first;
			Direction = glm::normalize(TriangleSidesRays[i + 1].second);

			std::vector<glm::dvec3> LocalResult;

			if (AABB.RayIntersect(Origin, Direction, LocalResult))
			{
				for (auto HitPoint : LocalResult)
					if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
						return true;
			}
		}
	}

	// Calculate the 8 corners of the AABB to use them for generating rays.
	std::vector<glm::vec3> Corners;
	// Bottom face corners
	Corners.push_back(AABB.GetMin());
	Corners.push_back(glm::vec3(AABB.GetMin().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::vec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(glm::vec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMax().z));
	// Top face corners
	Corners.push_back(glm::vec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMin().z));
	Corners.push_back(glm::vec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::vec3(AABB.GetMax().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(AABB.GetMax());

	// Define the rays along the edges of the AABB.
	std::vector<std::pair<glm::vec3, glm::vec3>> EdgesRays = {
		// Bottom face edges
		{Corners[0], Corners[1] - Corners[0]},
		{Corners[1], Corners[3] - Corners[1]},
		{Corners[3], Corners[2] - Corners[3]},
		{Corners[2], Corners[0] - Corners[2]},
		// Top face edges
		{Corners[4], Corners[5] - Corners[4]},
		{Corners[5], Corners[7] - Corners[5]},
		{Corners[7], Corners[6] - Corners[7]},
		{Corners[6], Corners[4] - Corners[6]},
		// Vertical edges
		{Corners[0], Corners[4] - Corners[0]},
		{Corners[1], Corners[5] - Corners[1]},
		{Corners[3], Corners[7] - Corners[3]},
		{Corners[2], Corners[6] - Corners[2]}
	};

	// Test each edge of the AABB as a ray to check for intersections with the triangle.
	for (const auto& Ray : EdgesRays)
	{
		glm::vec3 Origin = Ray.first;
		glm::vec3 Direction = Ray.second;

		float Distance;
		glm::vec3 HitPoint;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (IsRayIntersectingTriangle(Origin, Direction, TriangleVertices, Distance, &HitPoint))
		{
			// ...and the intersection point is within the edge's span (i.e., the edge 'hits' the triangle),
			// conclude that the AABB and triangle intersect.
			if (glm::length(HitPoint - Origin) <= glm::length(Direction))
				return true;
		}
	}

	// If none of the edges intersect with the triangle, there is no intersection.
	return false;
}

bool FEGeometry::IsAABBIntersectTriangle(FEAABB& AABB, std::vector<glm::dvec3>& TriangleVertices)
{
	// Check if any of the triangle's vertices are inside the AABB, providing an early exit if true.
	if (AABB.ContainsPoint(TriangleVertices[0]) ||
		AABB.ContainsPoint(TriangleVertices[1]) ||
		AABB.ContainsPoint(TriangleVertices[2]))
		return true;

	// We will define 6 rays that represent 3 sides of triangle, for each side we will define 2 rays.
	std::vector<std::pair<glm::dvec3, glm::dvec3>> TriangleSidesRays = {
		// Edge 1: Vertex 0 to Vertex 1 and Vertex 1 to Vertex 0
		{TriangleVertices[0], TriangleVertices[1] - TriangleVertices[0]},
		{TriangleVertices[1], TriangleVertices[0] - TriangleVertices[1]},
		// Edge 2: Vertex 1 to Vertex 2 and Vertex 2 to Vertex 1
		{TriangleVertices[1], TriangleVertices[2] - TriangleVertices[1]},
		{TriangleVertices[2], TriangleVertices[1] - TriangleVertices[2]},
		// Edge 3: Vertex 2 to Vertex 0 and Vertex 0 to Vertex 2
		{TriangleVertices[2], TriangleVertices[0] - TriangleVertices[2]},
		{TriangleVertices[0], TriangleVertices[2] - TriangleVertices[0]},
	};

	for (int i = 0; i < TriangleSidesRays.size(); i += 2)
	{
		glm::dvec3 Origin = TriangleSidesRays[i].first;
		glm::dvec3 Direction = glm::normalize(TriangleSidesRays[i].second);

		std::vector<glm::dvec3> LocalResult;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (AABB.RayIntersect(Origin, Direction, LocalResult))
		{
			for (auto HitPoint : LocalResult)
				if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
					return true;
		}
		else
		{
			Origin = TriangleSidesRays[i + 1].first;
			Direction = glm::normalize(TriangleSidesRays[i + 1].second);

			std::vector<glm::dvec3> LocalResult;

			if (AABB.RayIntersect(Origin, Direction, LocalResult))
			{
				for (auto HitPoint : LocalResult)
					if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
						return true;
			}
		}
	}

	// Calculate the 8 corners of the AABB to use them for generating rays.
	std::vector<glm::dvec3> Corners;
	// Bottom face corners
	Corners.push_back(AABB.GetMin());
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMax().z));
	// Top face corners
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMin().z));
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(AABB.GetMax());

	// Define the rays along the edges of the AABB.
	std::vector<std::pair<glm::dvec3, glm::dvec3>> EdgesRays = {
		// Bottom face edges
		{Corners[0], Corners[1] - Corners[0]},
		{Corners[1], Corners[3] - Corners[1]},
		{Corners[3], Corners[2] - Corners[3]},
		{Corners[2], Corners[0] - Corners[2]},
		// Top face edges
		{Corners[4], Corners[5] - Corners[4]},
		{Corners[5], Corners[7] - Corners[5]},
		{Corners[7], Corners[6] - Corners[7]},
		{Corners[6], Corners[4] - Corners[6]},
		// Vertical edges
		{Corners[0], Corners[4] - Corners[0]},
		{Corners[1], Corners[5] - Corners[1]},
		{Corners[3], Corners[7] - Corners[3]},
		{Corners[2], Corners[6] - Corners[2]}
	};

	// Test each edge of the AABB as a ray to check for intersections with the triangle.
	for (const auto& Ray : EdgesRays)
	{
		glm::dvec3 Origin = Ray.first;
		glm::dvec3 Direction = Ray.second;

		double Distance;
		glm::dvec3 HitPoint;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (IsRayIntersectingTriangle(Origin, Direction, TriangleVertices, Distance, &HitPoint))
		{
			// ...and the intersection point is within the edge's span (i.e., the edge 'hits' the triangle),
			// conclude that the AABB and triangle intersect.
			if (glm::length(HitPoint - Origin) <= glm::length(Direction))
				return true;
		}
	}

	// If none of the edges intersect with the triangle, there is no intersection.
	return false;
}

std::vector<glm::dvec3> FEGeometry::GetIntersectionPoints(FEAABB& AABB, std::vector<glm::dvec3> TriangleVertices)
{
	std::vector<glm::dvec3> Result;

	if (TriangleVertices.size() != 3)
		return Result;

	// We will define 6 rays that represent 3 sides of triangle, for each side we will define 2 rays.
	std::vector<std::pair<glm::dvec3, glm::dvec3>> TriangleSidesRays = {
		// Edge 1: Vertex 0 to Vertex 1 and Vertex 1 to Vertex 0
		{TriangleVertices[0], TriangleVertices[1] - TriangleVertices[0]},
		{TriangleVertices[1], TriangleVertices[0] - TriangleVertices[1]},
		// Edge 2: Vertex 1 to Vertex 2 and Vertex 2 to Vertex 1
		{TriangleVertices[1], TriangleVertices[2] - TriangleVertices[1]},
		{TriangleVertices[2], TriangleVertices[1] - TriangleVertices[2]},
		// Edge 3: Vertex 2 to Vertex 0 and Vertex 0 to Vertex 2
		{TriangleVertices[2], TriangleVertices[0] - TriangleVertices[2]},
		{TriangleVertices[0], TriangleVertices[2] - TriangleVertices[0]},
	};

	for (int i=0; i < TriangleSidesRays.size(); i+=2)
	{
		glm::dvec3 Origin = TriangleSidesRays[i].first;
		glm::dvec3 Direction = glm::normalize(TriangleSidesRays[i].second);

		//float Distance = 0.0f;
		std::vector<glm::dvec3> LocalResult;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (AABB.RayIntersect(Origin, Direction, LocalResult))
		{
			for (auto HitPoint : LocalResult)
				if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
					Result.push_back(HitPoint);

			if (LocalResult.size() > 0)
				continue;
		}
		else
		{
			Origin = TriangleSidesRays[i + 1].first;
			Direction = glm::normalize(TriangleSidesRays[i + 1].second);

			std::vector<glm::dvec3> LocalResult;

			if (AABB.RayIntersect(Origin, Direction, LocalResult))
			{
				for (auto HitPoint : LocalResult)
					if (glm::length(HitPoint - Origin) <= glm::length(TriangleSidesRays[i].second))
						Result.push_back(HitPoint);
			}
		}
	}

	// Clear duplicates
	for (size_t i = 0; i < Result.size(); i++)
	{
		int PointsThatAreNotSame = 0;
		for (size_t j = 0; j < Result.size(); j++)
		{
			if (i == j)
				continue;

			if (abs(Result[i] - Result[j]).x > glm::dvec3(DBL_EPSILON).x ||
				abs(Result[i] - Result[j]).y > glm::dvec3(DBL_EPSILON).y ||
				abs(Result[i] - Result[j]).z > glm::dvec3(DBL_EPSILON).z)
			{
				PointsThatAreNotSame++;
			}
		}

		if (PointsThatAreNotSame != Result.size() - 1)
		{
			Result.erase(Result.begin() + i);
			i--;
			break;
		}
	}

	// Calculate the 8 corners of the AABB to use them for generating rays.
	std::vector<glm::dvec3> Corners;
	// Bottom face corners
	Corners.push_back(AABB.GetMin());
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(glm::dvec3(AABB.GetMin().x, AABB.GetMax().y, AABB.GetMax().z));
	// Top face corners
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMin().z));
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMin().y, AABB.GetMax().z));
	Corners.push_back(glm::dvec3(AABB.GetMax().x, AABB.GetMax().y, AABB.GetMin().z));
	Corners.push_back(AABB.GetMax());

	// Define the rays along the edges of the AABB.
	std::vector<std::pair<glm::dvec3, glm::dvec3>> EdgesRays = {
		// Bottom face edges
		{Corners[0], Corners[1] - Corners[0]},
		{Corners[1], Corners[3] - Corners[1]},
		{Corners[3], Corners[2] - Corners[3]},
		{Corners[2], Corners[0] - Corners[2]},
		// Top face edges
		{Corners[4], Corners[5] - Corners[4]},
		{Corners[5], Corners[7] - Corners[5]},
		{Corners[7], Corners[6] - Corners[7]},
		{Corners[6], Corners[4] - Corners[6]},
		// Vertical edges
		{Corners[0], Corners[4] - Corners[0]},
		{Corners[1], Corners[5] - Corners[1]},
		{Corners[3], Corners[7] - Corners[3]},
		{Corners[2], Corners[6] - Corners[2]}
	};

	// Test each edge of the AABB as a ray to check for intersections with the triangle.
	for (const auto& Ray : EdgesRays)
	{
		glm::dvec3 Origin = Ray.first;
		glm::dvec3 Direction = Ray.second;

		double Distance;
		glm::dvec3 HitPoint;

		// If a ray originating from an edge of the AABB intersects with the triangle...
		if (IsRayIntersectingTriangle(Origin, Direction, TriangleVertices, Distance, &HitPoint))
		{
			// ...and the intersection point is within the edge's span (i.e., the edge 'hits' the triangle),
			// conclude that the AABB and triangle intersect.
			if (glm::length(HitPoint - Origin) <= glm::length(Direction))
			{
				Result.push_back(HitPoint);
			}
		}
	}

	// Clear duplicates
	for (size_t i = 0; i < Result.size(); i++)
	{
		int PointsThatAreNotSame = 0;
		for (size_t j = 0; j < Result.size(); j++)
		{
			if (i == j)
				continue;

			if (abs(Result[i] - Result[j]).x > glm::dvec3(DBL_EPSILON).x ||
				abs(Result[i] - Result[j]).y > glm::dvec3(DBL_EPSILON).y ||
				abs(Result[i] - Result[j]).z > glm::dvec3(DBL_EPSILON).z)
			{
				PointsThatAreNotSame++;
			}
		}

		if (PointsThatAreNotSame != Result.size() - 1)
		{
			Result.erase(Result.begin() + i);
			i--;
			break;
		}
	}

	return Result;
}

glm::dvec3 FEGeometry::CreateMouseRayToWorld(const double MouseScreenX, const double MouseScreenY, const glm::dmat4 ViewMatrix, const glm::dmat4 ProjectionMatrix, const glm::ivec2 ViewportPosition, const glm::ivec2 ViewportSize) const
{
	glm::dvec2 NormalizedMouseCoordinates;
	NormalizedMouseCoordinates.x = (2.0f * (MouseScreenX - ViewportPosition.x)) / ViewportSize.x - 1;
	NormalizedMouseCoordinates.y = 1.0f - (2.0f * ((MouseScreenY - ViewportPosition.y))) / ViewportSize.y;

	const glm::dvec4 ClipCoordinates = glm::dvec4(NormalizedMouseCoordinates.x, NormalizedMouseCoordinates.y, -1.0, 1.0);
	glm::dvec4 EyeCoordinates = glm::inverse(ProjectionMatrix) * ClipCoordinates;
	EyeCoordinates.z = -1.0f;
	EyeCoordinates.w = 0.0f;

	glm::dvec3 WorldRay = glm::inverse(ViewMatrix) * EyeCoordinates;
	WorldRay = glm::normalize(WorldRay);

	return WorldRay;
}