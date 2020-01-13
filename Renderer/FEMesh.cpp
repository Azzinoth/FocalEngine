#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes)
{
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexBuffers = VertexBuffersTypes;
}

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB)
{
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexBuffers = VertexBuffersTypes;
	this->AABB = AABB;
}

FEMesh::~FEMesh()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &vaoID));
}

GLuint FEMesh::getVaoID() const
{
	return vaoID;
}

GLuint FEMesh::getVertexCount() const
{
	return vertexCount;
}

std::string FEMesh::getName()
{
	return name;
}

void FEMesh::setName(std::string newName)
{
	name = newName;
}

FEAABB FEMesh::getAABB()
{
	return AABB;
}