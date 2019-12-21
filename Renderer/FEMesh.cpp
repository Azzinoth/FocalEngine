#include "FEMesh.h"

FocalEngine::FEMesh::FEMesh(GLuint vaoID, unsigned int vertexCount)
{
	this->vaoID = vaoID;
	this->vertexCount = vertexCount;
}

FocalEngine::FEMesh::~FEMesh()
{
	glDeleteVertexArrays(1, &vaoID);
	//glDeleteBuffers(vbos.size(), vbos.data());
}

GLuint FocalEngine::FEMesh::getVaoID() const
{
	return vaoID;
}

GLuint FocalEngine::FEMesh::getVertexCount() const
{
	return vertexCount;
}