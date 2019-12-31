#pragma once

#include "..\SubSystems\FECoreIncludes.h"

namespace FocalEngine
{
	enum FE_TEXTURE_MAG_FILTER
	{
		FE_NEAREST = 0,
		FE_LINEAR = 1,
	};

	class FEResourceManager;

	class FETexture
	{
		friend FEResourceManager;
	public:
		FETexture();
		~FETexture();

		GLuint getTextureID();

		std::string getName();
		void setName(std::string newName);

		virtual void bind(const unsigned int textureUnit);
		virtual void unBind();
	private:
		GLuint textureID = -1;
		std::string name = "DefaultName";

		int width = 0;
		int height = 0;
		GLuint defaultTextureUnit = -1;

		FE_TEXTURE_MAG_FILTER magFilter = FE_LINEAR;
		bool mipEnabled = true;
	};
}