#pragma once

#include "..\SubSystems\FECoreIncludes.h"
#include "FEAsset.h"

namespace FocalEngine
{
	enum FE_TEXTURE_MAG_FILTER
	{
		FE_NEAREST = 0,
		FE_LINEAR = 1,
	};

	class FEResourceManager;
	class FERenderer;
	class FEPostProcess;

	class FETexture : public FEAsset
	{
		friend FEResourceManager;
		friend FERenderer;
		friend FEPostProcess;
	public:
		static void GPUAllocateTeture(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data);
		FETexture(std::string Name);
		FETexture(int Width, int Height, std::string Name);
		FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name);
		~FETexture();

		GLuint getTextureID();

		std::string getFileName();

		virtual void bind(const unsigned int textureUnit = 0);
		virtual void unBind();

		GLint getInternalFormat();
		int getWidth();
		int getHeight();
	private:
		GLuint textureID = -1;
		std::string fileName = "";
		bool hdr = false;

		int width = 0;
		int height = 0;
		GLint internalFormat;
		GLenum format;
		GLuint defaultTextureUnit = -1;

		FE_TEXTURE_MAG_FILTER magFilter = FE_LINEAR;
		bool mipEnabled = true;
	};

	#define LOG FELOG::getInstance()
}