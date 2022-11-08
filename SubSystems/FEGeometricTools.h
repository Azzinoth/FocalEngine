#pragma once

#include "FECoreIncludes.h"

namespace FocalEngine
{
	class FEEntity;
	class FEEntityInstanced;
	class FETerrain;
	class FEResourceManager;

	class FEAABB
	{
		friend FEEntity;
		friend FEEntityInstanced;
		friend FETerrain;
		friend FEResourceManager;
	public:
		FEAABB();
		FEAABB(glm::vec3 Min, glm::vec3 Max);
		FEAABB(std::vector<glm::vec3>& VertexPositions);
		FEAABB(std::vector<float>& VertexPositions);
		FEAABB(float* VertexPositions, int VertexCount);
		// only for uniform sized AABB
		FEAABB(glm::vec3 Center, float Size);
		FEAABB(FEAABB Other, glm::mat4 TransformMatrix);
		~FEAABB();

		glm::vec3 GetMin();
		glm::vec3 GetMax();

		bool RayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& Distance);
		inline bool FEAABB::AABBIntersect(FEAABB Other)
		{
			if (Max[0] < Other.Min[0] || Min[0] > Other.Max[0]) return false;
			if (Max[1] < Other.Min[1] || Min[1] > Other.Max[1]) return false;
			if (Max[2] < Other.Min[2] || Min[2] > Other.Max[2]) return false;
			return true;

			/*__m128 max_ = _mm_set_ps(max[0], max[1], max[2], max[2]);
			__m128 otherMin_ = _mm_set_ps(other.min[0], other.min[1], other.min[2], other.min[2]);
			__m128 result1 = _mm_cmpgt_ps(max_, otherMin_);

			__m128 min_ = _mm_set_ps(min[0], min[1], min[2], min[2]);
			__m128 otherMax_ = _mm_set_ps(other.max[0], other.max[1], other.max[2], other.max[2]);
			__m128 result2 = _mm_cmpgt_ps(otherMax_, min_);

			return _mm_movemask_ps(result1) == 15 && _mm_movemask_ps(result2) == 15;*/


			/*return _mm256_movemask_ps(_mm256_cmp_ps(_mm256_set_ps(max[0], max[1], max[2], max[2], other.max[0], other.max[1], other.max[2], other.max[2]),
									  _mm256_set_ps(other.min[0], other.min[1], other.min[2], other.min[2], min[0], min[1], min[2], min[2]), _CMP_GT_OS)) == 255;*/
		}

		inline bool FEAABB::AABBContain(FEAABB& Other)
		{
			if (Min[0] > Other.Min[0] || Max[0] < Other.Max[0]) return false;
			if (Min[1] > Other.Min[1] || Max[1] < Other.Max[1]) return false;
			if (Min[2] > Other.Min[2] || Max[2] < Other.Max[2]) return false;
			return true;

			/*return _mm256_movemask_ps(_mm256_cmp_ps(_mm256_set_ps(other.min[0], other.min[1], other.min[2], other.min[2], max[0], max[1], max[2], max[2]),
									  _mm256_set_ps(min[0], min[1], min[2], min[2], other.max[0], other.max[1], other.max[2], other.max[2]), _CMP_GT_OS)) == 255;*/
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

			Result.Size = abs(Result.Max.x - Result.Min.x);
			if (abs(Result.Max.y - Result.Min.y) > Result.Size)
				Result.Size = abs(Result.Max.y - Result.Min.y);

			if (abs(Result.Max.z - Result.Min.z) > Result.Size)
				Result.Size = abs(Result.Max.z - Result.Min.z);

			return Result;
		}

		FEAABB FEAABB::Merge(FEAABB& Other)
		{
			if (this->Size == 0)
				return Other;

			FEAABB result;

			result.Min[0] = Min[0] < Other.Min[0] ? Min[0] : Other.Min[0];
			result.Min[1] = Min[1] < Other.Min[1] ? Min[1] : Other.Min[1];
			result.Min[2] = Min[2] < Other.Min[2] ? Min[2] : Other.Min[2];

			result.Max[0] = Max[0] > Other.Max[0] ? Max[0] : Other.Max[0];
			result.Max[1] = Max[1] > Other.Max[1] ? Max[1] : Other.Max[1];
			result.Max[2] = Max[2] > Other.Max[2] ? Max[2] : Other.Max[2];

			result.Size = abs(result.Max.x - result.Min.x);
			if (abs(result.Max.y - result.Min.y) > result.Size)
				result.Size = abs(result.Max.y - result.Min.y);

			if (abs(result.Max.z - result.Min.z) > result.Size)
				result.Size = abs(result.Max.z - result.Min.z);

			return result;
		}

		float GetSize();
		glm::vec3 FEAABB::GetCenter();

	private:
		glm::vec3 Min = glm::vec3(0.0f);
		glm::vec3 Max = glm::vec3(0.0f);

		float Size = 0.0f;
	};
}