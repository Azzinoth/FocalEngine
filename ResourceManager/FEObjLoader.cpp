#include "FEObjLoader.h"
using namespace FocalEngine;

FEObjLoader* FEObjLoader::_instance = nullptr;

FEObjLoader::FEObjLoader()
{

}

FEObjLoader::~FEObjLoader()
{

}

void FEObjLoader::readLine(std::stringstream& lineStream)
{
	std::string sTemp;

	lineStream >> sTemp;
	// if it is comment or object declaration or other not relevant info
	if (sTemp[0] == '#' || sTemp[0] == 'o')
	{
		// get to the next line
		return;
	}

	// if this line contains vertex coordinates
	if (sTemp[0] == 'v' && sTemp.size() == 1)
	{
		glm::vec3 newVec;
		for (int i = 0; i <= 2; i++)
		{
			lineStream >> sTemp;
			newVec[i] = std::stof(sTemp);
		}

		rawVertexCoordinates.push_back(newVec);
	}
	// if this line contains vertex texture coordinates
	else if (sTemp[0] == 'v' && sTemp.size() == 2 && sTemp[1] == 't')
	{
		glm::vec2 newVec;
		for (int i = 0; i <= 1; i++)
		{
			lineStream >> sTemp;
			newVec[i] = std::stof(sTemp);
		}

		rawTextureCoordinates.push_back(newVec);
	}
	// if this line contains vertex texture coordinates
	else if (sTemp[0] == 'v' && sTemp.size() == 2 && sTemp[1] == 'n')
	{
		glm::vec3 newVec;
		for (int i = 0; i <= 2; i++)
		{
			lineStream >> sTemp;
			newVec[i] = std::stof(sTemp);
		}

		rawNormalCoordinates.push_back(newVec);
	}
	// if this line contains indices
	else if (sTemp[0] == 'f' && sTemp.size() == 1)
	{
		for (int i = 0; i <= 2; i++)
		{
			lineStream >> sTemp;

			std::stringstream tempLineStrem;
			tempLineStrem << sTemp;

			while (std::getline(tempLineStrem, sTemp, '/'))
			{
				rawIndices.push_back(std::stoi(sTemp));
			}
		}
	}
}

void FEObjLoader::readFile(const char* fileName)
{
	rawVertexCoordinates.clear();
	rawTextureCoordinates.clear();
	rawNormalCoordinates.clear();
	rawIndices.clear();

	// to-do: error no file name
	if (fileName == nullptr)
		return;

	std::ifstream file;
	file.open(fileName);

	if ((file.rdstate() & std::ifstream::failbit) != 0)
	{
		// to-do: error can't open file
		return;
	}

	std::stringstream fileData;
	// read file to fileData and close it.
	fileData << file.rdbuf();
	file.close();

	std::string line;
	while (std::getline(fileData, line))
	{
		// read next line
		std::stringstream lineStream;
		lineStream << line;

		readLine(lineStream);
	}

	processRawData();
}

glm::vec3 FEObjLoader::calculateTangent(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::vector<glm::vec2>&& textures)
{
	// to-do: code from old engine, refraction needed!
	glm::vec3 q1 = v1 - v0;
	glm::vec3 q2 = v2 - v0;
	glm::vec2 uv0 = textures[0];
	glm::vec2 uv1 = textures[1];
	glm::vec2 uv2 = textures[2];

	float t1 = uv1.y - uv0.y;
	float t2 = uv2.y - uv0.y;

	glm::vec3 tangent = t2*q1 - t1*q2;

	return tangent;
}

void FEObjLoader::calculateTangents()
{
	for (size_t i = 0; i < fInd.size() - 1; i += 3)
	{
		glm::vec3 v0 = { fVerC[fInd[i] * 3], fVerC[fInd[i] * 3 + 1], fVerC[fInd[i] * 3 + 2] };
		glm::vec3 v1 = { fVerC[fInd[i + 1] * 3], fVerC[fInd[i + 1] * 3 + 1], fVerC[fInd[i + 1] * 3 + 2] };
		glm::vec3 v2 = { fVerC[fInd[i + 2] * 3], fVerC[fInd[i + 2] * 3 + 1], fVerC[fInd[i + 2] * 3 + 2] };

		glm::vec2 t0 = { fTexC[fInd[i] * 2], fTexC[fInd[i] * 2 + 1] };
		glm::vec2 t1 = { fTexC[fInd[i + 1] * 2], fTexC[fInd[i + 1] * 2 + 1] };
		glm::vec2 t2 = { fTexC[fInd[i + 2] * 2], fTexC[fInd[i + 2] * 2 + 1] };

		glm::vec3 tangent = calculateTangent(v0, v1, v2, { t0, t1, t2 });
		tangent = glm::normalize(tangent);

		fTanC[fInd[i] * 3] = tangent.x;
		fTanC[fInd[i] * 3 + 1] = tangent.y;
		fTanC[fInd[i] * 3 + 2] = tangent.z;

		fTanC[fInd[i + 1] * 3] = tangent.x;
		fTanC[fInd[i + 1] * 3 + 1] = tangent.y;
		fTanC[fInd[i + 1] * 3 + 2] = tangent.z;

		fTanC[fInd[i + 2] * 3] = tangent.x;
		fTanC[fInd[i + 2] * 3 + 1] = tangent.y;
		fTanC[fInd[i + 2] * 3 + 2] = tangent.z;
	}
}

void FEObjLoader::processRawData()
{
	std::vector<std::tuple<int, int, int, bool>> vertexThatNeedDoubling;
	std::unordered_map<int, int> indexesMap;

	for (size_t i = 0; i < rawIndices.size(); i += 3)
	{
		indexesMap[rawIndices[i]] = i;
	}

	for (size_t i = 0; i < rawIndices.size(); i += 3)
	{
		if (indexesMap.find(rawIndices[i]) != indexesMap.end())
		{
			size_t j = indexesMap.find(rawIndices[i])->second;
			std::swap(i, j);

			bool TexD = rawIndices[i + 1] != rawIndices[j + 1];
			bool NormD = rawIndices[i + 2] != rawIndices[j + 2];
			if (rawIndices[i] == rawIndices[j] && TexD)
			{
				// we do not need to add first appearance of vertex that is we need to double
				auto newVertexTuple = std::make_tuple(j, rawIndices[j], rawIndices[j + 1], false);
				if (std::find(vertexThatNeedDoubling.begin(), vertexThatNeedDoubling.end(), newVertexTuple) == vertexThatNeedDoubling.end())
				{
					vertexThatNeedDoubling.push_back(newVertexTuple);
				}
			}
			else if (rawIndices[i] == rawIndices[j] && NormD)
			{
				// to-do: check do I need to fill that. OldCode.
			}

			std::swap(i, j);
		}
	}

	// to-do: rewrite with out std::tuple
	for (auto& ver : vertexThatNeedDoubling)
	{
		if (std::get<3>(ver)) continue;

		rawVertexCoordinates.push_back(rawVertexCoordinates[std::get<1>(ver) - 1]);

		int newVertexIndex = rawVertexCoordinates.size();
		rawIndices[std::get<0>(ver)] = newVertexIndex;
		std::get<3>(ver) = true;

		for (auto& verNext : vertexThatNeedDoubling)
		{
			if (std::get<3>(verNext)) continue;
			if (std::get<1>(ver) == std::get<1>(verNext) && std::get<2>(ver) == std::get<2>(verNext))
			{
				rawIndices[std::get<0>(verNext)] = newVertexIndex;
				std::get<3>(verNext) = true;
			}
		}
	}

	fVerC.resize(rawVertexCoordinates.size() * 3);
	fTexC.resize(rawVertexCoordinates.size() * 2);
	fNorC.resize(rawVertexCoordinates.size() * 3);
	fTanC.resize(rawVertexCoordinates.size() * 3);

	for (size_t i = 0; i < rawIndices.size(); i += 3)
	{
		// faces index in OBJ file begins from 1 not 0.
		int vIndex = rawIndices[i] - 1;
		int tIndex = rawIndices[i + 1] - 1;
		int nIndex = rawIndices[i + 2] - 1;

		int shiftInVerArr = vIndex * 3;
		int shiftInTexArr = vIndex * 2;

		fVerC[shiftInVerArr] = rawVertexCoordinates[vIndex][0];
		fVerC[shiftInVerArr + 1] = rawVertexCoordinates[vIndex][1];
		fVerC[shiftInVerArr + 2] = rawVertexCoordinates[vIndex][2];

		fTexC[shiftInTexArr] = rawTextureCoordinates[tIndex][0];
		fTexC[shiftInTexArr + 1] = 1 - rawTextureCoordinates[tIndex][1];

		fNorC[shiftInVerArr] = rawNormalCoordinates[nIndex][0];
		fNorC[shiftInVerArr + 1] = rawNormalCoordinates[nIndex][1];
		fNorC[shiftInVerArr + 2] = rawNormalCoordinates[nIndex][2];

		fInd.push_back(vIndex);
	}

	calculateTangents();
}