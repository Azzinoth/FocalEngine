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

GLuint FEMesh::GetColorBufferID() const
{
	return ColorBufferID;
}

GLuint FEMesh::GetColorCount() const
{
	return ColorCount;
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

std::vector<std::vector<glm::vec3>> FEMesh::GetTrianglePositions()
{
	float* Positions = new float[GetPositionsCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(GetPositionsBufferID(), 0, sizeof(float) * GetPositionsCount(), Positions));

	int* Indices = new int[GetIndicesCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(GetIndicesBufferID(), 0, sizeof(int) * GetIndicesCount(), Indices));

	std::vector<glm::vec3> Triangle;
	Triangle.resize(3);
	std::vector<std::vector<glm::vec3>> Result;
	int IndexCount = GetIndicesCount();
	for (size_t i = 0; i < IndexCount; i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		Triangle[0] = glm::vec3(Positions[VertexPosition], Positions[VertexPosition + 1], Positions[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		Triangle[1] = glm::vec3(Positions[VertexPosition], Positions[VertexPosition + 1], Positions[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		Triangle[2] = glm::vec3(Positions[VertexPosition], Positions[VertexPosition + 1], Positions[VertexPosition + 2]);

		Result.push_back(Triangle);
	}

	return Result;
}

std::vector<std::vector<glm::vec2>> FEMesh::GetTriangleUVs()
{
	float* UVs = new float[GetUVCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(GetUVBufferID(), 0, sizeof(float) * GetUVCount(), UVs));

	int* Indices = new int[GetIndicesCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(GetIndicesBufferID(), 0, sizeof(int) * GetIndicesCount(), Indices));

	std::vector<glm::vec2> Triangle;
	Triangle.resize(3);
	std::vector<std::vector<glm::vec2>> Result;
	int IndexCount = GetIndicesCount();
	for (size_t i = 0; i < IndexCount; i += 3)
	{
		int VertexPosition = Indices[i] * 2;
		Triangle[0] = glm::vec2(UVs[VertexPosition], UVs[VertexPosition + 1]);

		VertexPosition = Indices[i + 1] * 2;
		Triangle[1] = glm::vec2(UVs[VertexPosition], UVs[VertexPosition + 1]);

		VertexPosition = Indices[i + 2] * 2;
		Triangle[2] = glm::vec2(UVs[VertexPosition], UVs[VertexPosition + 1]);

		Result.push_back(Triangle);
	}

	return Result;
}