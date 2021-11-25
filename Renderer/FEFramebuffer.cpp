#include "FEFramebuffer.h"
using namespace FocalEngine;

FEFramebuffer::FEFramebuffer() : FEObject(FE_FRAME_BUFFER, "unnamedFrameBuffer")
{
}

FEFramebuffer::~FEFramebuffer()
{
	FE_GL_ERROR(glDeleteFramebuffers(1, &fbo));
	for (size_t i = 0; i < colorAttachments.size(); i++)
	{
		delete colorAttachments[i];
		colorAttachments[i] = nullptr;
	}
	
	delete depthAttachment;
	depthAttachment = nullptr;
	delete stencilAttachment;
	stencilAttachment = nullptr;
}

void FEFramebuffer::bind()
{
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	binded = true;
}

void FEFramebuffer::unBind()
{
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	binded = false;
}

FETexture* FEFramebuffer::getColorAttachment(size_t index)
{
	if (index >= colorAttachments.size())
		return nullptr;

	return colorAttachments[index];
}

FETexture* FEFramebuffer::getDepthAttachment()
{
	return depthAttachment;
}

FETexture* FEFramebuffer::getStencilAttachment()
{
	return stencilAttachment;
}

void FEFramebuffer::attachTexture(GLenum attachment, GLenum textarget, FETexture* texture)
{
	FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, texture->getTextureID(), 0));
}

void FEFramebuffer::setColorAttachment(FETexture* newTexture, size_t index)
{
	if (colorAttachments[index] != nullptr)
		colorAttachments[index]->eraseFromOnDeleteCallBackList(getObjectID());

	// This check was added because of postProcesses and how they "manage" colorAttachment of FB.
	if (newTexture == nullptr)
	{
		colorAttachments[index] = nullptr;
		return;
	}

	bool wasBind = binded;
	if (!wasBind) bind();
	colorAttachments[index] = newTexture;
	attachTexture(GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, newTexture);
	if (!wasBind) unBind();

	newTexture->addToOnDeleteCallBackList(getObjectID());
}

void FEFramebuffer::setDepthAttachment(FETexture* newTexture)
{
	bool wasBind = binded;
	if (!wasBind) bind();
		depthAttachment = newTexture;
	attachTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment);
	if (!wasBind) unBind();
}

void FEFramebuffer::setStencilAttachment(FETexture* newTexture)
{
	bool wasBind = binded;
	if (!wasBind) bind();
		stencilAttachment = newTexture;
	attachTexture(GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilAttachment);
	if (!wasBind) unBind();
}

int FEFramebuffer::getWidth()
{
	return width;
}

int FEFramebuffer::getHeight()
{
	return height;
}

void FEFramebuffer::processOnDeleteCallbacks(std::string deletingFEObject)
{
	FEObject* object = FEObjectManager::getInstance().getFEObject(deletingFEObject);
	if (object != nullptr && object->getType() == FE_TEXTURE)
	{
		for (size_t i = 0; i < colorAttachments.size(); i++)
		{
			if (colorAttachments[i] == nullptr)
				continue;

			if (colorAttachments[i]->getObjectID() == deletingFEObject)
			{
				colorAttachments[i] = nullptr;
				return;
			}
		}
	}
}

bool FEFramebuffer::hasTexture(std::string objectID)
{
	for (size_t i = 0; i < colorAttachments.size(); i++)
	{
		if (colorAttachments[i] != nullptr && colorAttachments[i]->getObjectID() == objectID)
			return true;
	}

	if (depthAttachment != nullptr && depthAttachment->getObjectID() == objectID)
		return true;

	if (stencilAttachment != nullptr && stencilAttachment->getObjectID() == objectID)
		return true;

	return false;
}

bool FEFramebuffer::hasTexture(GLuint textureID)
{
	for (size_t i = 0; i < colorAttachments.size(); i++)
	{
		if (colorAttachments[i] != nullptr && colorAttachments[i]->getTextureID() == textureID)
			return true;
	}

	if (depthAttachment != nullptr && depthAttachment->getTextureID() == textureID)
		return true;

	if (stencilAttachment != nullptr && stencilAttachment->getTextureID() == textureID)
		return true;

	return false;
}