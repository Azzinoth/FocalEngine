#include "FEFramebuffer.h"
using namespace FocalEngine;

FEFramebuffer::FEFramebuffer() : FEObject(FE_FRAME_BUFFER, "unnamedFrameBuffer")
{
}

FEFramebuffer::~FEFramebuffer()
{
	FE_GL_ERROR(glDeleteFramebuffers(1, &FBO));
	for (size_t i = 0; i < ColorAttachments.size(); i++)
	{
		delete ColorAttachments[i];
		ColorAttachments[i] = nullptr;
	}
	
	delete DepthAttachment;
	DepthAttachment = nullptr;
	delete StencilAttachment;
	StencilAttachment = nullptr;
}

void FEFramebuffer::Bind()
{
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, FBO));
	bBinded = true;
}

void FEFramebuffer::UnBind()
{
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	bBinded = false;
}

FETexture* FEFramebuffer::GetColorAttachment(const size_t Index)
{
	if (Index >= ColorAttachments.size())
		return nullptr;

	return ColorAttachments[Index];
}

FETexture* FEFramebuffer::GetDepthAttachment()
{
	return DepthAttachment;
}

FETexture* FEFramebuffer::GetStencilAttachment()
{
	return StencilAttachment;
}

void FEFramebuffer::AttachTexture(const GLenum Attachment, const GLenum Textarget, FETexture* Texture)
{
	FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, Textarget, Texture->GetTextureID(), 0));
}

void FEFramebuffer::SetColorAttachment(FETexture* NewTexture, const size_t Index)
{
	if (ColorAttachments[Index] != nullptr)
		ColorAttachments[Index]->EraseFromOnDeleteCallBackList(GetObjectID());

	// This check was added because of postProcesses and how they "manage" colorAttachment of FB.
	if (NewTexture == nullptr)
	{
		ColorAttachments[Index] = nullptr;
		return;
	}

	const bool bWasBind = bBinded;
	if (!bWasBind) Bind();
	ColorAttachments[Index] = NewTexture;
	AttachTexture(GL_COLOR_ATTACHMENT0 + static_cast<int>(Index), GL_TEXTURE_2D, NewTexture);
	if (!bWasBind) UnBind();

	NewTexture->AddToOnDeleteCallBackList(GetObjectID());
}

void FEFramebuffer::SetDepthAttachment(FETexture* NewTexture)
{
	const bool bWasBind = bBinded;
	if (!bWasBind) Bind();
		DepthAttachment = NewTexture;
	AttachTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthAttachment);
	if (!bWasBind) UnBind();
}

void FEFramebuffer::SetStencilAttachment(FETexture* NewTexture)
{
	const bool bWasBind = bBinded;
	if (!bWasBind) Bind();
		StencilAttachment = NewTexture;
	AttachTexture(GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, StencilAttachment);
	if (!bWasBind) UnBind();
}

int FEFramebuffer::GetWidth()
{
	return Width;
}

int FEFramebuffer::GetHeight()
{
	return Height;
}

void FEFramebuffer::ProcessOnDeleteCallbacks(const std::string DeletingFEObject)
{
	const FEObject* object = OBJECT_MANAGER.GetFEObject(DeletingFEObject);
	if (object != nullptr && object->GetType() == FE_TEXTURE)
	{
		for (size_t i = 0; i < ColorAttachments.size(); i++)
		{
			if (ColorAttachments[i] == nullptr)
				continue;

			if (ColorAttachments[i]->GetObjectID() == DeletingFEObject)
			{
				ColorAttachments[i] = nullptr;
				return;
			}
		}
	}
}

bool FEFramebuffer::HasTexture(const std::string ObjectID)
{
	for (size_t i = 0; i < ColorAttachments.size(); i++)
	{
		if (ColorAttachments[i] != nullptr && ColorAttachments[i]->GetObjectID() == ObjectID)
			return true;
	}

	if (DepthAttachment != nullptr && DepthAttachment->GetObjectID() == ObjectID)
		return true;

	if (StencilAttachment != nullptr && StencilAttachment->GetObjectID() == ObjectID)
		return true;

	return false;
}

bool FEFramebuffer::HasTexture(const GLuint TextureID)
{
	for (size_t i = 0; i < ColorAttachments.size(); i++)
	{
		if (ColorAttachments[i] != nullptr && ColorAttachments[i]->GetTextureID() == TextureID)
			return true;
	}

	if (DepthAttachment != nullptr && DepthAttachment->GetTextureID() == TextureID)
		return true;

	if (StencilAttachment != nullptr && StencilAttachment->GetTextureID() == TextureID)
		return true;

	return false;
}