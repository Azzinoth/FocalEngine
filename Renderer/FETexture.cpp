#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture()
{
	FE_GL_ERROR(glGenTextures(1, &textureID));
}

FETexture::FETexture(int Width, int Height)
{
	width = Width;
	height = Height;
	FE_GL_ERROR(glGenTextures(1, &textureID));
	bind(0);
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	unBind();
}

FETexture::FETexture(GLint InternalFormat, GLenum Format, int Width, int Height)
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
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/GL_LINEAR));
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

std::string FETexture::getName()
{
	return name;
}

void FETexture::setName(std::string newName)
{
	name = newName;
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

FETexture* FETexture::createSameFormatTexture(int differentW, int differentH)
{
	if (differentW == 0 && differentH == 0)
		return new FETexture(internalFormat, format, width, height);

	if (differentW != 0 && differentH == 0)
		return new FETexture(internalFormat, format, differentW, height);

	if (differentW == 0 && differentH != 0)
		return new FETexture(internalFormat, format, width, differentH);

	return new FETexture(internalFormat, format, differentW, differentH);
}

std::string FETexture::getFileName()
{
	return fileName;
}