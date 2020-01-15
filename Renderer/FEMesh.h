#pragma once

#include "..\SubSystems\FEGeometricTools.h"

namespace FocalEngine
{
	class FEEntity;
	class FEResourceManager;

	class FEMesh
	{	
		friend FEEntity;
		friend FEResourceManager;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes);
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB);
		~FEMesh();

		GLuint getVaoID() const;
		unsigned int getVertexCount() const;

		std::string getName();
		void setName(std::string newName);

		std::string getFileName();

		FEAABB getAABB();
	private:
		GLuint vaoID = -1;
		std::string name;
		std::string fileName = "";
		unsigned int vertexCount;

		int vertexBuffers = 1;
		FEAABB AABB;
	};
}