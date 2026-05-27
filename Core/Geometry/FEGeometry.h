#pragma once

#include "FEFrustum.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEGeometry
	{
	public:
		SINGLETON_PUBLIC_PART(FEGeometry)

		bool IsEpsilonEqual(const glm::dvec3& FirstVector, const glm::dvec3& SecondVector, double Epsilon = 1e-5);
		bool IsEpsilonEqual(const glm::dquat& FirstQuaternion, const glm::dquat& SecondQuaternion, double Epsilon = 1e-5);
		bool IsEpsilonEqual(const glm::dmat4& FirstMatrix, const glm::dmat4& SecondMatrix, double Epsilon = 1e-5);

		bool DecomposeMatrixToTranslationRotationScale(const glm::dmat4& Matrix, glm::dvec3& OutTranslation, glm::dquat& OutRotationQuaternion, glm::dvec3& OutScale);

		glm::vec3 CalculateNormal(glm::dvec3 FirstVertex, glm::dvec3 SecondVertex, glm::dvec3 ThirdVertex);
		void CalculateNormals(const std::vector<int>& Indices, const std::vector<float>& Vertices, std::vector<float>& NormalsToFill);
		void CalculateNormals(const std::vector<int>& Indices, const std::vector<double>& Vertices, std::vector<float>& NormalsToFill);

		glm::vec3 CalculateTangent(const glm::vec3 FirstVertex, const glm::vec3 SecondVertex, const glm::vec3 ThirdVertex, std::vector<glm::vec2>&& TextureCoordinates);
		void CalculateTangents(const std::vector<int>& Indices, const std::vector<float>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill);
		void CalculateTangents(const std::vector<int>& Indices, const std::vector<double>& Vertices, const std::vector<float>& TextureCoordinates, const std::vector<float>& Normals, std::vector<float>& TangentsToFill);

		glm::dvec3 CreateMouseRayToWorld(const double MouseScreenX, const double MouseScreenY, const glm::dmat4 ViewMatrix, const glm::dmat4 ProjectionMatrix, const glm::ivec2 ViewportPosition, const glm::ivec2 ViewportSize) const;
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

		std::vector<FELine> GetAABBEdges(const FEAABB& AABB);
	private:
		SINGLETON_PRIVATE_PART(FEGeometry)
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetGeometry();
	#define GEOMETRY (*static_cast<FEGeometry*>(GetGeometry()))
#else
	#define GEOMETRY FEGeometry::GetInstance()
#endif
}