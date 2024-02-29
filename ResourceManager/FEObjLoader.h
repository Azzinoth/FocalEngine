#pragma once

#include "../SubSystems/FEFileSystem.h"

namespace FocalEngine
{
#define FE_OBJ_DOUBLE_VERTEX_ON_SEAMS

	struct MaterialRecord
	{
		std::string Name = "";

		std::string AlbedoMapFile = "";
		std::string SpecularMapFile = "";
		std::string SpecularHighlightMapFile = "";
		std::string AlphaMapFile = "";
		std::string NormalMapFile = "";
		std::string BumpMapFile = "";
		std::string DisplacementMapFile = "";
		std::string StencilDecalMapFile = "";

		unsigned int MinVertexIndex = INT_MAX;
		unsigned int MaxVertexIndex = 0;

		unsigned int MinTextureIndex = INT_MAX;
		unsigned int MaxTextureIndex = 0;

		unsigned int MinNormalIndex = INT_MAX;
		unsigned int MaxNormalIndex = 0;

		unsigned int FacesSeenBefore = 0;
		unsigned int FaceCount = 0;
	};

	struct FERawOBJData
	{
		std::vector<glm::vec3> RawVertexCoordinates;
		std::vector<glm::vec3> RawVertexColors;
		std::vector<glm::vec2> RawTextureCoordinates;
		std::vector<glm::vec3> RawNormalCoordinates;
		std::vector<int> RawIndices;

		// final vertex coordinates
		std::vector<float> FVerC;
		// final texture coordinates
		std::vector<float> FTexC;
		// final normal coordinates
		std::vector<float> FNorC;
		// final tangent coordinates
		std::vector<float> FTanC;
		// final indices
		std::vector<int> FInd;
		// material records
		std::vector<MaterialRecord> MaterialRecords;
		std::vector<float> MatIDs;
	};

	class FEResourceManager;

	class FEObjLoader
	{
		friend FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEObjLoader)
		
		void ForceOneMesh(bool bForce);
		bool IsForcingOneMesh();

		void ForcePositionNormalization(bool bForce);
		bool IsForcingPositionNormalization();

		void ReadFile(const char* FileName);

		// Use to get raw data from the loaded file.
		// Recommenede only if you know what you are doing.
		std::vector<FERawOBJData*>* GetLoadedObjects();
	private:
		SINGLETON_PRIVATE_PART(FEObjLoader)
		
		std::vector<FERawOBJData*> LoadedObjects;

		bool bForceOneMesh = false;
		bool bForcePositionNormalization = false;
		bool bHaveColors = false;
		bool bHaveTextureCoord = false;
		bool bHaveNormalCoord = false;

		std::string CurrentFilePath = "";
		std::string MaterialFileName = "";
		FERawOBJData* CurrentMaterialObject = nullptr;
		void ReadMaterialFile(const char* OriginalOBJFile);
		void ReadMaterialLine(std::stringstream& LineStream);

		void ReadLine(std::stringstream& LineStream, FERawOBJData* Data);
		void ProcessRawData(FERawOBJData* Data);

		glm::vec3 CalculateNormal(glm::dvec3 V0, glm::dvec3 V1, glm::dvec3 V2);
		void CalculateNormals(FERawOBJData* Data);

		glm::vec3 CalculateTangent(glm::vec3 V0, glm::vec3 V1, glm::vec3 V2, std::vector<glm::vec2>&& Textures);
		void CalculateTangents(FERawOBJData* Data);

		void NormalizeVertexPositions(FERawOBJData* data);
		
		bool CheckCurrentMaterialObject();

#ifdef FE_OBJ_DOUBLE_VERTEX_ON_SEAMS
		struct VertexThatNeedDoubling
		{
			VertexThatNeedDoubling(const int IndexInArray, const int AcctualIndex, const int TexIndex, const int NormIndex) : IndexInArray(IndexInArray),
			                                                                                                                  AcctualIndex(AcctualIndex), TexIndex(TexIndex), NormIndex(NormIndex), bWasDone(false) {};

			int IndexInArray;
			int AcctualIndex;
			int TexIndex;
			int NormIndex;
			bool bWasDone;

			friend bool operator==(const VertexThatNeedDoubling& Lhs, const VertexThatNeedDoubling& Rhs)
			{
				return Lhs.AcctualIndex == Rhs.AcctualIndex && Lhs.IndexInArray == Rhs.IndexInArray && Lhs.TexIndex == Rhs.TexIndex && Lhs.NormIndex == Rhs.NormIndex;
			}
		};
#endif // FE_OBJ_DOUBLE_VERTEX_ON_SEAMS
	};

	#define LOG FELOG::getInstance()
}