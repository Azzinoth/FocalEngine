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

		
	private:
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);

		float size = 0.0f;
	};
}