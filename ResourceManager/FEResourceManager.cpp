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
	float minX = -1.0f;
	float minY = -1.0f;
	float minZ = -1.0f;

	float maxX = 1.0f;
	float maxY = 1.0f;
	float maxZ = 1.0f;

	std::vector<float> VERTICES = {
		maxX, maxY, minZ, // 3
		maxX, minY, minZ, // 2
		minX, minY, minZ, // 1
		minX, minY, minZ, // 1
		minX, maxY, minZ, // 0
		maxX, maxY, minZ, // 3

		minX, maxY, minZ, // 0
		minX, minY, minZ, // 1
		minX, minY, maxZ, // 4
		minX, minY, maxZ, // 4
		minX, maxY, maxZ, // 5
		minX, maxY, minZ, // 0

		maxX, maxY, maxZ, // 7
		maxX, minY, maxZ, // 6
		maxX, minY, minZ, // 2
		maxX, minY, minZ, // 2
		maxX, maxY, minZ, // 3
		maxX, maxY, maxZ, // 7

		minX, maxY, maxZ, // 5
		minX, minY, maxZ, // 4
		maxX, maxY, maxZ, // 7
		maxX, maxY, maxZ, // 7
		minX, minY, maxZ, // 4
		maxX, minY, maxZ, // 6

		maxX, maxY, minZ, // 3
		minX, maxY, minZ, // 0
		minX, maxY, maxZ, // 5
		minX, maxY, maxZ, // 5
		maxX, maxY, maxZ, // 7
		maxX, maxY, minZ, // 3

		minX, minY, minZ, // 1
		maxX, minY, minZ, // 2
		maxX, minY, maxZ, // 6
		maxX, minY, maxZ, // 6
		minX, minY, maxZ, // 4
		minX, minY, minZ  // 1
	};

	std::vector<float> normals;
	for (size_t i = 0; i < VERTICES.size() / 3; i++)
	{
		normals.push_back(0.0f);
		normals.push_back(1.0f);
		normals.push_back(0.0f);
	}

	cube = rawDataToMesh(VERTICES, normals);
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

	return nullptr;
}

FEMesh* FEResourceManager::loadObjMeshData(const char* fileName)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	objLoader.readFile(fileName);

	return rawObjDataToMesh();
}