#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture()
{
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
		glBindTexture(defaultTextureUnit, 0);
}