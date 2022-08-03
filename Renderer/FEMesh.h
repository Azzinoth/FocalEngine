#pragma once

#include "..\SubSystems\FEGeometricTools.h"
#include "..\SubSystems\FEObject.h"

namespace FocalEngine
{
	class FEEntity;
	class FEEntityInstanced;
	class FERenderer;
	class FEResourceManager;

	class FEMesh : public FEObject
	{	
		friend FEEntity;
		friend FEEntityInstanced;
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name);
		~FEMesh();

		GLuint GetVaoID() const;
		GLuint GetVertexCount() const;
		GLuint GetIndicesBufferID() const;
		GLuint GetIndicesCount() const;
		GLuint GetPositionsBufferID() const;
		GLuint GetPositionsCount() const;
		GLuint GetNormalsBufferID() const;
		GLuint GetNormalsCount() const;
		GLuint GetTangentsBufferID() const;
		GLuint GetTangentsCount() const;
		GLuint GetUVBufferID() const;
		GLuint GetUVCount() const;
		GLuint GetMaterialsIndicesBufferID() const;
		GLuint GetMaterialsIndicesCount() const;

		int GetMaterialCount() const;

		FEAABB GetAABB();
	private:
		GLuint VaoID = -1;
		GLuint IndicesBufferID = -1;
		unsigned int IndicesCount = -1;
		GLuint PositionsBufferID = -1;
		unsigned int PositionsCount = -1;
		GLuint NormalsBufferID = -1;
		unsigned int NormalsCount = -1;
		GLuint TangentsBufferID = -1;
		unsigned int TangentsCount = -1;
		GLuint UVBufferID = -1;
		unsigned int UVCount = -1;
		GLuint MaterialsIndicesBufferID = -1;
		unsigned int MaterialsIndicesCount = -1;

		unsigned int VertexCount;
		unsigned int MaterialsCount;

		int VertexAttributes = 1;
		FEAABB AABB;
	};
}