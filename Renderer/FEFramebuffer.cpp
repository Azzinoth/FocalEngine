#include "FEFramebuffer.h"
using namespace FocalEngine;

FEFramebuffer::FEFramebuffer(int attachments, int Width, int Height, bool HDR)
{
	width = Width;
	height = Height;

	FE_GL_ERROR(glGenFramebuffers(1, &fbo));
	bind();

	if (attachments & FE_COLOR_ATTACHMENT)
	{
		HDR ? colorAttachment = new FETexture(GL_RGBA16F, GL_RGBA, Width, Height) : colorAttachment = new FETexture(Width, Height);
		// Allocate the mipmaps
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment);
	}

	if (attachments & FE_DEPTH_ATTACHMENT)
	{
		depthAttachment = new FETexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, Width, Height);
		attachTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment);

		// if only DEPTH_ATTACHMENT
		if (!(attachments & FE_COLOR_ATTACHMENT))
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		depthAttachment->bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		depthAttachment->unBind();
	}

	if (attachments & FE_STENCIL_ATTACHMENT)
	{
		//to-do: make it correct
		stencilAttachment = new FETexture(Width, Height);
		attachTexture(GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilAttachment);
	}

	unBind();
}

FEFramebuffer::~FEFramebuffer()
{
	FE_GL_ERROR(glDeleteFramebuffers(1, &fbo));
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

FETexture* FEFramebuffer::getColorAttachment()
{
	return colorAttachment;
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

void FEFramebuffer::setColorAttachment(FETexture* newTexture)
{
	bool wasBind = binded;
	if (!wasBind) bind();
	colorAttachment = newTexture;
	attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment);
	if (!wasBind) unBind();
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