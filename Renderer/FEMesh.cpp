#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint vaoID, unsigned int vertexCount)
{
	this->vaoID = vaoID;
	this->vertexCount = vertexCount;
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