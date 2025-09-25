#pragma once

#include "../Core/FEObject.h"
#include "../Core/FEGeometricTools.h"

namespace FocalEngine
{
	class FELineCollection : public FEObject
	{
		friend class FEEntity;
		friend class FERenderer;
		friend class FEResourceManager;

		GLuint VaoID = -1;
		GLenum BufferID = -1;
		unsigned int LineCount = 0;

		FELineCollection() : FEObject(FE_LINE_COLLECTION, "Name") {};
		FELineCollection(std::string Name) : FEObject(FE_LINE_COLLECTION, Name) {};
		FELineCollection(std::vector<FELine> Lines);
		~FELineCollection();
	public:
		GLuint GetVaoID() const;
		GLenum GetBufferID() const;
		unsigned int GetLineCount() const;

		std::vector<FELine> GetRawData() const;
	};
}