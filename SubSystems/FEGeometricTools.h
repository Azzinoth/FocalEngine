#pragma once

#include "FELog.h"

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
		FEAABB(glm::vec3 center, float size);
		FEAABB(FEAABB other, glm::mat4 transformMatrix);
		~FEAABB();

		glm::vec3 getMin();
		glm::vec3 getMax();

		bool rayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& distance);
		inline bool FEAABB::AABBIntersect(FEAABB other)
		{
			if (max[0] < other.min[0] || min[0] > other.max[0]) return false;
			if (max[1] < other.min[1] || min[1] > other.max[1]) return false;
			if (max[2] < other.min[2] || min[2] > other.max[2]) return false;
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

		inline bool FEAABB::AABBContain(FEAABB& other)
		{
			if (min[0] > other.min[0] || max[0] < other.max[0]) return false;
			if (min[1] > other.min[1] || max[1] < other.max[1]) return false;
			if (min[2] > other.min[2] || max[2] < other.max[2]) return false;
			return true;

			/*return _mm256_movemask_ps(_mm256_cmp_ps(_mm256_set_ps(other.min[0], other.min[1], other.min[2], other.min[2], max[0], max[1], max[2], max[2]),
									  _mm256_set_ps(min[0], min[1], min[2], min[2], other.max[0], other.max[1], other.max[2], other.max[2]), _CMP_GT_OS)) == 255;*/
		}

		FEAABB FEAABB::transform(glm::mat4 transformMatrix)
		{
			FEAABB result;

			// firstly we generate 8 points that represent AABBCube.
			// bottom 4 points
			glm::vec4 bottomLeftFront = glm::vec4(min.x, min.y, max.z, 1.0f);
			glm::vec4 bottomRightFront = glm::vec4(max.x, min.y, max.z, 1.0f);
			glm::vec4 bottomRightBack = glm::vec4(max.x, min.y, min.z, 1.0f);
			glm::vec4 bottomLeftBack = glm::vec4(min.x, min.y, min.z, 1.0f);
			// top 4 points
			glm::vec4 topLeftFront = glm::vec4(min.x, max.y, max.z, 1.0f);
			glm::vec4 topRightFront = glm::vec4(max.x, max.y, max.z, 1.0f);
			glm::vec4 topRightBack = glm::vec4(max.x, max.y, min.z, 1.0f);
			glm::vec4 topLeftBack = glm::vec4(min.x, max.y, min.z, 1.0f);

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

			result.min = glm::vec3(FLT_MAX);
			result.max = glm::vec3(-FLT_MAX);
			for (auto point : allPoints)
			{
				if (point.x < result.min.x)
					result.min.x = point.x;

				if (point.x > result.max.x)
					result.max.x = point.x;

				if (point.y < result.min.y)
					result.min.y = point.y;

				if (point.y > result.max.y)
					result.max.y = point.y;

				if (point.z < result.min.z)
					result.min.z = point.z;

				if (point.z > result.max.z)
					result.max.z = point.z;
			}

			result.size = abs(result.max.x - result.min.x);
			if (abs(result.max.y - result.min.y) > result.size)
				result.size = abs(result.max.y - result.min.y);

			if (abs(result.max.z - result.min.z) > result.size)
				result.size = abs(result.max.z - result.min.z);

			return result;
		}

		FEAABB FEAABB::merge(FEAABB& other)
		{
			if (this->size == 0)
				return other;

			FEAABB result;

			result.min[0] = min[0] < other.min[0] ? min[0] : other.min[0];
			result.min[1] = min[1] < other.min[1] ? min[1] : other.min[1];
			result.min[2] = min[2] < other.min[2] ? min[2] : other.min[2];

			result.max[0] = max[0] > other.max[0] ? max[0] : other.max[0];
			result.max[1] = max[1] > other.max[1] ? max[1] : other.max[1];
			result.max[2] = max[2] > other.max[2] ? max[2] : other.max[2];

			result.size = abs(result.max.x - result.min.x);
			if (abs(result.max.y - result.min.y) > result.size)
				result.size = abs(result.max.y - result.min.y);

			if (abs(result.max.z - result.min.z) > result.size)
				result.size = abs(result.max.z - result.min.z);

			return result;
		}
		
	private:
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);

		float size = 0.0f;
	};
}