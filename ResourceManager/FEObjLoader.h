#pragma once

#include "../SubSystems/FECoreIncludes.h"

namespace FocalEngine
{
	class FEResourceManager;

	class FEObjLoader
	{
		friend FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEObjLoader)
	private:
		SINGLETON_PRIVATE_PART(FEObjLoader)

		std::vector<glm::vec3> rawVertexCoordinates;
		std::vector<glm::vec2> rawTextureCoordinates;
		std::vector<glm::vec3> rawNormalCoordinates;
		std::vector<int> rawIndices;

		// final vertex coordinates
		std::vector<float> fVerC;
		// final texture coordinates
		std::vector<float> fTexC;
		// final normal coordinates
		std::vector<float> fNorC;
		// final tangent coordinates
		std::vector<float> fTanC;
		// final indices
		std::vector<int> fInd;

		void readFile(const char* fileName);

		void readLine(std::stringstream& lineStream);
		void processRawData();

		glm::vec3 calculateTangent(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::vector<glm::vec2>&& textures);
		void calculateTangents();
	};
}