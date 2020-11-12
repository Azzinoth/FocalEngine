#pragma once

#include "..\SubSystems\FEGeometricTools.h"
#include "FEAsset.h"

namespace FocalEngine
{
	class FEEntity;
	class FEEntityInstanced;
	class FERenderer;
	class FEResourceManager;

	class FEMesh : public FEAsset
	{	
		friend FEEntity;
		friend FEEntityInstanced;
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name);
		~FEMesh();

		GLuint getVaoID() const;
		GLuint getVertexCount() const;
		GLuint getIndicesBufferID() const;
		GLuint getIndicesCount() const;
		GLuint getPositionsBufferID() const;
		GLuint getPositionsCount() const;
		GLuint getNormalsBufferID() const;
		GLuint getNormalsCount() const;
		GLuint getTangentsBufferID() const;
		GLuint getTangentsCount() const;
		GLuint getUVBufferID() const;
		GLuint getUVCount() const;
		GLuint getMaterialsIndicesBufferID() const;
		GLuint getMaterialsIndicesCount() const;

		int FEMesh::getMaterialCount() const;

		FEAABB getAABB();
	private:
		GLuint vaoID = -1;
		GLuint indicesBufferID = -1;
		unsigned int indicesCount = -1;
		GLuint positionsBufferID = -1;
		unsigned int positionsCount = -1;
		GLuint normalsBufferID = -1;
		unsigned int normalsCount = -1;
		GLuint tangentsBufferID = -1;
		unsigned int tangentsCount = -1;
		GLuint UVBufferID = -1;
		unsigned int UVCount = -1;
		GLuint materialsIndicesBufferID = -1;
		unsigned int materialsIndicesCount = -1;

		unsigned int vertexCount;
		unsigned int materialsCount;

		int vertexAttributes = 1;
		FEAABB AABB;
	};
}