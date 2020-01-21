#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

FETexture* FEResourceManager::LoadPngTexture(const char* fileName, std::string Name)
{
	FETexture* newTexture = new FETexture();
	std::vector<unsigned char> rawData;
	unsigned uWidth, uHeight;

	if (Name.size() == 0 || textures.find(Name) != textures.end())
	{
		Name = getFileNameFromFilePath(fileName);
		size_t nextID = textures.size();
		size_t index = 0;
		while (textures.find(Name) != textures.end() || Name.size() == 0)
		{
			index++;
			Name = "texture_" + std::to_string(nextID + index);
		}
	}

	lodepng::decode(rawData, uWidth, uHeight, fileName);
	if (rawData.size() == 0)
	{
		//Log...
		assert(rawData.size());
	}
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data()));

	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));// to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	newTexture->setName(Name);
	newTexture->fileName = fileName;

	// save texture in internal format and add it to project\scene
	saveFETexture((Name + ".texture").c_str(), newTexture, (char*)rawData.data());
	textures[Name] = newTexture;

	return newTexture;
}

FETexture* FEResourceManager::LoadFETexture(const char* fileName, std::string Name)
{
	FETexture* newTexture = new FETexture();

	if (Name.size() == 0)
		Name = getFileNameFromFilePath(fileName);

	std::fstream file;

	file.open(fileName, std::ios::in | std::ios::binary);
	char * buffer = new char[4];

	file.read(buffer, 4);
	newTexture->width = *(int*)buffer;

	file.read(buffer, 4);
	newTexture->height = *(int*)buffer;

	file.read(buffer, 4);
	int size = *(int*)buffer;

	char* textureData = new char[size];
	file.read(textureData, size);

	file.close();

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));

	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));// to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	newTexture->setName(Name);
	newTexture->fileName = fileName;

	delete[] buffer;
	delete[] textureData;

	return newTexture;
}

void FEResourceManager::saveFETexture(const char* fileName, FETexture* texture, char* textureData)
{
	std::fstream file;

	file.open(fileName, std::ios::out | std::ios::binary);

	file.write((char*)&texture->width, sizeof(int));
	file.write((char*)&texture->height, sizeof(int));

	int size = strlen(textureData);
	file.write((char*)&size, sizeof(int));
	file.write((char*)textureData, sizeof(unsigned char) * size);

	file.close();
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	return new FEMesh(vaoID, positions.size() / 3, FE_POSITION, FEAABB(positions));
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	return new FEMesh(vaoID, positions.size() / 3, FE_POSITION | FE_NORMAL, FEAABB(positions));
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index.size(), index.data(), GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangents.size(), tangents.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UV.size(), UV.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	return new FEMesh(vaoID, index.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB(positions));
}

FEMesh* FEResourceManager::rawDataToMesh(float* positions, int posSize, float* UV, int UVSize, float* normals, int normSize,
										 float* tangents, int tanSize, int* indices, int indexSize)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	return new FEMesh(vaoID, indexSize, FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB());
}

FEMesh* FEResourceManager::rawObjDataToMesh()
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * objLoader.fInd.size(), objLoader.fInd.data(), GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fVerC.size(), objLoader.fVerC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fNorC.size(), objLoader.fNorC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTanC.size(), objLoader.fTanC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTexC.size(), objLoader.fTexC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	return new FEMesh(vaoID, objLoader.fInd.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB(objLoader.fVerC));
}

void FEResourceManager::loadStandardMeshes()
{
	if (meshes.find("cube") != meshes.end())
		return;

	std::vector<int> cubeIndices = {
		4, 2, 0, 9, 7, 3, 6, 5,	20,	21,	15,
		22,	10,	12,	18,	8, 1, 19, 4, 17, 2,
		9, 23, 7, 6, 13, 5, 24, 16, 15, 10,
		14, 12, 8, 11, 1
	};

	std::vector<float> cubePositions = {
		1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};

	std::vector<float> cubeNormals = {
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f, 0.0f
	};

	std::vector<float> cubeTangents = {
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
	};

	std::vector<float> cubeUV = {
		0.375f, 1.0f, 0.625f, 0.25f, 0.375f, 0.75f,
		0.375f, 0.75f, 0.625f, 1.0f, 0.375f, 0.25f,
		0.625f, 0.5f, 0.375f, 0.5f, 0.875f, 0.5f,
		0.625f, 0.75f, 0.375f, 0.5f, 0.875f, 0.25f,
		0.125f, 0.25f, 0.625f, 0.25f, 0.375f, 0.25f,
		0.375f, 0.0f, 0.625f, 0.0f, 0.625f, 0.75f,
		0.125f, 0.5f, 0.625f, 0.5f, 0.375f, 0.5f,
		0.625f, 0.25f, 0.375f, 0.25f, 0.625f, 0.5f,
		0.625f, 0.25f
	};

	meshes["cube"] = rawDataToMesh(cubePositions, cubeNormals, cubeTangents, cubeUV, cubeIndices);
	meshes["cube"]->setName("cube");

	std::vector<int> planeIndices = {
		0, 1, 2, 3, 0, 2
	};

	std::vector<float> planePositions = {
		-1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	std::vector<float> planeNormals = {
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	std::vector<float> planeTangents = {
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f
	};

	std::vector<float> planeUV = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	meshes["plane"] = rawDataToMesh(planePositions, planeNormals, planeTangents, planeUV, planeIndices);
	meshes["plane"]->setName("plane");
}

FEResourceManager::FEResourceManager()
{
	loadStandardMaterial();
	loadStandardMeshes();
}

FEResourceManager::~FEResourceManager()
{
	clear();
}

FEMesh* FEResourceManager::getSimpleMesh(std::string meshName)
{
	if (meshes.find(meshName) != meshes.end())
		return meshes[meshName];

	return nullptr;
}

FEMesh* FEResourceManager::LoadOBJMesh(const char* fileName, std::string Name)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.readFile(fileName);
	FEMesh* newMesh = rawObjDataToMesh();

	if (Name.size() == 0)
		Name = getFileNameFromFilePath(fileName);

	if (meshes.find(Name) != meshes.end())
		Name = "mesh_" + std::to_string(meshes.size());
	
	newMesh->setName(Name);
	newMesh->fileName = fileName;

	meshes[newMesh->getName()] = newMesh;
	
	saveFEMesh((Name + std::string(".model")).c_str());
	return newMesh;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* fileName, std::string Name)
{
	std::fstream file;

	file.open(fileName, std::ios::in | std::ios::binary);
	char * buffer = new char[4];

	file.read(buffer, 4);
	int vertexCout = *(int*)buffer;
	char* vertexBuffer = new char[vertexCout * 4];
	file.read(vertexBuffer, vertexCout * 4);

	file.read(buffer, 4);
	int texCout = *(int*)buffer;
	char* texBuffer = new char[texCout * 4];
	file.read(texBuffer, texCout * 4);

	file.read(buffer, 4);
	int normCout = *(int*)buffer;
	char* normBuffer = new char[normCout * 4];
	file.read(normBuffer, normCout * 4);

	file.read(buffer, 4);
	int tangCout = *(int*)buffer;
	char* tangBuffer = new char[tangCout * 4];
	file.read(tangBuffer, tangCout * 4);

	file.read(buffer, 4);
	int indexCout = *(int*)buffer;
	char* indexBuffer = new char[indexCout * 4];
	file.read(indexBuffer, indexCout * 4);

	FEAABB meshAABB;
	for (size_t i = 0; i <= 2; i++)
	{
		file.read(buffer, 4);
		meshAABB.min[i] = *(float*)buffer;
	}

	for (size_t i = 0; i <= 2; i++)
	{
		file.read(buffer, 4);
		meshAABB.max[i] = *(float*)buffer;
	}

	file.close();

	FEMesh* newMesh = rawDataToMesh((float*)vertexBuffer, vertexCout,
		(float*)texBuffer, texCout,
		(float*)normBuffer, normCout,
		(float*)tangBuffer, tangCout,
		(int*)indexBuffer, indexCout);

	delete[] buffer;
	delete[] vertexBuffer;
	delete[] texBuffer;
	delete[] normBuffer;
	delete[] tangBuffer;
	delete[] indexBuffer;

	newMesh->AABB = meshAABB;
	newMesh->setName(Name);
	newMesh->fileName = fileName;

	meshes[newMesh->getName()] = newMesh;

	return newMesh;
}

FEPostProcess* FEResourceManager::createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name)
{
	return new FEPostProcess(getSimpleMesh("plane"), ScreenWidth, ScreenHeight, Name);
}

FEMaterial* FEResourceManager::createMaterial(std::string Name)
{
	size_t nextID = materials.size();
	if (Name.size() == 0 || materials.find(Name) != materials.end())
	{
		size_t index = 0;
		while (materials.find(Name) != materials.end() || Name.size() == 0)
		{
			index++;
			Name = "material_" + std::to_string(nextID + index);
		}
	}

	materials[Name] = new FEMaterial((int)nextID, Name);
	return materials[Name];
}

FEEntity* FEResourceManager::createEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	return new FEEntity(Mesh, Material, Name);
}

std::vector<std::string> FEResourceManager::getMaterialList()
{
	FE_MAP_TO_STR_VECTOR(materials)
}

FEMaterial* FEResourceManager::getMaterial(std::string name)
{
	if (materials.find(name) == materials.end())
		return nullptr;

	return materials[name];
}

std::string FEResourceManager::getFileNameFromFilePath(std::string filePath)
{
	for (size_t i = filePath.size() - 1; i > 0; i--)
	{
		if (filePath[i] == '\\' || filePath[i] == '/')
			return filePath.substr(i + 1, filePath.size() - 1 - i);
	}

	return std::string("");
}

std::vector<std::string> FEResourceManager::getMeshList()
{
	FE_MAP_TO_STR_VECTOR(meshes)
}

FEMesh* FEResourceManager::getMesh(std::string name)
{
	if (meshes.find(name) == meshes.end())
		return nullptr;

	return meshes[name];
}

void FEResourceManager::loadStandardMaterial()
{
	FEMaterial* newMat = createMaterial("SolidColorMaterial");
	newMat->shader = new FEShader(FESolidColorVS, FESolidColorFS);
	FocalEngine::FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	newMat->addParameter(color);
}

void FEResourceManager::clear()
{
	materials.clear();
	meshes.clear();

	loadStandardMaterial();
	loadStandardMeshes();
}

// save model raw data to FocalEngine binary file format
void FEResourceManager::saveFEMesh(const char* fileName)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	std::fstream file;
	
	file.open(fileName, std::ios::out | std::ios::binary);
	int count = objLoader.fVerC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fVerC.data(), sizeof(float) * objLoader.fVerC.size());

	count = objLoader.fTexC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fTexC.data(), sizeof(float) * objLoader.fTexC.size());

	count = objLoader.fNorC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fNorC.data(), sizeof(float) * objLoader.fNorC.size());

	count = objLoader.fTanC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fTanC.data(), sizeof(float) * objLoader.fTanC.size());

	count = objLoader.fInd.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fInd.data(), sizeof(int) * objLoader.fInd.size());

	FEAABB tempAABB(objLoader.fVerC);
	file.write((char*)&tempAABB.min[0], sizeof(float));
	file.write((char*)&tempAABB.min[1], sizeof(float));
	file.write((char*)&tempAABB.min[2], sizeof(float));

	file.write((char*)&tempAABB.max[0], sizeof(float));
	file.write((char*)&tempAABB.max[1], sizeof(float));
	file.write((char*)&tempAABB.max[2], sizeof(float));

	file.close();
}

std::vector<std::string> FEResourceManager::getTextureList()
{
	FE_MAP_TO_STR_VECTOR(textures)
}

FETexture* FEResourceManager::getTexture(std::string name)
{
	if (textures.find(name) == textures.end())
		return nullptr;

	return textures[name];
}