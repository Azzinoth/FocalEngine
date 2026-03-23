#include "FEObjLoader.h"
using namespace FocalEngine;

FEObjLoader::FEObjLoader()
{
	
}

FEObjLoader::~FEObjLoader()
{

}

void FEObjLoader::ReadLine(std::stringstream& LineStream, FERawOBJData* Data)
{
	std::string CurrentLine;

	LineStream >> CurrentLine;
	// To lower case
	std::transform(CurrentLine.begin(), CurrentLine.end(), CurrentLine.begin(), [](const unsigned char C) { return std::tolower(C); });

	// if it is comment or object declaration or other not relevant info
	if (CurrentLine[0] == '#' || CurrentLine[0] == 'o')
	{
		// get to the next line
		return;
	}

	// if this line contains vertex coordinates
	if (CurrentLine[0] == 'v' && CurrentLine.size() == 1)
	{
		glm::dvec3 NewVec;
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> CurrentLine;
			NewVec[i] = std::stod(CurrentLine);
		}

		if (bUseDoublePrecisionForReadingCoordinates)
		{
			Data->RawVertexCoordinatesDoublePrecision.push_back(NewVec);
		}
		else
		{
			Data->RawVertexCoordinates.push_back(glm::vec3(NewVec));
		}

		// File could contain RGB.
		if (!LineStream.eof())
		{
			for (int i = 0; i <= 2; i++)
			{
				if (!(LineStream >> CurrentLine) || CurrentLine.empty())
					break;

				NewVec[i] = std::stof(CurrentLine);
				bHaveColors = true;
			}

			Data->RawVertexColors.push_back(NewVec);
		}
	}
	// if this line contains vertex texture coordinates
	else if (CurrentLine[0] == 'v' && CurrentLine.size() == 2 && CurrentLine[1] == 't')
	{
		bHaveTextureCoord = true;

		glm::vec2 NewVec;
		for (int i = 0; i <= 1; i++)
		{
			LineStream >> CurrentLine;
			NewVec[i] = std::stof(CurrentLine);
		}

		Data->RawTextureCoordinates.push_back(NewVec);
	}
	// if this line contains vertex texture coordinates
	else if (CurrentLine[0] == 'v' && CurrentLine.size() == 2 && CurrentLine[1] == 'n')
	{
		bHaveNormalCoord = true;

		glm::vec3 NewVector;
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> CurrentLine;
			NewVector[i] = std::stof(CurrentLine);
		}
		
		glm::vec3 NormalizedVector = glm::normalize(NewVector);

		if (isnan(NormalizedVector.x) || isnan(NormalizedVector.y) || isnan(NormalizedVector.z))
			NormalizedVector = glm::vec3(0.0f);

		Data->RawNormalCoordinates.push_back(NormalizedVector);
	}
	// if this line contains indices
	else if (CurrentLine[0] == 'f' && CurrentLine.size() == 1)
	{
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> CurrentLine;

			std::stringstream TemporaryLineStream;
			TemporaryLineStream << CurrentLine;

			int Iterations = 0;
			while (std::getline(TemporaryLineStream, CurrentLine, '/'))
			{
				if (!CurrentLine.empty())
				{
					Data->RawIndices.push_back(std::stoi(CurrentLine));
				}
				else
				{
					// Texture coordinates are optional.
					if (bHaveTextureCoord && Iterations == 1)
					{
						// It is not proper fix!
						Data->RawIndices.push_back(1);
					}
					else
					{
						LOG.Add(std::string("Texture coordinates were absent in face description in function FEObjLoader::readFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
					}
				}

				if (!Data->MaterialRecords.empty())
				{
					if (Iterations == 0)
					{
						if (Data->MaterialRecords.back().MinVertexIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinVertexIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxVertexIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxVertexIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}
					else if (Iterations == 1)
					{
						if (Data->MaterialRecords.back().MinTextureIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinTextureIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxTextureIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxTextureIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}
					else if (Iterations == 2)
					{
						if (Data->MaterialRecords.back().MinNormalIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinNormalIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxNormalIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxNormalIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}

					Data->MaterialRecords.back().FaceCount++;
				}
				
				Iterations++;
			}
		}
	}
	// if this line contains new material declaration
	else if (CurrentLine.find("usemtl") != std::string::npos)
	{
		Data->MaterialRecords.push_back(MaterialRecord());
		if (Data->MaterialRecords.size() > 1)
			Data->MaterialRecords.back().FacesSeenBefore = Data->MaterialRecords[Data->MaterialRecords.size() - 2].FacesSeenBefore + Data->MaterialRecords[Data->MaterialRecords.size() - 2].FaceCount;
		
		LineStream >> Data->MaterialRecords.back().Name;
	}
	// file with materials data
	else if (CurrentLine.find("mtllib") != std::string::npos)
	{
		LineStream >> MaterialFileName;
	}
}

void FEObjLoader::ReadFile(const char* FileName)
{
	bHaveColors = false;
	bHaveTextureCoord = false;
	bHaveNormalCoord = false;
	CurrentFilePath = FileName;
	MaterialFileName = "";
	CurrentMaterialObject = nullptr;
	for (size_t i = 0; i < LoadedObjects.size(); i++)
	{
		delete LoadedObjects[i];
	}
	LoadedObjects.clear();
	LoadedObjects.push_back(new FERawOBJData());

	if (FileName == nullptr)
	{
		LOG.Add(std::string("No file name in function FEObjLoader::readFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::ifstream File(FileName, std::ios::binary);
	const auto Begin = File.tellg();
	File.seekg(0, std::ios::end);
	const auto End = File.tellg();
	const auto FileSize = static_cast<size_t>(End - Begin);

	File.seekg(0, 0);

	std::string CurrentLine;
	for (size_t i = 0; i < FileSize; i++)
	{
		char NewChar;
		File.read(&NewChar, 1);
		CurrentLine += NewChar;
			
		if (NewChar == '\n')
		{
			CurrentLine.erase(CurrentLine.end() - 1, CurrentLine.end());
			ReadLine(std::stringstream(CurrentLine), LoadedObjects.back());

			CurrentLine = "";
		}
	}

	if (!bForceOneMesh)
	{
		// Each material should be represented by different FERawOBJData
		std::vector<FERawOBJData*> ObjectsPerMaterialList;
		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			for (size_t j = 0; j < LoadedObjects[i]->MaterialRecords.size(); j++)
			{
				FERawOBJData* TempObject = new FERawOBJData();
				TempObject->MaterialRecords.push_back(MaterialRecord(LoadedObjects[i]->MaterialRecords[j]));

				size_t StartIndex = LoadedObjects[i]->MaterialRecords[j].MinVertexIndex - 1;
				size_t EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxVertexIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawVertexCoordinates.push_back(LoadedObjects[i]->RawVertexCoordinates[k]);
				}

				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawVertexCoordinatesDoublePrecision.push_back(LoadedObjects[i]->RawVertexCoordinatesDoublePrecision[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].MinTextureIndex - 1;
				EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxTextureIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawTextureCoordinates.push_back(LoadedObjects[i]->RawTextureCoordinates[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].MinNormalIndex - 1;
				EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxNormalIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawNormalCoordinates.push_back(LoadedObjects[i]->RawNormalCoordinates[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].FacesSeenBefore;
				EndIndex = StartIndex + LoadedObjects[i]->MaterialRecords[j].FaceCount;
				for (size_t k = StartIndex; k < EndIndex; k += 3)
				{
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k] - LoadedObjects[i]->MaterialRecords[j].MinVertexIndex + 1);
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k + 1] - LoadedObjects[i]->MaterialRecords[j].MinTextureIndex + 1);
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k + 2] - LoadedObjects[i]->MaterialRecords[j].MinNormalIndex + 1);
				}

				ObjectsPerMaterialList.push_back(TempObject);
			}

			if (LoadedObjects[i]->MaterialRecords.empty())
				ObjectsPerMaterialList.push_back(new FERawOBJData(*LoadedObjects[i]));
		}

		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			delete LoadedObjects[i];
		}
		LoadedObjects.clear();
		LoadedObjects = ObjectsPerMaterialList;
	}

	ReadMaterialFile(FileName);

	for (size_t i = 0; i < LoadedObjects.size(); i++)
	{
		ProcessRawData(LoadedObjects[i]);
	}
}

void FEObjLoader::CalculateNormals(FERawOBJData* Data)
{
	std::vector<glm::dvec3> TrianglePoints;
	TrianglePoints.resize(3);

	std::vector<int> Indices;
	for (size_t i = 0; i < Data->FInd.size(); i++)
	{
		Indices.push_back(Data->FInd[i]);
	}

	if (bUseDoublePrecisionForReadingCoordinates)
	{
		GEOMETRY.CalculateNormals(Data->FInd, Data->DVerC, Data->FNorC);
	}
	else
	{
		std::vector<double> DoubleVertices;
		for (size_t i = 0; i < Data->FVerC.size(); i++)
		{
			DoubleVertices.push_back(Data->FVerC[i]);
		}

		GEOMETRY.CalculateNormals(Data->FInd, DoubleVertices, Data->FNorC);
	}
}

void FEObjLoader::CalculateTangents(FERawOBJData* Data)
{
	std::vector<int> Indices;
	for (size_t i = 0; i < Data->FInd.size(); i++)
	{
		Indices.push_back(Data->FInd[i]);
	}

	if (bUseDoublePrecisionForReadingCoordinates)
	{
		GEOMETRY.CalculateTangents(Data->FInd, Data->DVerC, Data->FTexC, Data->FNorC, Data->FTanC);
	}
	else
	{
		std::vector<double> DoubleVertices;
		for (size_t i = 0; i < Data->FVerC.size(); i++)
		{
			DoubleVertices.push_back(Data->FVerC[i]);
		}

		GEOMETRY.CalculateTangents(Data->FInd, DoubleVertices, Data->FTexC, Data->FNorC, Data->FTanC);
	}
}

void FEObjLoader::NormalizeVertexPositions(FERawOBJData* Data)
{
	float MinX = std::numeric_limits<float>::max();
	float MaxX = -std::numeric_limits<float>::max();
	float MinY = std::numeric_limits<float>::max();
	float MaxY = -std::numeric_limits<float>::max();
	float MinZ = std::numeric_limits<float>::max();
	float MaxZ = -std::numeric_limits<float>::max();

	for (size_t i = 0; i < Data->RawVertexCoordinates.size(); i++)
	{
		if (MinX > Data->RawVertexCoordinates[i].x)
			MinX = Data->RawVertexCoordinates[i].x;

		if (MaxX < Data->RawVertexCoordinates[i].x)
			MaxX = Data->RawVertexCoordinates[i].x;

		if (MinY > Data->RawVertexCoordinates[i].y)
			MinY = Data->RawVertexCoordinates[i].y;

		if (MaxY < Data->RawVertexCoordinates[i].y)
			MaxY = Data->RawVertexCoordinates[i].y;

		if (MinZ > Data->RawVertexCoordinates[i].z)
			MinZ = Data->RawVertexCoordinates[i].z;

		if (MaxZ < Data->RawVertexCoordinates[i].z)
			MaxZ = Data->RawVertexCoordinates[i].z;
	}

	float RangeX = abs(MaxX - MinX);
	float RangeY = abs(MaxY - MinY);
	float RangeZ = abs(MaxZ - MinZ);

	float MinRange = std::min(std::min(RangeX, RangeY), RangeZ);
	float ScaleFactor = 1.0f;

	if (MinRange < 1.0f)
	{
		ScaleFactor = 1.0f / MinRange;
	}

	glm::vec3 ShiftToApply(-MinX, -MinY, -MinZ);
	for (size_t i = 0; i < Data->RawVertexCoordinates.size(); i++)
	{
		Data->RawVertexCoordinates[i] -= ShiftToApply;
		Data->RawVertexCoordinates[i] *= ScaleFactor;
	}

	LastAppliedShift = ShiftToApply;
}

void FEObjLoader::NormalizeVertexPositionsDoublePrecision(FERawOBJData* Data)
{
	double MinX = std::numeric_limits<double>::max();
	double MaxX = -std::numeric_limits<double>::max();
	double MinY = std::numeric_limits<double>::max();
	double MaxY = -std::numeric_limits<double>::max();
	double MinZ = std::numeric_limits<double>::max();
	double MaxZ = -std::numeric_limits<double>::max();

	for (size_t i = 0; i < Data->RawVertexCoordinatesDoublePrecision.size(); i++)
	{
		if (MinX > Data->RawVertexCoordinatesDoublePrecision[i].x)
			MinX = Data->RawVertexCoordinatesDoublePrecision[i].x;

		if (MaxX < Data->RawVertexCoordinatesDoublePrecision[i].x)
			MaxX = Data->RawVertexCoordinatesDoublePrecision[i].x;

		if (MinY > Data->RawVertexCoordinatesDoublePrecision[i].y)
			MinY = Data->RawVertexCoordinatesDoublePrecision[i].y;

		if (MaxY < Data->RawVertexCoordinatesDoublePrecision[i].y)
			MaxY = Data->RawVertexCoordinatesDoublePrecision[i].y;

		if (MinZ > Data->RawVertexCoordinatesDoublePrecision[i].z)
			MinZ = Data->RawVertexCoordinatesDoublePrecision[i].z;

		if (MaxZ < Data->RawVertexCoordinatesDoublePrecision[i].z)
			MaxZ = Data->RawVertexCoordinatesDoublePrecision[i].z;
	}

	double RangeX = abs(MaxX - MinX);
	double RangeY = abs(MaxY - MinY);
	double RangeZ = abs(MaxZ - MinZ);

	double MinRange = std::min(std::min(RangeX, RangeY), RangeZ);
	double ScaleFactor = 1.0;

	if (MinRange < 1.0)
	{
		ScaleFactor = 1.0 / MinRange;
	}

	glm::dvec3 ShiftToApply(-MinX, -MinY, -MinZ);
	for (size_t i = 0; i < Data->RawVertexCoordinatesDoublePrecision.size(); i++)
	{
		Data->RawVertexCoordinatesDoublePrecision[i] -= ShiftToApply;
		Data->RawVertexCoordinatesDoublePrecision[i] *= ScaleFactor;
	}

	LastAppliedShift = ShiftToApply;
}

void FEObjLoader::ProcessRawData(FERawOBJData* Data)
{
	if (bForcePositionNormalization)
	{
		if (bUseDoublePrecisionForReadingCoordinates)
		{
			NormalizeVertexPositionsDoublePrecision(Data);
		}
		else
		{
			NormalizeVertexPositions(Data);
		}
	}

	// Convert normalized vertex coordinates from double to float
	if (bUseDoublePrecisionForReadingCoordinates)
	{
		Data->RawVertexCoordinates.resize(Data->RawVertexCoordinatesDoublePrecision.size());
		for (size_t i = 0; i < Data->RawVertexCoordinatesDoublePrecision.size(); i++)
		{
			Data->RawVertexCoordinates[i] = glm::vec3(Data->RawVertexCoordinatesDoublePrecision[i]);
		}
	}

	if (bHaveTextureCoord && bHaveNormalCoord)
	{
		std::vector<std::pair<int, float>> DoubledVertexMatIndices;
		if (bDoubleVertexOnSeams)
		{
			std::vector<FEObjLoader::VertexThatNeedDoubling> VertexList;
			std::unordered_map<int, int> IndexesMap;

			for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
			{
				IndexesMap[Data->RawIndices[i]] = static_cast<int>(i);
			}

			for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
			{
				if (IndexesMap.find(Data->RawIndices[i]) != IndexesMap.end())
				{
					size_t j = IndexesMap.find(Data->RawIndices[i])->second;
					std::swap(i, j);

					const bool bTextureCoordRequiresSplit = Data->RawIndices[i + 1] != Data->RawIndices[j + 1];
					const bool bNormalRequiresSplit = Data->RawIndices[i + 2] != Data->RawIndices[j + 2];
					if (Data->RawIndices[i] == Data->RawIndices[j] && (bTextureCoordRequiresSplit || bNormalRequiresSplit))
					{
						// We do not need to add first appearance of vertex that we need to double.
						FEObjLoader::VertexThatNeedDoubling NewVertex = FEObjLoader::VertexThatNeedDoubling(static_cast<int>(j), Data->RawIndices[j], Data->RawIndices[j + 1], Data->RawIndices[j + 2]);
						if (std::find(VertexList.begin(), VertexList.end(), NewVertex) == VertexList.end())
						{
							VertexList.push_back(NewVertex);
						}
					}

					std::swap(i, j);
				}
			}

			for (auto& Vertex : VertexList)
			{
				if (Vertex.bWasDone) continue;

				Data->RawVertexCoordinates.push_back(Data->RawVertexCoordinates[Vertex.ActualIndex - 1]);

				int NewVertexIndex = static_cast<int>(Data->RawVertexCoordinates.size());
				Data->RawIndices[Vertex.IndexInArray] = NewVertexIndex;
				Vertex.bWasDone = true;

				// Preserve matIndex!
				for (size_t i = 0; i < Data->MaterialRecords.size(); i++)
				{
					if (Vertex.ActualIndex >= (static_cast<int>(Data->MaterialRecords[i].MinVertexIndex + 1)) && Vertex.ActualIndex <= (static_cast<int>(Data->MaterialRecords[i].MaxVertexIndex + 1)))
					{
						DoubledVertexMatIndices.push_back(std::make_pair(NewVertexIndex, static_cast<float>(i)));
					}
				}

				for (auto& VertexNext : VertexList)
				{
					if (VertexNext.bWasDone) continue;
					if (Vertex.IndexInArray == VertexNext.IndexInArray && (Vertex.TexIndex == VertexNext.TexIndex || Vertex.NormIndex == VertexNext.NormIndex))
					{
						Data->RawIndices[VertexNext.IndexInArray] = NewVertexIndex;
						VertexNext.bWasDone = true;
					}
				}
			}
		}

		if (bUseDoublePrecisionForReadingCoordinates)
			Data->DVerC.resize(Data->RawVertexCoordinatesDoublePrecision.size() * 3);
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(Data->RawVertexCoordinates.size() * 2);
		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTanC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FInd.resize(0);
		Data->MaterialIDs.resize(Data->RawVertexCoordinates.size());

		for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
		{
			// faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			const int TIndex = Data->RawIndices[i + 1] - 1;
			const int NIndex = Data->RawIndices[i + 2] - 1;

			const int ShiftInVerArr = VIndex * 3;
			const int ShiftInTexArr = VIndex * 2;

			if (bUseDoublePrecisionForReadingCoordinates)
			{
				Data->DVerC[ShiftInVerArr] = Data->RawVertexCoordinatesDoublePrecision[VIndex][0];
				Data->DVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinatesDoublePrecision[VIndex][1];
				Data->DVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinatesDoublePrecision[VIndex][2];
			}

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			// saving material ID in vertex attribute array
			for (size_t j = 0; j < Data->MaterialRecords.size(); j++)
			{
				if (VIndex >= static_cast<int>(Data->MaterialRecords[j].MinVertexIndex - 1) && VIndex <= static_cast<int>(Data->MaterialRecords[j].MaxVertexIndex - 1))
				{
					Data->MaterialIDs[VIndex] = static_cast<float>(j);
				}
			}

			Data->FTexC[ShiftInTexArr] = Data->RawTextureCoordinates[TIndex][0];
			Data->FTexC[ShiftInTexArr + 1] = 1 - Data->RawTextureCoordinates[TIndex][1];

			Data->FNorC[ShiftInVerArr] = Data->RawNormalCoordinates[NIndex][0];
			Data->FNorC[ShiftInVerArr + 1] = Data->RawNormalCoordinates[NIndex][1];
			Data->FNorC[ShiftInVerArr + 2] = Data->RawNormalCoordinates[NIndex][2];
	
			Data->FInd.push_back(VIndex);
		}

		if (bDoubleVertexOnSeams)
		{
			for (size_t j = 0; j < DoubledVertexMatIndices.size(); j++)
			{
				Data->MaterialIDs[DoubledVertexMatIndices[j].first - 1] = DoubledVertexMatIndices[j].second;
			}
		}

		CalculateTangents(Data);
	}
	else if (bHaveTextureCoord && !bHaveNormalCoord)
	{
		if (bUseDoublePrecisionForReadingCoordinates)
			Data->DVerC.resize(Data->RawVertexCoordinatesDoublePrecision.size() * 3);
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(Data->RawVertexCoordinates.size() * 2);
		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTanC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FInd.resize(0);
		Data->MaterialIDs.resize(Data->RawVertexCoordinates.size());

		for (size_t i = 0; i < Data->RawIndices.size(); i+=2)
		{
			// faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			int TIndex = Data->RawIndices[i + 1] - 1;

			const int ShiftInVerArr = VIndex * 3;
			const int ShiftInTexArr = VIndex * 2;

			if (bUseDoublePrecisionForReadingCoordinates)
			{
				Data->DVerC[ShiftInVerArr] = Data->RawVertexCoordinatesDoublePrecision[VIndex][0];
				Data->DVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinatesDoublePrecision[VIndex][1];
				Data->DVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinatesDoublePrecision[VIndex][2];
			}

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			// Saving material ID in vertex attribute array.
			for (size_t i = 0; i < Data->MaterialRecords.size(); i++)
			{
				if (VIndex >= int(Data->MaterialRecords[i].MinVertexIndex - 1) && VIndex <= int(Data->MaterialRecords[i].MaxVertexIndex - 1))
				{
					Data->MaterialIDs[VIndex] = float(i);
				}
			}

			Data->FTexC[ShiftInTexArr] = Data->RawTextureCoordinates[TIndex][0];
			Data->FTexC[ShiftInTexArr + 1] = 1 - Data->RawTextureCoordinates[TIndex][1];

			Data->FInd.push_back(VIndex);
		}

		CalculateNormals(Data);
		CalculateTangents(Data);
	}
	else if (!bHaveTextureCoord && bHaveNormalCoord)
	{
		if (bUseDoublePrecisionForReadingCoordinates)
			Data->DVerC.resize(Data->RawVertexCoordinatesDoublePrecision.size() * 3);
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(0);
		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTanC.resize(0);
		Data->FInd.resize(0);
		Data->MaterialIDs.resize(0);

		for (size_t i = 0; i < Data->RawIndices.size(); i += 2)
		{
			// Faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			const int NIndex = Data->RawIndices[i + 1] - 1;

			const int ShiftInVerArr = VIndex * 3;

			if (bUseDoublePrecisionForReadingCoordinates)
			{
				Data->DVerC[ShiftInVerArr] = Data->RawVertexCoordinatesDoublePrecision[VIndex][0];
				Data->DVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinatesDoublePrecision[VIndex][1];
				Data->DVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinatesDoublePrecision[VIndex][2];
			}

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			Data->FNorC[ShiftInVerArr] = Data->RawNormalCoordinates[NIndex][0];
			Data->FNorC[ShiftInVerArr + 1] = Data->RawNormalCoordinates[NIndex][1];
			Data->FNorC[ShiftInVerArr + 2] = Data->RawNormalCoordinates[NIndex][2];

			Data->FInd.push_back(VIndex);
		}
	}
	else
	{
		if (bUseDoublePrecisionForReadingCoordinates)
			Data->DVerC.resize(Data->RawVertexCoordinatesDoublePrecision.size() * 3);
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(0);
		Data->FNorC.resize(0);
		Data->FTanC.resize(0);
		Data->FInd.resize(0);
		Data->MaterialIDs.resize(0);

		for (size_t i = 0; i < Data->RawIndices.size(); i++)
		{
			// Faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;

			int ShiftInVerArr = VIndex * 3;

			if (bUseDoublePrecisionForReadingCoordinates)
			{
				Data->DVerC[ShiftInVerArr] = Data->RawVertexCoordinatesDoublePrecision[VIndex][0];
				Data->DVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinatesDoublePrecision[VIndex][1];
				Data->DVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinatesDoublePrecision[VIndex][2];
			}

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			Data->FInd.push_back(VIndex);
		}

		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		CalculateNormals(Data);
	}

	if (bHaveColors)
	{
		Data->FColorsC.resize(Data->RawVertexCoordinates.size() * 3);
		for (size_t i = 0; i < Data->RawIndices.size(); i++)
		{
			// Faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			if (VIndex >= Data->RawVertexCoordinates.size())
				continue;
			const int ShiftInVerArr = VIndex * 3;

			Data->FColorsC[ShiftInVerArr] = Data->RawVertexColors[VIndex][0];
			Data->FColorsC[ShiftInVerArr + 1] = Data->RawVertexColors[VIndex][1];
			Data->FColorsC[ShiftInVerArr + 2] = Data->RawVertexColors[VIndex][2];
		}
	}
}

void FEObjLoader::ReadMaterialFile(const char* OriginalOBJFile)
{
	if (MaterialFileName.empty() || OriginalOBJFile == "")
		return;

	std::string MaterialFileFullPath = FILE_SYSTEM.GetDirectoryPath(OriginalOBJFile);
	MaterialFileFullPath += MaterialFileName;
	if (!FILE_SYSTEM.DoesFileExist(MaterialFileFullPath.c_str()))
	{
		LOG.Add(std::string("material file: ") + MaterialFileName + " was indicated in OBJ file but this file can't be located.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::ifstream File;
	File.open(MaterialFileFullPath);

	if ((File.rdstate() & std::ifstream::failbit) != 0)
	{
		LOG.Add(std::string("can't load material file: ") + MaterialFileFullPath + " in function FEObjLoader::ReadMaterialFile.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::stringstream FileData;
	FileData << File.rdbuf();
	File.close();

	std::string Line;
	while (std::getline(FileData, Line))
	{
		// read next line
		std::stringstream LineStream;
		LineStream << Line;

		ReadMaterialLine(LineStream);
	}
}

bool FEObjLoader::CheckCurrentMaterialObject()
{
	if (CurrentMaterialObject == nullptr)
	{
		LOG.Add("currentMaterialObject is nullptr in function FEObjLoader::ReadMaterialFile.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (CurrentMaterialObject->MaterialRecords.empty())
	{
		LOG.Add("MaterialRecords is empty in function FEObjLoader::ReadMaterialFile.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void FEObjLoader::ReadMaterialLine(std::stringstream& LineStream)
{
	auto LookForFile = [&](std::string& FilePath) {
		if (CurrentMaterialObject->MaterialRecords[0].Name.find('/') != std::string::npos)
		{
			std::string name = CurrentMaterialObject->MaterialRecords[0].Name;
			for (size_t i = name.size() - 1; i > 0; i--)
			{
				if (name[i] == '/')
				{
					name.erase(name.begin() + i, name.end());
					break;
				}
			}

			const std::string NewPath = std::string(FILE_SYSTEM.GetDirectoryPath(CurrentFilePath.c_str())) + name + "/" + FilePath;
			FilePath = NewPath;
		}
		else
		{
			const std::string NewPath = std::string(FILE_SYSTEM.GetDirectoryPath(CurrentFilePath.c_str())) + FilePath;
			FilePath = NewPath;
		}
	};

	std::string STemp;
	std::string* StringEdited = nullptr;

	LineStream >> STemp;
	// To lower case
	std::transform(STemp.begin(), STemp.end(), STemp.begin(), [](const unsigned char C) { return std::tolower(C); });

	// if it is comment or object declaration or other not relevant info
	if (STemp[0] == '#' || STemp[0] == 'o')
	{
		// get to the next line
		return;
	}
	// if this line contains material declaration
	else if (STemp.find("newmtl") != std::string::npos)
	{
		std::string MaterialName;
		LineStream >> MaterialName;

		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			for (size_t j = 0; j < LoadedObjects[i]->MaterialRecords.size(); j++)
			{
				if (LoadedObjects[i]->MaterialRecords[j].Name == MaterialName)
				{
					CurrentMaterialObject = LoadedObjects[i];
					return;
				}
			}
		}
		
		LOG.Add(std::string("can't find material: ") + MaterialName + " from material file in function FEObjLoader::ReadMaterialLine.", "FE_LOG_LOADING", FE_LOG_ERROR);
	}
	// The diffuse texture map.
	else if (STemp.find("map_kd") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].AlbedoMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Specular color texture map
	else if (STemp.find("map_ks") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].SpecularMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Specular highlight component
	else if (STemp.find("map_ns") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].SpecularHighlightMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// The alpha texture map
	else if (STemp.find("map_d") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].AlphaMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Some implementations use 'map_bump' instead of 'bump' below
	else if (STemp.find("map_bump") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].NormalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Bump map (which by default uses luminance channel of the image)
	else if (STemp.find("bump") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].NormalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Displacement map
	else if (STemp.find("disp") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].DisplacementMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Stencil decal texture (defaults to 'matte' channel of the image)
	else if (STemp.find("decal") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].StencilDecalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.DoesFileExist(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
}

void FEObjLoader::ForceOneMesh(bool NewValue)
{
	bForceOneMesh = NewValue;
}

bool FEObjLoader::IsForcingOneMesh()
{
	return bForceOneMesh;
}

void FEObjLoader::ForcePositionNormalization(bool NewValue)
{
	bForcePositionNormalization = NewValue;
}

bool FEObjLoader::IsForcingPositionNormalization()
{
	return bForcePositionNormalization;
}

std::vector<FERawOBJData*>* FEObjLoader::GetLoadedObjects()
{
	return &LoadedObjects;
}

void FEObjLoader::UseDoublePrecisionForReadingCoordinates(bool NewValue)
{
	bUseDoublePrecisionForReadingCoordinates = NewValue;
}

bool FEObjLoader::IsUsingDoublePrecisionForReadingCoordinates()
{
	return bUseDoublePrecisionForReadingCoordinates;
}

void FEObjLoader::DoubleVertexOnSeams(bool NewValue)
{
	bDoubleVertexOnSeams = NewValue;
}

bool FEObjLoader::IsDoubleVertexOnSeams()
{
	return bDoubleVertexOnSeams;
}

bool FEObjLoader::SaveToOBJ(const char* FileName, FERawOBJData* Data)
{
	if (FileName == nullptr || Data == nullptr)
	{
		LOG.Add(std::string("Invalid parameters in function FEObjLoader::SaveToOBJ."), "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	std::ofstream File(FileName);
	if (!File.is_open())
	{
		LOG.Add(std::string("Failed to open file for writing in function FEObjLoader::SaveToOBJ: ") + FileName, "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	// Write header/comment
	File << "# OBJ file exported by FocalEngine\n";
	File << "# " << Data->RawVertexCoordinates.size() << " vertices\n";
	File << "# " << Data->RawTextureCoordinates.size() << " texture coordinates\n";
	File << "# " << Data->RawNormalCoordinates.size() << " normals\n";
	File << "# " << Data->RawIndices.size() / 3 << " faces\n\n";

	// Write material library reference if we have materials
	if (!Data->MaterialRecords.empty())
	{
		// Nothing to do here, we don't support writing MTL files
	}

	// Write vertices
	for (size_t i = 0; i < Data->RawVertexCoordinates.size(); i++)
	{
		File << "v " << Data->RawVertexCoordinates[i].x << " " 
			 << Data->RawVertexCoordinates[i].y << " " 
			 << Data->RawVertexCoordinates[i].z;

		// Add vertex colors if they exist
		if (bHaveColors && i < Data->RawVertexColors.size())
		{
			File << " " << Data->RawVertexColors[i].x << " "
				 << Data->RawVertexColors[i].y << " "
				 << Data->RawVertexColors[i].z;
		}

		File << "\n";
	}
	File << "\n";

	// Write texture coordinates
	if (bHaveTextureCoord)
	{
		for (size_t i = 0; i < Data->RawTextureCoordinates.size(); i++)
		{
			// OBJ format typically has Y flipped compared to OpenGL
			File << "vt " << Data->RawTextureCoordinates[i].x << " "
				 << (1.0f - Data->RawTextureCoordinates[i].y) << "\n";
		}
		File << "\n";
	}

	// Write normals
	if (bHaveNormalCoord)
	{
		for (size_t i = 0; i < Data->RawNormalCoordinates.size(); i++)
		{
			File << "vn " << Data->RawNormalCoordinates[i].x << " "
				 << Data->RawNormalCoordinates[i].y << " "
				 << Data->RawNormalCoordinates[i].z << "\n";
		}
		File << "\n";
	}

	size_t FaceCount = Data->RawIndices.size();
	for (size_t i = 0; i < FaceCount; i += 3)
	{
		// Write face indices
		// OBJ indices are 1-based, our internal indices are 0-based
		File << "f ";

		// Handle different formats based on what data we have
		if (bHaveTextureCoord && bHaveNormalCoord)
		{
			// Format: v/vt/vn
			for (size_t j = 0; j < 3; j++)
			{
				if (i + j < FaceCount)
				{
					int CurrentIndex = Data->RawIndices[i + j];
					int CurrentTextureIndex = Data->UVIndices[i + j];
					int CurrentNormalIndex = Data->NormalIndices[i + j];
					File << CurrentIndex << "/" << CurrentTextureIndex << "/" << CurrentNormalIndex << " ";
				}
			}
		}
		else if (bHaveTextureCoord && !bHaveNormalCoord)
		{
			// Format: v/vt
			for (size_t j = 0; j < 3; j++)
			{
				if (i + j < FaceCount)
				{
					int CurrentIndex = Data->RawIndices[i + j];
					int CurrentTextureIndex = Data->UVIndices[i + j];
					File << CurrentIndex << "/" << CurrentTextureIndex << " ";
				}
			}
		}
		else if (!bHaveTextureCoord && bHaveNormalCoord)
		{
			// Format: v//vn
			for (size_t j = 0; j < 3; j++)
			{
				if (i + j < FaceCount)
				{
					int CurrentIndex = Data->RawIndices[i + j];
					int CurrentNormalIndex = Data->NormalIndices[i + j];
					File << CurrentIndex << "//" << CurrentNormalIndex << " ";
				}
			}
		}
		else
		{
			// Format: v
			for (size_t j = 0; j < 3; j++)
			{
				if (i + j < FaceCount)
				{
					File << (Data->RawIndices[i + j]) << " ";
				}
			}
		}

		File << "\n";
	}

	File.close();
	LOG.Add(std::string("Successfully saved OBJ file: ") + FileName, "FE_LOG_SAVING", FE_LOG_INFO);
	return true;
}

glm::dvec3 FEObjLoader::GetLastAppliedShift() const
{
	return LastAppliedShift;
}