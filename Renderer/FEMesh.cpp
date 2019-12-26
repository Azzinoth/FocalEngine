#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint vaoID, unsigned int vertexCount, int vertexBuffersTypes)
{
	this->vaoID = vaoID;
	this->vertexCount = vertexCount;
	this->vertexBuffers = vertexBuffersTypes;
}

FEMesh::~FEMesh()
{
	glDeleteVertexArrays(1, &vaoID);
}

GLuint FEMesh::getVaoID() const
{
	return vaoID;
}

GLuint FEMesh::getVertexCount() const
{
	return vertexCount;
}