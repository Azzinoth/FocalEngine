#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture(std::string Name) : FEObject(FE_TEXTURE, Name)
{
	name = Name;
	getNewGLTextureID();
}

FETexture::FETexture(int Width, int Height, std::string Name) : FEObject(FE_TEXTURE, Name)
{
	name = Name;
	width = Width;
	height = Height;
	getNewGLTextureID();
	bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	unBind();
}

FETexture::FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name) : FEObject(FE_TEXTURE, Name)
{
	width = Width;
	height = Height;
	internalFormat = InternalFormat;
	format = Format;
	getNewGLTextureID();
	bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// to-do: it is needed for screen space effects but could interfere with other purposes
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	if (internalFormat == GL_RGBA16F || internalFormat == GL_RGB16F || internalFormat == GL_RGB32F || internalFormat == GL_RGBA32F)
		hdr = true;
}

void FETexture::getNewGLTextureID()
{
	FE_GL_ERROR(glGenTextures(1, &textureID));
	//LOG.add("Texture creation with textureID: " + std::to_string(textureID));
}

FETexture::~FETexture()
{
	for (size_t i = 0; i < noDeletingList.size(); i++)
	{
		if (noDeletingList[i] == textureID)
			return;
	}
	//LOG.add("Texture deletion with textureID: " + std::to_string(textureID));
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

void FETexture::GPUAllocateTeture(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{
	FE_GL_ERROR(glTexImage2D(target, level, internalformat, width, height, border, format, type, data));
#ifdef FE_GPUMEM_ALLOCATION_LOGING
	FELOG::getInstance().logError("Texture creation with width: " + std::to_string(width) + " height: " + std::to_string(height));
#endif
}

void FETexture::addToOnDeleteCallBackList(std::string objectID)
{
	callListOnDeleteFEObject.push_back(objectID);
}

void FETexture::eraseFromOnDeleteCallBackList(std::string objectID)
{
	for (size_t i = 0; i < callListOnDeleteFEObject.size(); i++)
	{
		if (callListOnDeleteFEObject[i] == objectID)
		{
			callListOnDeleteFEObject.erase(callListOnDeleteFEObject.begin() + i, callListOnDeleteFEObject.begin() + i + 1);
			break;
		}
	}
}

std::string FETexture::textureInternalFormatToString(GLint internalFormat)
{
	std::string result = "";

	if (internalFormat == GL_RGBA)
	{
		result += "GL_RGBA";
	}
	else if (internalFormat == GL_RED)
	{
		result += "GL_RED";
	}
	else if (internalFormat == GL_R16)
	{
		result += "GL_R16";
	}
	else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
	}
	else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
	}
	
	return result;
}

std::vector<GLuint> FETexture::noDeletingList = std::vector<GLuint>();
void FETexture::addToNoDeletingList(GLuint textureID)
{
	noDeletingList.push_back(textureID);
}