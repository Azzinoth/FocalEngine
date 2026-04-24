#include "FEAABB.h"
using namespace FocalEngine;

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

FEAABB::FEAABB(std::vector<glm::vec3>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min = glm::vec3(Positions[0].x, Positions[0].y, Positions[0].z);
	Max = Min;

	for (size_t i = 1; i < Positions.size(); i++)
	{
		if (Min.x > Positions[i].x)
			Min.x = Positions[i].x;

		if (Min.y > Positions[i].y)
			Min.y = Positions[i].y;

		if (Min.z > Positions[i].z)
			Min.z = Positions[i].z;

		if (Max.x < Positions[i].x)
			Max.x = Positions[i].x;

		if (Max.y < Positions[i].y)
			Max.y = Positions[i].y;

		if (Max.z < Positions[i].z)
			Max.z = Positions[i].z;
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(std::vector<glm::dvec3>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min = glm::vec3(static_cast<float>(Positions[0].x), static_cast<float>(Positions[0].y), static_cast<float>(Positions[0].z));
	Max = Min;

	for (size_t i = 1; i < Positions.size(); i++)
	{
		if (Min.x > static_cast<float>(Positions[i].x))
			Min.x = static_cast<float>(Positions[i].x);

		if (Min.y > static_cast<float>(Positions[i].y))
			Min.y = static_cast<float>(Positions[i].y);

		if (Min.z > static_cast<float>(Positions[i].z))
			Min.z = static_cast<float>(Positions[i].z);

		if (Max.x < static_cast<float>(Positions[i].x))
			Max.x = static_cast<float>(Positions[i].x);

		if (Max.y < static_cast<float>(Positions[i].y))
			Max.y = static_cast<float>(Positions[i].y);

		if (Max.z < static_cast<float>(Positions[i].z))
			Max.z = static_cast<float>(Positions[i].z);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(std::vector<glm::vec2>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min = glm::vec3(Positions[0].x, Positions[0].y, 0.0f);
	Max = Min;

	for (size_t i = 1; i < Positions.size(); i++)
	{
		if (Positions[i].x < Min.x) Min.x = Positions[i].x;
		if (Positions[i].y < Min.y) Min.y = Positions[i].y;

		if (Positions[i].x > Max.x) Max.x = Positions[i].x;
		if (Positions[i].y > Max.y) Max.y = Positions[i].y;
	}

	Size = Max - Min;
	LongestAxisLength = glm::max(Size.x, Size.y);
}

FEAABB::FEAABB(std::vector<glm::dvec2>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min = glm::vec3(static_cast<float>(Positions[0].x), static_cast<float>(Positions[0].y), 0.0f);
	Max = Min;

	for (size_t i = 1; i < Positions.size(); i++)
	{
		float X = static_cast<float>(Positions[i].x);
		float Y = static_cast<float>(Positions[i].y);

		if (X < Min.x) Min.x = X;
		if (Y < Min.y) Min.y = Y;

		if (X > Max.x) Max.x = X;
		if (Y > Max.y) Max.y = Y;
	}

	Size = Max - Min;
	LongestAxisLength = glm::max(Size.x, Size.y);
}

FEAABB::FEAABB(std::vector<float>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (Positions.size() % 3 != 0)
	{
		LOG.Add("FEAABB::FEAABB: Positions size is not a multiple of 3, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min.x = Positions[0];
	Min.y = Positions[1];
	Min.z = Positions[2];

	Max.x = Positions[0];
	Max.y = Positions[1];
	Max.z = Positions[2];

	for (size_t i = 3; i < Positions.size(); i += 3)
	{
		if (Min.x > Positions[i])
			Min.x = Positions[i];

		if (Min.y > Positions[i + 1])
			Min.y = Positions[i + 1];

		if (Min.z > Positions[i + 2])
			Min.z = Positions[i + 2];

		if (Max.x < Positions[i])
			Max.x = Positions[i];

		if (Max.y < Positions[i + 1])
			Max.y = Positions[i + 1];

		if (Max.z < Positions[i + 2])
			Max.z = Positions[i + 2];
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(std::vector<double>& Positions)
{
	if (Positions.empty())
	{
		LOG.Add("FEAABB::FEAABB: Positions is empty, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (Positions.size() % 3 != 0)
	{
		LOG.Add("FEAABB::FEAABB: Positions size is not a multiple of 3, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min.x = static_cast<float>(Positions[0]);
	Min.y = static_cast<float>(Positions[1]);
	Min.z = static_cast<float>(Positions[2]);

	Max.x = static_cast<float>(Positions[0]);
	Max.y = static_cast<float>(Positions[1]);
	Max.z = static_cast<float>(Positions[2]);

	for (size_t i = 3; i < Positions.size(); i += 3)
	{
		if (Min.x > static_cast<float>(Positions[i]))
			Min.x = static_cast<float>(Positions[i]);

		if (Min.y > static_cast<float>(Positions[i + 1]))
			Min.y = static_cast<float>(Positions[i + 1]);

		if (Min.z > static_cast<float>(Positions[i + 2]))
			Min.z = static_cast<float>(Positions[i + 2]);

		if (Max.x < static_cast<float>(Positions[i]))
			Max.x = static_cast<float>(Positions[i]);

		if (Max.y < static_cast<float>(Positions[i + 1]))
			Max.y = static_cast<float>(Positions[i + 1]);

		if (Max.z < static_cast<float>(Positions[i + 2]))
			Max.z = static_cast<float>(Positions[i + 2]);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(float* Positions, const int Count)
{
	if (Count == 0)
	{
		LOG.Add("FEAABB::FEAABB: Count is zero, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (Count % 3 != 0)
	{
		LOG.Add("FEAABB::FEAABB: Count is not a multiple of 3, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min.x = Positions[0];
	Min.y = Positions[1];
	Min.z = Positions[2];

	Max.x = Positions[0];
	Max.y = Positions[1];
	Max.z = Positions[2];

	for (size_t i = 3; i < static_cast<size_t>(Count); i += 3)
	{
		if (Min.x > Positions[i])
			Min.x = Positions[i];

		if (Min.y > Positions[i + 1])
			Min.y = Positions[i + 1];

		if (Min.z > Positions[i + 2])
			Min.z = Positions[i + 2];

		if (Max.x < Positions[i])
			Max.x = Positions[i];

		if (Max.y < Positions[i + 1])
			Max.y = Positions[i + 1];

		if (Max.z < Positions[i + 2])
			Max.z = Positions[i + 2];
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::FEAABB(double* Positions, const int Count)
{
	if (Count == 0)
	{
		LOG.Add("FEAABB::FEAABB: Count is zero, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (Count % 3 != 0)
	{
		LOG.Add("FEAABB::FEAABB: Count is not a multiple of 3, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Min.x = static_cast<float>(Positions[0]);
	Min.y = static_cast<float>(Positions[1]);
	Min.z = static_cast<float>(Positions[2]);

	Max.x = static_cast<float>(Positions[0]);
	Max.y = static_cast<float>(Positions[1]);
	Max.z = static_cast<float>(Positions[2]);

	for (size_t i = 3; i < static_cast<size_t>(Count); i += 3)
	{
		if (Min.x > static_cast<float>(Positions[i]))
			Min.x = static_cast<float>(Positions[i]);

		if (Min.y > static_cast<float>(Positions[i + 1]))
			Min.y = static_cast<float>(Positions[i + 1]);

		if (Min.z > static_cast<float>(Positions[i + 2]))
			Min.z = static_cast<float>(Positions[i + 2]);

		if (Max.x < static_cast<float>(Positions[i]))
			Max.x = static_cast<float>(Positions[i]);

		if (Max.y < static_cast<float>(Positions[i + 1]))
			Max.y = static_cast<float>(Positions[i + 1]);

		if (Max.z < static_cast<float>(Positions[i + 2]))
			Max.z = static_cast<float>(Positions[i + 2]);
	}

	Size = Max - Min;
	LongestAxisLength = std::max({ Size.x, Size.y, Size.z });
}

FEAABB::~FEAABB()
{
}

glm::vec3 FEAABB::GetMin() const
{
	return Min;
}

glm::vec3 FEAABB::GetMax() const
{
	return Max;
}

template<typename T>
bool FEAABB::RayIntersectInternal(const T& RayOrigin, const T& RayDirection, typename T::value_type& HitMin, typename T::value_type& HitMax)
{
	using Scalar = typename T::value_type;

	Scalar XHitMin = static_cast<Scalar>(Min.x - RayOrigin.x) / RayDirection.x;
	Scalar XHitMax = static_cast<Scalar>(Max.x - RayOrigin.x) / RayDirection.x;
	if (XHitMin > XHitMax) std::swap(XHitMin, XHitMax);

	Scalar YHitMin = static_cast<Scalar>(Min.y - RayOrigin.y) / RayDirection.y;
	Scalar YHitMax = static_cast<Scalar>(Max.y - RayOrigin.y) / RayDirection.y;
	if (YHitMin > YHitMax) std::swap(YHitMin, YHitMax);

	if ((XHitMin > YHitMax) || (YHitMin > XHitMax))
		return false;

	if (YHitMin > XHitMin)
		XHitMin = YHitMin;
	if (YHitMax < XHitMax)
		XHitMax = YHitMax;

	Scalar ZHitMin = static_cast<Scalar>(Min.z - RayOrigin.z) / RayDirection.z;
	Scalar ZHitMax = static_cast<Scalar>(Max.z - RayOrigin.z) / RayDirection.z;
	if (ZHitMin > ZHitMax) std::swap(ZHitMin, ZHitMax);

	if ((XHitMin > ZHitMax) || (ZHitMin > XHitMax))
		return false;

	if (ZHitMin > XHitMin)
		XHitMin = ZHitMin;
	if (ZHitMax < XHitMax)
		XHitMax = ZHitMax;

	HitMin = XHitMin;
	HitMax = XHitMax;
	return true;
}

bool FEAABB::RayIntersect(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, float& Distance)
{
	float HitMin, HitMax;
	if (!RayIntersectInternal(RayOrigin, RayDirection, HitMin, HitMax))
		return false;

	Distance = (HitMin >= 0.0f) ? HitMin : HitMax;
	return true;
}

bool FEAABB::RayIntersect(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, std::vector<glm::vec3>& HitPoints)
{
	float HitMin, HitMax;
	if (!RayIntersectInternal(RayOrigin, RayDirection, HitMin, HitMax))
		return false;

	if (HitMax < 0.0f)
		return false;

	HitPoints.clear();

	// If tmin < 0, the origin is inside the AABB, so we only consider the exit point.
	if (HitMin >= 0.0f)
		HitPoints.push_back(RayOrigin + HitMin * RayDirection);

	HitPoints.push_back(RayOrigin + HitMax * RayDirection);
	return true;
}

bool FEAABB::RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, double& Distance)
{
	double HitMin, HitMax;
	if (!RayIntersectInternal(RayOrigin, RayDirection, HitMin, HitMax))
		return false;

	Distance = (HitMin >= 0.0) ? HitMin : HitMax;
	return true;
}

bool FEAABB::RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, std::vector<glm::dvec3>& HitPoints)
{
	double HitMin, HitMax;
	if (!RayIntersectInternal(RayOrigin, RayDirection, HitMin, HitMax))
		return false;

	if (HitMax < 0.0)
		return false;

	// If tmin < 0, the origin is inside the AABB, so we only consider the exit point.
	if (HitMin >= 0.0)
		HitPoints.push_back(RayOrigin + HitMin * RayDirection);

	HitPoints.push_back(RayOrigin + HitMax * RayDirection);
	return true;
}

FEAABB::FEAABB(glm::vec3 Center, const float Size)
{
	if (Size < 0.0f)
	{
		LOG.Add("FEAABB::FEAABB: Size is negative, cannot create AABB.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

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

	Min = glm::vec3(std::numeric_limits<float>::max());
	Max = glm::vec3(-std::numeric_limits<float>::max());
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

	Result.Min = glm::vec3(std::numeric_limits<float>::max());
	Result.Max = glm::vec3(-std::numeric_limits<float>::max());
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

glm::vec3 FEAABB::GetApproximateForwardDirection() const
{
	glm::vec3 Result = glm::vec3(0.0f);
	if (glm::all(glm::epsilonEqual(Size, glm::vec3(0.0f), glm::vec3(1e-6f))))
		return Result;

	// Find the largest and second largest axes
	float Axes[3] = { Size.x, Size.y, Size.z };
	int PrimaryAxis = 0;
	int SecondaryAxis = 1;
	int TertiaryAxis = 2;

	// Sort to find primary (largest), secondary (middle), and tertiary (smallest) axes
	if (Axes[1] > Axes[PrimaryAxis])
	{
		SecondaryAxis = PrimaryAxis;
		PrimaryAxis = 1;
	}
	if (Axes[2] > Axes[PrimaryAxis])
	{
		TertiaryAxis = SecondaryAxis;
		SecondaryAxis = PrimaryAxis;
		PrimaryAxis = 2;
	}
	else if (Axes[2] > Axes[SecondaryAxis])
	{
		TertiaryAxis = SecondaryAxis;
		SecondaryAxis = 2;
	}

	// Ensure tertiary axis is the remaining one
	for (int i = 0; i < 3; i++)
	{
		if (i != PrimaryAxis && i != SecondaryAxis)
		{
			TertiaryAxis = i;
			break;
		}
	}

	// If it is a cube, return zero vector
	if (abs(Axes[0] - Axes[1]) < 1e-6f && abs(Axes[1] - Axes[2]) < 1e-6f && abs(Axes[0] - Axes[2]) < 1e-6f)
		return Result;

	// Calculate the magnitude based on how much larger the primary axis is
	if (Axes[PrimaryAxis] < 1e-6f || Axes[SecondaryAxis] < 1e-6f)
	{
		Result[TertiaryAxis] = 1.0f;
		return Result;
	}

	float Magnitude = Axes[PrimaryAxis] / Axes[SecondaryAxis];
	Result[TertiaryAxis] = Magnitude;

	return Result;
}

std::vector<glm::vec3> FEAABB::GetCorners() const
{
	std::vector<glm::vec3> Result;
	
	// Bottom face
	Result.push_back(glm::vec3(Min.x, Min.y, Min.z));
	Result.push_back(glm::vec3(Max.x, Min.y, Min.z));
	Result.push_back(glm::vec3(Max.x, Min.y, Max.z));
	Result.push_back(glm::vec3(Min.x, Min.y, Max.z));
	// Top face
	Result.push_back(glm::vec3(Min.x, Max.y, Min.z));
	Result.push_back(glm::vec3(Max.x, Max.y, Min.z));
	Result.push_back(glm::vec3(Max.x, Max.y, Max.z));
	Result.push_back(glm::vec3(Min.x, Max.y, Max.z));
	
	return Result;
}