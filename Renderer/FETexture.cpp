#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture(char* fileName, std::string Name)
{
	textureID = png_texture_load(fileName);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // smooth textures not blocky :)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.8f);

	setName(Name);
}

FETexture::FETexture(std::string Name)
{
	setName(Name);
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