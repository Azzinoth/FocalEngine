#pragma once

#include "..\SubSystems\FECoreIncludes.h"

namespace FocalEngine
{
	class FEEntity;

	class FEMesh
	{	
		friend FEEntity;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes);
		~FEMesh();

		GLuint getVaoID() const;
		unsigned int getVertexCount() const;
		std::string getName();
		void setName(std::string newName);
	private:
		GLuint vaoID = -1;
		std::string name;
		unsigned int vertexCount;

		int vertexBuffers = 1;
	};
}