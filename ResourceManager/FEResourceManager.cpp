#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions)
{
	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vboID;
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return new FEMesh(vaoID, positions.size() / 3, FE_POSITION);
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals)
{
	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vboID;
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normals
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return new FEMesh(vaoID, positions.size() / 3, FE_POSITION | FE_NORMAL);
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index)
{
	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vboID;
	// index
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index.size(), index.data(), GL_STATIC_DRAW);

	// verCoords
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normals
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// tangents
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangents.size(), tangents.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// UV
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UV.size(), UV.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return new FEMesh(vaoID, index.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX);
}

FEMesh* FEResourceManager::rawObjDataToMesh()
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vboID;
	// index
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * objLoader.fInd.size(), objLoader.fInd.data(), GL_STATIC_DRAW);

	// verCoords
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fVerC.size(), objLoader.fVerC.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normals
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fNorC.size(), objLoader.fNorC.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// tangents
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTanC.size(), objLoader.fTanC.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// UV
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTexC.size(), objLoader.fTexC.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return new FEMesh(vaoID, objLoader.fInd.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX);
}

FEResourceManager::FEResourceManager()
{
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

	cube = rawDataToMesh(cubePositions, cubeNormals, cubeTangents, cubeUV, cubeIndices);

	std::vector<int> planeIndices = {
		0, 1, 2, 3, 0, 2
	};

	std::vector<float> planePositions = {
		-0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f
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
	
	plane = rawDataToMesh(planePositions, planeNormals, planeTangents, planeUV, planeIndices);
}

FEResourceManager::~FEResourceManager()
{
	
}

FEMesh* FEResourceManager::getSimpleMesh(std::string meshName)
{
	if (meshName == std::string("cube"))
	{
		return cube;
	}

	if (meshName == std::string("plane"))
	{
		return plane;
	}

	return nullptr;
}

FEMesh* FEResourceManager::loadObjMeshData(const char* fileName)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	objLoader.readFile(fileName);

	return rawObjDataToMesh();
}