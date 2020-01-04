#include "FEFramebuffer.h"
using namespace FocalEngine;

FEFramebuffer::FEFramebuffer(int attachments, int Width, int Height)
{
	FE_GL_ERROR(glGenFramebuffers(1, &fbo));
	bind();

	if ((attachments & FE_COLOR_ATTACHMENT) == FE_COLOR_ATTACHMENT)
	{
		colorAttachment = new FETexture(Width, Height);
		FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->getTextureID(), 0));
	}

	if ((attachments & FE_DEPTH_ATTACHMENT) == FE_DEPTH_ATTACHMENT)
	{
		depthAttachment = new FETexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, Width, Height);
		FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment->getTextureID(), 0));

		depthAttachment->bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		depthAttachment->unBind();
	}

	if ((attachments & FE_STENCIL_ATTACHMENT) == FE_STENCIL_ATTACHMENT)
	{
		//to-do: make it correct
		stencilAttachment = new FETexture(Width, Height);
		FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilAttachment->getTextureID(), 0));
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
}

void FEFramebuffer::unBind()
{
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
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