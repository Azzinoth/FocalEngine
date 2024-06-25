#pragma once

#include "../Core/FECoreIncludes.h"
#include <glm/gtx/matrix_decompose.hpp>

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
		FEAABB(glm::dvec3 Min, glm::dvec3 Max);
		FEAABB(std::vector<glm::vec3>& VertexPositions);
		FEAABB(std::vector<glm::dvec3>& VertexPositions);
		FEAABB(std::vector<float>& VertexPositions);
		FEAABB(std::vector<double>& VertexPositions);
		FEAABB(float* VertexPositions, int VertexCount);
		FEAABB(double* VertexPositions, int VertexCount);
		// only for uniform sized AABB
		FEAABB(glm::vec3 Center, float Size);
		FEAABB(FEAABB Other, glm::mat4 TransformMatrix);
		~FEAABB();

		glm::vec3 GetMin();
		glm::vec3 GetMax();

		bool RayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& Distance);
		bool RayIntersect(const glm::dvec3& RayOrigin, const glm::dvec3& RayDirection, std::vector<glm::dvec3>& HitPoints);
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

		FEAABB FEAABB::GetIntersectionAABB(FEAABB& Other);
		float FEAABB::GetVolume();

		inline bool FEAABB::AABBContain(FEAABB& Other)
		{
			if (Min[0] > Other.Min[0] || Max[0] < Other.Max[0]) return false;
			if (Min[1] > Other.Min[1] || Max[1] < Other.Max[1]) return false;
			if (Min[2] > Other.Min[2] || Max[2] < Other.Max[2]) return false;
			return true;

			/*return _mm256_movemask_ps(_mm256_cmp_ps(_mm256_set_ps(other.min[0], other.min[1], other.min[2], other.min[2], max[0], max[1], max[2], max[2]),
									  _mm256_set_ps(min[0], min[1], min[2], min[2], other.max[0], other.max[1], other.max[2], other.max[2]), _CMP_GT_OS)) == 255;*/
		}

		FEAABB FEAABB::Transform(const glm::mat4 TransformMatrix);
		FEAABB FEAABB::Merge(FEAABB& Other);

		glm::vec3 FEAABB::GetCenter();
		glm::vec3 FEAABB::GetSize();

		float GetLongestAxisLength();
		bool ContainsPoint(const glm::vec3& Point) const;
	private:
		glm::vec3 Min = glm::vec3(0.0f);
		glm::vec3 Max = glm::vec3(0.0f);

		glm::vec3 Size = glm::vec3(0.0f);
		float LongestAxisLength = 0.0f;
	};

	class FEGeometry
	{
	public:
		SINGLETON_PUBLIC_PART(FEGeometry)

		bool DecomposeMatrixToTranslationRotationScale(const glm::dmat4& Matrix, glm::dvec3& OutTranslation, glm::dquat& OutRotationQuaternion, glm::dvec3& OutScale);

		glm::vec3 CalculateNormal(glm::dvec3 FirstVertex, glm::dvec3 SecondVertex, glm::dvec3 ThirdVertex);
		void CalculateNormals(const std::vector<int>& Indices, const std::vector<float>& Vertices, std::vector<float>& NormalsToFill);
		void CalculateNormals(const std::vector<int>& Indices, const std::vector<double>& Vertices, std::vector<float>& NormalsToFill);

		glm::vec3 CalculateTangent(const glm::vec3 FirstVertex, const glm::vec3 SecondVertex, const glm::vec3 ThirdVertex, std::vector<glm::vec2>&& TextureCoordinates);
		void CalculateTangents(const std::vector<int>& Indices, const std::vector<float>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill);
		void CalculateTangents(const std::vector<int>& Indices, const std::vector<double>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill);

		bool RaysIntersection(const glm::vec3& FirstRayOrigin, const glm::vec3& FirstRayDirection, const glm::vec3& SecondRayOrigin, const glm::vec3& SecondRayDirection, float& FirstRayParametricIntersection, float& SecondRayParametricIntersection) const;

		bool IsRayIntersectingTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& TriangleVertices, float& Distance, glm::vec3* HitPoint = nullptr, float* U = nullptr, float* V = nullptr);
		bool IsRayIntersectingTriangle(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, std::vector<glm::dvec3>& TriangleVertices, double& Distance, glm::dvec3* HitPoint = nullptr, double* U = nullptr, double* V = nullptr);

		float CalculateTriangleArea(std::vector<glm::vec3>& TriangleVertices);
		double CalculateTriangleArea(std::vector<glm::dvec3>& TriangleVertices);
		float CalculateTriangleArea(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 PointC);
		double CalculateTriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		bool IsAABBIntersectTriangle(FEAABB& AABB, std::vector<glm::vec3>& TriangleVertices);
		bool IsAABBIntersectTriangle(FEAABB& AABB, std::vector<glm::dvec3>& TriangleVertices);

		std::vector<glm::dvec3> GetIntersectionPoints(FEAABB& AABB, std::vector<glm::dvec3> TriangleVertices);
	private:
		SINGLETON_PRIVATE_PART(FEGeometry)
	};

#define GEOMETRY FEGeometry::getInstance()
}