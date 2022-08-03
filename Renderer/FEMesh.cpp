#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name) : FEObject(FE_MESH, Name)
{
	SetName(Name);
	this->VaoID = VaoID;
	this->VertexCount = VertexCount;
	VertexAttributes = VertexBuffersTypes;
	this->AABB = AABB;
}

FEMesh::~FEMesh()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &VaoID));
}

GLuint FEMesh::GetVaoID() const
{
	return VaoID;
}

GLuint FEMesh::GetVertexCount() const
{
	return VertexCount;
}

GLuint FEMesh::GetIndicesBufferID() const
{
	return IndicesBufferID;
}

GLuint FEMesh::GetIndicesCount() const
{
	return IndicesCount;
}

GLuint FEMesh::GetPositionsBufferID() const
{
	return PositionsBufferID;
}

GLuint FEMesh::GetPositionsCount() const
{
	return PositionsCount;
}

GLuint FEMesh::GetNormalsBufferID() const
{
	return NormalsBufferID;
}

GLuint FEMesh::GetNormalsCount() const
{
	return NormalsCount;
}

GLuint FEMesh::GetTangentsBufferID() const
{
	return TangentsBufferID;
}

GLuint FEMesh::GetTangentsCount() const
{
	return TangentsCount;
}

GLuint FEMesh::GetUVBufferID() const
{
	return UVBufferID;
}

GLuint FEMesh::GetUVCount() const
{
	return UVCount;
}

GLuint FEMesh::GetMaterialsIndicesBufferID() const
{
	return MaterialsIndicesBufferID;
}

GLuint FEMesh::GetMaterialsIndicesCount() const
{
	return MaterialsIndicesCount;
}

int FEMesh::GetMaterialCount() const
{
	return MaterialsCount;
}

FEAABB FEMesh::GetAABB()
{
	return AABB;
}