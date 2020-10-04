#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name) : FEAsset(FE_MESH, Name)
{
	name = Name;
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexAttributes = VertexBuffersTypes;
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

GLuint FEMesh::getIndicesBufferID() const
{
	return indicesBufferID;
}

GLuint FEMesh::getIndicesCount() const
{
	return indicesCount;
}

GLuint FEMesh::getPositionsBufferID() const
{
	return positionsBufferID;
}

GLuint FEMesh::getPositionsCount() const
{
	return positionsCount;
}

GLuint FEMesh::getNormalsBufferID() const
{
	return normalsBufferID;
}

GLuint FEMesh::getNormalsCount() const
{
	return normalsCount;
}

GLuint FEMesh::getTangentsBufferID() const
{
	return tangentsBufferID;
}

GLuint FEMesh::getTangentsCount() const
{
	return tangentsCount;
}

GLuint FEMesh::getUVBufferID() const
{
	return UVBufferID;
}

GLuint FEMesh::getUVCount() const
{
	return UVCount;
}

GLuint FEMesh::getMaterialsIndicesBufferID() const
{
	return materialsIndicesBufferID;
}

GLuint FEMesh::getMaterialsIndicesCount() const
{
	return materialsIndicesCount;
}

int FEMesh::getMaterialCount() const
{
	return materialsCount;
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