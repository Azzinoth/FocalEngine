#pragma once

#include "..\SubSystems\FECoreIncludes.h"

namespace FocalEngine
{
	class FEEntity;

	class FEMesh
	{	
		friend FEEntity;
	public:
		FEMesh(GLuint vaoID, unsigned int vertexCount, int vertexBuffersTypes);
		~FEMesh();

		GLuint getVaoID() const;
		unsigned int getVertexCount() const;
	private:
		GLuint vaoID = -1;
		unsigned int vertexCount;

		int vertexBuffers = 1;
	};
}