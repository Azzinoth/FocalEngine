#pragma once

#include "..\SubSystems\FECoreIncludes.h"

namespace FocalEngine
{
	class FEMesh
	{
	public:
		FEMesh(GLuint vaoID, unsigned int vertexCount);
		~FEMesh();

		GLuint getVaoID() const;
		unsigned int getVertexCount() const;
	private:
		GLuint vaoID = -1;
		unsigned int vertexCount;
	};
}