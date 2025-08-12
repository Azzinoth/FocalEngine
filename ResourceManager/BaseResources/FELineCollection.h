#pragma once

#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FELine
	{
		glm::vec3 Begin;
		glm::vec3 End;
		glm::vec3 Color = glm::vec3(1.0f);
		float Width = 1.0f;
	};

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