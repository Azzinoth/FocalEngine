#include "FEObjLoader.h"
using namespace FocalEngine;

FEObjLoader* FEObjLoader::_instance = nullptr;

FEObjLoader::FEObjLoader()
{
	
}

FEObjLoader::~FEObjLoader()
{

}

void FEObjLoader::readLine(std::stringstream& lineStream, FERawOBJData* data)
{
	std::string sTemp;

	lineStream >> sTemp;
	// To lower case
	std::transform(sTemp.begin(), sTemp.end(), sTemp.begin(), [](unsigned char c) { return std::tolower(c); });

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

		data->rawVertexCoordinates.push_back(newVec);
	}
	// if this line contains vertex texture coordinates
	else if (sTemp[0] == 'v' && sTemp.size() == 2 && sTemp[1] == 't')
	{
		haveTextureCoord = true;

		glm::vec2 newVec;
		for (int i = 0; i <= 1; i++)
		{
			lineStream >> sTemp;
			newVec[i] = std::stof(sTemp);
		}

		data->rawTextureCoordinates.push_back(newVec);
	}
	// if this line contains vertex texture coordinates
	else if (sTemp[0] == 'v' && sTemp.size() == 2 && sTemp[1] == 'n')
	{
		haveNormalCoord = true;

		glm::vec3 newVec;
		for (int i = 0; i <= 2; i++)
		{
			lineStream >> sTemp;
			newVec[i] = std::stof(sTemp);
		}

		data->rawNormalCoordinates.push_back(newVec);
	}
	// if this line contains indices
	else if (sTemp[0] == 'f' && sTemp.size() == 1)
	{
		for (int i = 0; i <= 2; i++)
		{
			lineStream >> sTemp;

			std::stringstream tempLineStrem;
			tempLineStrem << sTemp;

			int iterations = 0;
			while (std::getline(tempLineStrem, sTemp, '/'))
			{
				if (sTemp != "")
				{
					data->rawIndices.push_back(std::stoi(sTemp));
				}
				else
				{
					// Texture coordinates are optional.
					if (iterations == 1)
					{
						// It is not proper fix!
						data->rawIndices.push_back(1);
					}
					else
					{
						LOG.add(std::string("Not texture coordinates was absent in face description in function FEObjLoader::readFile."), FE_LOG_ERROR, FE_LOG_LOADING);
					}
				}

				if (data->materialRecords.size() != 0)
				{
					if (iterations == 0)
					{
						if (data->materialRecords.back().minVertexIndex > unsigned int(data->rawIndices.back()))
							data->materialRecords.back().minVertexIndex = unsigned int(data->rawIndices.back());

						if (data->materialRecords.back().maxVertexIndex < unsigned int(data->rawIndices.back()))
							data->materialRecords.back().maxVertexIndex = unsigned int(data->rawIndices.back());
					}
					else if (iterations == 1)
					{
						if (data->materialRecords.back().minTextureIndex > unsigned int(data->rawIndices.back()))
							data->materialRecords.back().minTextureIndex = unsigned int(data->rawIndices.back());

						if (data->materialRecords.back().maxTextureIndex < unsigned int(data->rawIndices.back()))
							data->materialRecords.back().maxTextureIndex = unsigned int(data->rawIndices.back());
					}
					else if (iterations == 2)
					{
						if (data->materialRecords.back().minNormalIndex > unsigned int(data->rawIndices.back()))
							data->materialRecords.back().minNormalIndex = unsigned int(data->rawIndices.back());

						if (data->materialRecords.back().maxNormalIndex < unsigned int(data->rawIndices.back()))
							data->materialRecords.back().maxNormalIndex = unsigned int(data->rawIndices.back());
					}

					data->materialRecords.back().faceCount++;
				}
				
				iterations++;
			}
		}
	}
	// if this line contains new material declaration
	else if (sTemp.find("usemtl") != std::string::npos)
	{
		data->materialRecords.push_back(materialRecord());
		if (data->materialRecords.size() > 1)
			data->materialRecords.back().facesSeenBefore = data->materialRecords[data->materialRecords.size() - 2].facesSeenBefore + data->materialRecords[data->materialRecords.size() - 2].faceCount;
		
		lineStream >> data->materialRecords.back().name;
	}
	// file with materials data
	else if (sTemp.find("mtllib") != std::string::npos)
	{
		lineStream >> materialFileName;
	}
}

void FEObjLoader::readFile(const char* fileName)
{
	haveTextureCoord = false;
	haveNormalCoord = false;
	currentFilePath = fileName;
	materialFileName = "";
	currentMaterialObject = nullptr;
	for (size_t i = 0; i < loadedObjects.size(); i++)
	{
		delete loadedObjects[i];
	}
	loadedObjects.clear();
	loadedObjects.push_back(new FERawOBJData());

	if (fileName == nullptr)
	{
		LOG.add(std::string("No file name in function FEObjLoader::readFile."), FE_LOG_ERROR, FE_LOG_LOADING);
		return;
	}

	std::ifstream file;
	file.open(fileName);

	if ((file.rdstate() & std::ifstream::failbit) != 0)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEObjLoader::readFile.", FE_LOG_ERROR, FE_LOG_LOADING);
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
			
		readLine(lineStream, loadedObjects.back());
	}

	if (!forceOneMesh)
	{
		// Each material should represented by different FERawOBJData
		std::vector<FERawOBJData*> objectsPerMaterialList;
		for (size_t i = 0; i < loadedObjects.size(); i++)
		{
			for (size_t j = 0; j < loadedObjects[i]->materialRecords.size(); j++)
			{
				FERawOBJData* tempObject = new FERawOBJData();
				tempObject->materialRecords.push_back(materialRecord(loadedObjects[i]->materialRecords[j]));

				size_t startIndex = loadedObjects[i]->materialRecords[j].minVertexIndex - 1;
				size_t endIndex = loadedObjects[i]->materialRecords[j].maxVertexIndex;
				for (size_t k = startIndex; k < endIndex; k++)
				{
					tempObject->rawVertexCoordinates.push_back(loadedObjects[i]->rawVertexCoordinates[k]);
				}

				startIndex = loadedObjects[i]->materialRecords[j].minTextureIndex - 1;
				endIndex = loadedObjects[i]->materialRecords[j].maxTextureIndex;
				for (size_t k = startIndex; k < endIndex; k++)
				{
					tempObject->rawTextureCoordinates.push_back(loadedObjects[i]->rawTextureCoordinates[k]);
				}

				startIndex = loadedObjects[i]->materialRecords[j].minNormalIndex - 1;
				endIndex = loadedObjects[i]->materialRecords[j].maxNormalIndex;
				for (size_t k = startIndex; k < endIndex; k++)
				{
					tempObject->rawNormalCoordinates.push_back(loadedObjects[i]->rawNormalCoordinates[k]);
				}

				startIndex = loadedObjects[i]->materialRecords[j].facesSeenBefore;
				endIndex = startIndex + loadedObjects[i]->materialRecords[j].faceCount;
				for (size_t k = startIndex; k < endIndex; k += 3)
				{
					tempObject->rawIndices.push_back(loadedObjects[i]->rawIndices[k] - loadedObjects[i]->materialRecords[j].minVertexIndex + 1);
					tempObject->rawIndices.push_back(loadedObjects[i]->rawIndices[k + 1] - loadedObjects[i]->materialRecords[j].minTextureIndex + 1);
					tempObject->rawIndices.push_back(loadedObjects[i]->rawIndices[k + 2] - loadedObjects[i]->materialRecords[j].minNormalIndex + 1);
				}

				objectsPerMaterialList.push_back(tempObject);
			}

			if (loadedObjects[i]->materialRecords.size() == 0)
				objectsPerMaterialList.push_back(new FERawOBJData(*loadedObjects[i]));
		}

		for (size_t i = 0; i < loadedObjects.size(); i++)
		{
			delete loadedObjects[i];
		}
		loadedObjects.clear();
		loadedObjects = objectsPerMaterialList;
	}

	readMaterialFile(fileName);

	for (size_t i = 0; i < loadedObjects.size(); i++)
	{
		processRawData(loadedObjects[i]);
	}
}

glm::vec3 FEObjLoader::calculateTangent(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::vector<glm::vec2>&& textures)
{
	glm::vec3 q1 = v1 - v0;
	glm::vec3 q2 = v2 - v0;
	glm::vec2 uv0 = textures[0];
	glm::vec2 uv1 = textures[1];
	glm::vec2 uv2 = textures[2];

	float t1 = uv1.y - uv0.y;
	float t2 = uv2.y - uv0.y;

	glm::vec3 tangent = t1 * q2 - t2 * q1;

	return tangent;
}

void FEObjLoader::calculateTangents(FERawOBJData* data)
{
	for (size_t i = 0; i < data->fInd.size() - 1; i += 3)
	{
		glm::vec3 v0 = { data->fVerC[data->fInd[i] * 3], data->fVerC[data->fInd[i] * 3 + 1], data->fVerC[data->fInd[i] * 3 + 2] };
		glm::vec3 v1 = { data->fVerC[data->fInd[i + 1] * 3], data->fVerC[data->fInd[i + 1] * 3 + 1], data->fVerC[data->fInd[i + 1] * 3 + 2] };
		glm::vec3 v2 = { data->fVerC[data->fInd[i + 2] * 3], data->fVerC[data->fInd[i + 2] * 3 + 1], data->fVerC[data->fInd[i + 2] * 3 + 2] };

		glm::vec2 t0 = { data->fTexC[data->fInd[i] * 2], data->fTexC[data->fInd[i] * 2 + 1] };
		glm::vec2 t1 = { data->fTexC[data->fInd[i + 1] * 2], data->fTexC[data->fInd[i + 1] * 2 + 1] };
		glm::vec2 t2 = { data->fTexC[data->fInd[i + 2] * 2], data->fTexC[data->fInd[i + 2] * 2 + 1] };

		glm::vec3 tangent = calculateTangent(v0, v1, v2, { t0, t1, t2 });
		// To eliminate NaN values after normalization.
		// I encounter this problem if triangle has same texture coordinates.
		if (tangent.x != 0 || tangent.y != 0 || tangent.z != 0)
		{
			tangent = glm::normalize(tangent);
		}
		else
		{
			glm::vec3 normal = { data->fNorC[data->fInd[i] * 3], data->fNorC[data->fInd[i] * 3 + 1], data->fNorC[data->fInd[i] * 3 + 2] };
			glm::vec3 tangentOne = glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 tangentTwo = glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f));
			// Choosing candidate with bigger length/magnitude.
			// Length/magnitude of cross product depend on sine of angle between vectors
			// and sine of 90 degrees is 1.0(max value), so basically we are choosing cross product in which vectors was closer to perpendicular(assuming both vectors are unit vectors).
			tangent = glm::length(tangentOne) > glm::length(tangentTwo) ? tangentOne : tangentTwo;
			tangent = glm::normalize(tangent);
		}	

		data->fTanC[data->fInd[i] * 3] = tangent.x;
		data->fTanC[data->fInd[i] * 3 + 1] = tangent.y;
		data->fTanC[data->fInd[i] * 3 + 2] = tangent.z;

		data->fTanC[data->fInd[i + 1] * 3] = tangent.x;
		data->fTanC[data->fInd[i + 1] * 3 + 1] = tangent.y;
		data->fTanC[data->fInd[i + 1] * 3 + 2] = tangent.z;

		data->fTanC[data->fInd[i + 2] * 3] = tangent.x;
		data->fTanC[data->fInd[i + 2] * 3 + 1] = tangent.y;
		data->fTanC[data->fInd[i + 2] * 3 + 2] = tangent.z;
	}
}

void FEObjLoader::processRawData(FERawOBJData* data)
{
	if (haveTextureCoord && haveNormalCoord)
	{
#ifdef FE_OBJ_DOUBLE_VERTEX_ON_SEAMS
	std::vector<FEObjLoader::vertexThatNeedDoubling> vertexList;
	std::unordered_map<int, int> indexesMap;

	for (size_t i = 0; i < data->rawIndices.size(); i += 3)
	{
		indexesMap[data->rawIndices[i]] = int(i);
	}

	for (size_t i = 0; i < data->rawIndices.size(); i += 3)
	{
		if (indexesMap.find(data->rawIndices[i]) != indexesMap.end())
		{
			size_t j = indexesMap.find(data->rawIndices[i])->second;
			std::swap(i, j);

			bool TexD = data->rawIndices[i + 1] != data->rawIndices[j + 1];
			bool NormD = data->rawIndices[i + 2] != data->rawIndices[j + 2];
			if (data->rawIndices[i] == data->rawIndices[j] && (TexD || NormD))
			{
				// we do not need to add first appearance of vertex that we need to double
				FEObjLoader::vertexThatNeedDoubling newVertex = FEObjLoader::vertexThatNeedDoubling(int(j), data->rawIndices[j], data->rawIndices[j + 1], data->rawIndices[j + 2]);
				if (std::find(vertexList.begin(), vertexList.end(), newVertex) == vertexList.end())
				{
					vertexList.push_back(newVertex);
				}
			}

			std::swap(i, j);
		}
	}

	std::vector<std::pair<int, float>> doubledVertexMatIndecies;
	for (auto& ver : vertexList)
	{
		if (ver.wasDone) continue;

		data->rawVertexCoordinates.push_back(data->rawVertexCoordinates[ver.acctualIndex - 1]);

		int newVertexIndex = int(data->rawVertexCoordinates.size());
		data->rawIndices[ver.indexInArray] = newVertexIndex;
		ver.wasDone = true;

		// preserve matIndex!
		for (size_t i = 0; i < data->materialRecords.size(); i++)
		{
			if (ver.acctualIndex >= (int(data->materialRecords[i].minVertexIndex + 1)) && ver.acctualIndex <= (int(data->materialRecords[i].maxVertexIndex + 1)))
			{
				doubledVertexMatIndecies.push_back(std::make_pair(newVertexIndex, float(i)));
			}
		}

		for (auto& verNext : vertexList)
		{
			if (verNext.wasDone) continue;
			if (ver.indexInArray == verNext.indexInArray && (ver.texIndex == verNext.texIndex || ver.normIndex == verNext.normIndex))
			{
				data->rawIndices[verNext.indexInArray] = newVertexIndex;
				verNext.wasDone = true;
			}
		}
	}
#endif // FE_OBJ_DOUBLE_VERTEX_ON_SEAMS

	data->fVerC.resize(data->rawVertexCoordinates.size() * 3);
	data->fTexC.resize(data->rawVertexCoordinates.size() * 2);
	data->fNorC.resize(data->rawVertexCoordinates.size() * 3);
	data->fTanC.resize(data->rawVertexCoordinates.size() * 3);
	data->fInd.resize(0);
	data->matIDs.resize(data->rawVertexCoordinates.size());
	
	for (size_t i = 0; i < data->rawIndices.size(); i += 3)
	{
		// faces index in OBJ file begins from 1 not 0.
		int vIndex = data->rawIndices[i] - 1;
		int tIndex = data->rawIndices[i + 1] - 1;
		int nIndex = data->rawIndices[i + 2] - 1;

		int shiftInVerArr = vIndex * 3;
		int shiftInTexArr = vIndex * 2;

		data->fVerC[shiftInVerArr] = data->rawVertexCoordinates[vIndex][0];
		data->fVerC[shiftInVerArr + 1] = data->rawVertexCoordinates[vIndex][1];
		data->fVerC[shiftInVerArr + 2] = data->rawVertexCoordinates[vIndex][2];

		// saving material ID in vertex attribute array
		for (size_t i = 0; i < data->materialRecords.size(); i++)
		{
			if (vIndex >= int(data->materialRecords[i].minVertexIndex - 1) && vIndex <= int(data->materialRecords[i].maxVertexIndex - 1))
			{
				data->matIDs[vIndex] = float(i);
			}
		}

		data->fTexC[shiftInTexArr] = data->rawTextureCoordinates[tIndex][0];
		data->fTexC[shiftInTexArr + 1] = 1 - data->rawTextureCoordinates[tIndex][1];

		data->fNorC[shiftInVerArr] = data->rawNormalCoordinates[nIndex][0];
		data->fNorC[shiftInVerArr + 1] = data->rawNormalCoordinates[nIndex][1];
		data->fNorC[shiftInVerArr + 2] = data->rawNormalCoordinates[nIndex][2];

		data->fInd.push_back(vIndex);
	}

#ifdef FE_OBJ_DOUBLE_VERTEX_ON_SEAMS
	for (size_t j = 0; j < doubledVertexMatIndecies.size(); j++)
	{
		data->matIDs[doubledVertexMatIndecies[j].first - 1] = doubledVertexMatIndecies[j].second;
	}
#endif // FE_OBJ_DOUBLE_VERTEX_ON_SEAMS

	calculateTangents(data);

	}
	else
	{
		data->fVerC.resize(data->rawVertexCoordinates.size() * 3);
		data->fTexC.resize(0);
		data->fNorC.resize(0);
		data->fTanC.resize(0);
		data->fInd.resize(0);
		data->matIDs.resize(0);

		for (size_t i = 0; i < data->rawIndices.size(); i++)
		{
			// faces index in OBJ file begins from 1 not 0.
			int vIndex = data->rawIndices[i] - 1;
			int shiftInVerArr = vIndex * 3;

			data->fVerC[shiftInVerArr] = data->rawVertexCoordinates[vIndex][0];
			data->fVerC[shiftInVerArr + 1] = data->rawVertexCoordinates[vIndex][1];
			data->fVerC[shiftInVerArr + 2] = data->rawVertexCoordinates[vIndex][2];

			data->fInd.push_back(vIndex);
		}
	}
}

void FEObjLoader::readMaterialFile(const char* originalOBJFile)
{
	if (materialFileName == "" || originalOBJFile == "")
		return;

	std::string materialFileFullPath = FILE_SYSTEM.getDirectoryPath(originalOBJFile);
	materialFileFullPath += materialFileName;
	if (!FILE_SYSTEM.checkFile(materialFileFullPath.c_str()))
	{
		LOG.add(std::string("material file: ") + materialFileName + " was indicated in OBJ file but this file can't be located.", FE_LOG_ERROR, FE_LOG_LOADING);
		return;
	}

	std::ifstream file;
	file.open(materialFileFullPath);

	if ((file.rdstate() & std::ifstream::failbit) != 0)
	{
		LOG.add(std::string("can't load material file: ") + materialFileFullPath + " in function FEObjLoader::readMaterialFile.", FE_LOG_ERROR, FE_LOG_LOADING);
		return;
	}

	std::stringstream fileData;
	fileData << file.rdbuf();
	file.close();

	std::string line;
	while (std::getline(fileData, line))
	{
		// read next line
		std::stringstream lineStream;
		lineStream << line;

		readMaterialLine(lineStream);
	}
}

bool FEObjLoader::checkCurrentMaterialObject()
{
	if (currentMaterialObject == nullptr)
	{
		LOG.add(std::string("currentMaterialObject is nullptr in function FEObjLoader::readMaterialLine.", FE_LOG_ERROR, FE_LOG_LOADING));
		return false;
	}

	if (currentMaterialObject->materialRecords.size() < 1)
	{
		LOG.add(std::string("materialRecords is empty in function FEObjLoader::readMaterialLine.", FE_LOG_ERROR, FE_LOG_LOADING));
		return false;
	}

	return true;
}

void FEObjLoader::readMaterialLine(std::stringstream& lineStream)
{
	auto lookForFile = [&](std::string& filePath) {
		if (currentMaterialObject->materialRecords[0].name.find('/') != std::string::npos)
		{
			std::string name = currentMaterialObject->materialRecords[0].name;
			for (size_t i = name.size() - 1; i > 0; i--)
			{
				if (name[i] == '/')
				{
					name.erase(name.begin() + i, name.end());
					break;
				}
			}

			std::string newPath = std::string(FILE_SYSTEM.getDirectoryPath(currentFilePath.c_str())) + name + "/" + filePath;
			filePath = newPath;
		}
		else
		{
			std::string newPath = std::string(FILE_SYSTEM.getDirectoryPath(currentFilePath.c_str())) + filePath;
			filePath = newPath;
		}
	};

	std::string sTemp;
	std::string* stringEdited = nullptr;

	lineStream >> sTemp;
	// To lower case
	std::transform(sTemp.begin(), sTemp.end(), sTemp.begin(), [](unsigned char c) { return std::tolower(c); });

	// if it is comment or object declaration or other not relevant info
	if (sTemp[0] == '#' || sTemp[0] == 'o')
	{
		// get to the next line
		return;
	}
	// if this line contains material declaration
	else if (sTemp.find("newmtl") != std::string::npos)
	{
		std::string materialName;
		lineStream >> materialName;

		for (size_t i = 0; i < loadedObjects.size(); i++)
		{
			for (size_t j = 0; j < loadedObjects[i]->materialRecords.size(); j++)
			{
				if (loadedObjects[i]->materialRecords[j].name == materialName)
				{
					currentMaterialObject = loadedObjects[i];
					return;
				}
			}
		}
		
		LOG.add(std::string("can't find material: ") + materialName + " from material file in function FEObjLoader::readMaterialLine.", FE_LOG_ERROR, FE_LOG_LOADING);
	}
	// The diffuse texture map.
	else if (sTemp.find("map_kd") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].albedoMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Specular color texture map
	else if (sTemp.find("map_ks") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].specularMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Specular highlight component
	else if (sTemp.find("map_ns") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].specularHighlightMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// The alpha texture map
	else if (sTemp.find("map_d") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].alphaMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Some implementations use 'map_bump' instead of 'bump' below
	else if (sTemp.find("map_bump") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].normalMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Bump map (which by default uses luminance channel of the image)
	else if (sTemp.find("bump") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].normalMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Displacement map
	else if (sTemp.find("disp") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].displacementMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
	// Stencil decal texture (defaults to 'matte' channel of the image)
	else if (sTemp.find("decal") != std::string::npos)
	{
		if (!checkCurrentMaterialObject())
			return;

		stringEdited = &currentMaterialObject->materialRecords[0].stencilDecalMapFile;
		std::getline(lineStream, *stringEdited);

		if ((*stringEdited)[0] == ' ')
			stringEdited->erase(stringEdited->begin());

		if (!FILE_SYSTEM.checkFile(stringEdited->c_str()))
			lookForFile(*stringEdited);
	}
}