#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture()
{
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

FETexture::FETexture(GLint internalFormat, GLenum format, int Width, int Height)
{
	width = Width;
	height = Height;
	FE_GL_ERROR(glGenTextures(1, &textureID));
	bind(0);

	FE_GL_ERROR(glGenTextures(1, &textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
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
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D + defaultTextureUnit, 0));
}