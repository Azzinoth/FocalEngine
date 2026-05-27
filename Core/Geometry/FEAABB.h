#pragma once

#include "FEPlane.h"

namespace FocalEngine
{
	class FEAABB
	{
		friend class FEEntity;
		friend class FEResourceManager;
		friend class FEScene;
	public:
		FEAABB();
		FEAABB(glm::vec3 Min, glm::vec3 Max);
		FEAABB(glm::dvec3 Min, glm::dvec3 Max);
		FEAABB(std::vector<glm::vec3>& Positions);
		FEAABB(std::vector<glm::dvec3>& Positions);
		FEAABB(std::vector<glm::vec2>& Positions);
		FEAABB(std::vector<glm::dvec2>& Positions);
		FEAABB(std::vector<float>& Positions);
		FEAABB(std::vector<double>& Positions);
		FEAABB(float* Positions, int Count);
		FEAABB(double* Positions, int Count);
		FEAABB(glm::vec3 Center, float Size);
		FEAABB(FEAABB Other, glm::mat4 TransformMatrix);
		~FEAABB();

		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;
		
		bool RayIntersect(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, float& Distance);
		bool RayIntersect(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, std::vector<glm::vec3>& HitPoints);
		bool RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, double& Distance);
		bool RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, std::vector<glm::dvec3>& HitPoints);
		inline bool AABBIntersect(FEAABB Other)
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

		FEAABB GetIntersectionAABB(FEAABB& Other);
		float GetVolume();

		inline bool AABBContain(FEAABB& Other)
		{
			if (Min[0] > Other.Min[0] || Max[0] < Other.Max[0]) return false;
			if (Min[1] > Other.Min[1] || Max[1] < Other.Max[1]) return false;
			if (Min[2] > Other.Min[2] || Max[2] < Other.Max[2]) return false;
			return true;

			/*return _mm256_movemask_ps(_mm256_cmp_ps(_mm256_set_ps(other.min[0], other.min[1], other.min[2], other.min[2], max[0], max[1], max[2], max[2]),
									  _mm256_set_ps(min[0], min[1], min[2], min[2], other.max[0], other.max[1], other.max[2], other.max[2]), _CMP_GT_OS)) == 255;*/
		}

		FEAABB Transform(const glm::mat4 TransformMatrix);
		FEAABB Merge(FEAABB& Other);

		glm::vec3 GetCenter();
		glm::vec3 GetSize();

		float GetLongestAxisLength();
		bool ContainsPoint(const glm::vec3& Point) const;

		glm::vec3 GetApproximateForwardDirection() const;

		std::vector<glm::vec3> GetCorners() const;
	private:
		template<typename T>
		bool RayIntersectInternal(const T& RayOrigin, const T& RayDirection, typename T::value_type& HitMin, typename T::value_type& HitMax);

		glm::vec3 Min = glm::vec3(0.0f);
		glm::vec3 Max = glm::vec3(0.0f);

		glm::vec3 Size = glm::vec3(0.0f);
		float LongestAxisLength = 0.0f;
	};
}