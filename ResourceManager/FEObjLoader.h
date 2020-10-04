#pragma once

#include "../SubSystems/FELog.h"

namespace FocalEngine
{
	struct materialRecord
	{
		std::string name;
		int firstFace = -1;
		int lastFace = -1;
	};

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
		int maxIndex = -1;

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
		// material records
		std::vector<materialRecord> materialRecords;
		std::vector<float> matIDs;

		void readFile(const char* fileName);

		void readLine(std::stringstream& lineStream);
		void processRawData();

		glm::vec3 calculateTangent(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::vector<glm::vec2>&& textures);
		void calculateTangents();

		struct vertexThatNeedDoubling
		{
			vertexThatNeedDoubling(int IndexInArray, int AcctualIndex, int TexIndex, int NormIndex) : indexInArray(IndexInArray),
				                   acctualIndex(AcctualIndex), texIndex(TexIndex), normIndex(NormIndex), wasDone(false) {};

			int indexInArray;
			int acctualIndex;
			int texIndex;
			int normIndex;
			bool wasDone;

			friend bool operator==(const vertexThatNeedDoubling& lhs, const vertexThatNeedDoubling& rhs)
			{
				return lhs.acctualIndex == rhs.acctualIndex && lhs.indexInArray == rhs.indexInArray && lhs.texIndex == rhs.texIndex && lhs.normIndex == rhs.normIndex;
			}
		};
	};

	#define LOG FELOG::getInstance()
}