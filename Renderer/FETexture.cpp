#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture(std::string Name) : FEAsset(FE_TEXTURE, Name)
{
	name = Name;
	FE_GL_ERROR(glGenTextures(1, &textureID));
}

FETexture::FETexture(int Width, int Height, std::string Name) : FEAsset(FE_TEXTURE, Name)
{
	name = Name;
	width = Width;
	height = Height;
	FE_GL_ERROR(glGenTextures(1, &textureID));
	bind(0);
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	unBind();
}

FETexture::FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name) : FEAsset(FE_TEXTURE, Name)
{
	width = Width;
	height = Height;
	internalFormat = InternalFormat;
	format = Format;
	FE_GL_ERROR(glGenTextures(1, &textureID));
	bind(0);

	FE_GL_ERROR(glGenTextures(1, &textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// to-do: it is needed for screen space effects but could interfere with other purposes
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	if (internalFormat == GL_RGBA16F || internalFormat == GL_RGB16F || internalFormat == GL_RGB32F || internalFormat == GL_RGBA32F)
		hdr = true;
}

FETexture::~FETexture()
{
	FE_GL_ERROR(glDeleteTextures(1, &textureID));
}

GLuint FETexture::getTextureID()
{
	return textureID;
}

void FETexture::bind(const unsigned int textureUnit)
{
	defaultTextureUnit = textureUnit;
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + textureUnit));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));
}

void FETexture::unBind()
{
	if (defaultTextureUnit != -1)
	{
		FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + defaultTextureUnit));
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

std::string FETexture::getFileName()
{
	return fileName;
}

GLint FETexture::getInternalFormat()
{
	return internalFormat;
}

int FETexture::getWidth()
{
	return width;
}

int FETexture::getHeight()
{
	return height;
}