#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture()
{
}

FETexture::FETexture(int Width, int Height)
{
	width = Width;
	height = Height;
	glGenTextures(1, &textureID);
	bind(0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unBind();
}

FETexture::FETexture(GLint internalFormat, GLenum format, int Width, int Height)
{
	width = Width;
	height = Height;
	glGenTextures(1, &textureID);
	bind(0);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

FETexture::~FETexture()
{
	glDeleteTextures(1, &textureID);
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
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void FETexture::unBind()
{
	if (defaultTextureUnit != -1)
		glBindTexture(GL_TEXTURE_2D + defaultTextureUnit, 0);
}