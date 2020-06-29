#pragma once

#include "FELog.h"

namespace FocalEngine
{
	class FEEntity;
	class FEResourceManager;

	class FEAABB
	{
		friend FEEntity;
		friend FEResourceManager;
	public:
		FEAABB();
		FEAABB(glm::vec3 Min, glm::vec3 Max);
		FEAABB(std::vector<glm::vec3>& VertexPositions);
		FEAABB(std::vector<float>& VertexPositions);
		~FEAABB();

		glm::vec3 getMin();
		glm::vec3 getMax();

		bool rayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& distance);
	private:
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);
	};
}