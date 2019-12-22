#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

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

	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vboID;
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTICES.size(), VERTICES.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	cube = new FEMesh(vaoID, VERTICES.size() / 3);
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