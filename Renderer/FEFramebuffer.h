#pragma once

#include "FEMaterial.h"

namespace FocalEngine
{
	enum FE_FRAMEBUFFER_ATTACHMENT
	{
		FE_COLOR_ATTACHMENT = 1,
		FE_DEPTH_ATTACHMENT = 2,
		FE_STENCIL_ATTACHMENT = 4,
	};

	class FEFramebuffer : public FEObject
	{
		friend struct FEVirtualUIComponent;
		friend class FEVirtualUIContext;
		friend class FEngine;
		friend class FEResourceManager;
		friend class FERenderer;
	public:
		~FEFramebuffer();

		void Bind();
		void UnBind();

		FETexture* GetColorAttachment(size_t Index = 0);
		FETexture* GetDepthAttachment();
		FETexture* GetStencilAttachment();

		void SetColorAttachment(FETexture* NewTexture, size_t Index = 0);
		void SetDepthAttachment(FETexture* NewTexture);
		void SetStencilAttachment(FETexture* NewTexture);

		int GetWidth();
		int GetHeight();

		bool HasTexture(std::string ObjectID);
		bool HasTexture(GLuint TextureID);
	private:
		FEFramebuffer();

		GLuint FBO = -1;
		bool bBinded = false;

		int Width = 0;
		int Height = 0;
		
		std::vector<FETexture*> ColorAttachments;
		FETexture* DepthAttachment = nullptr;
		FETexture* StencilAttachment = nullptr;

		void AttachTexture(GLenum Attachment, GLenum Textarget, FETexture* Texture);
		void ProcessOnDeleteCallbacks(std::string DeletingFEObject);
	};
}