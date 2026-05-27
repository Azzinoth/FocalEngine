#include "FELineCollection.h"
using namespace FocalEngine;

FELineCollection::FELineCollection(std::vector<FELine> Lines) : FEObject(FE_LINE_COLLECTION, "Unnamed Line Collection")
{
	if (Lines.empty())
		return;

	const float QuadVertices[] = {
		0.0f,  -0.5f,  0.0f,
		1.0f,  -0.5f,  1.0f,
		1.0f,  0.5f,   1.0f,

		0.0f,  -0.5f,  0.0f,
		1.0f,  0.5f,   1.0f,
		0.0f,  0.5f,   0.0f,
	};
	glGenVertexArrays(1, &VaoID);
	glBindVertexArray(VaoID);

	unsigned int QuadVBO;
	glGenBuffers(1, &QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	glGenBuffers(1, &BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, BufferID);
	glBufferData(GL_ARRAY_BUFFER, Lines.size() * sizeof(FELine), Lines.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), static_cast<void*>(nullptr));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(6 * sizeof(float)));
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(9 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	LineCount = static_cast<unsigned int>(Lines.size());
}

GLuint FELineCollection::GetVaoID() const
{
	return VaoID;
}

GLenum FELineCollection::GetBufferID() const
{
	return BufferID;
}

unsigned int FELineCollection::GetLineCount() const
{
	return LineCount;
}

FELineCollection::~FELineCollection()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &VaoID));
}

std::vector<FELine> FELineCollection::GetRawData() const
{
	std::vector<FELine> Result;
	if (BufferID == GLenum(-1) || LineCount == 0)
		return Result;

	Result.resize(LineCount);
	FE_GL_ERROR(glGetNamedBufferSubData(BufferID, 0, sizeof(FELine) * LineCount, Result.data()));

	return Result;
}