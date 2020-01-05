#pragma once

#include "FEMaterial.h"

namespace FocalEngine
{
	enum FE_FRAMEBUFFER_ATTACHMENT
	{
		FE_COLOR_ATTACHMENT = 0,
		FE_DEPTH_ATTACHMENT = 1,
		FE_STENCIL_ATTACHMENT = 2,
	};

	class FEFramebuffer
	{
	public:
		FEFramebuffer(int attachments, int Width, int Height, bool HDR = true);
		~FEFramebuffer();

		void bind();
		void unBind();

		FETexture* getColorAttachment();
		FETexture* getDepthAttachment();
		FETexture* getStencilAttachment();

		void setColorAttachment(FETexture* newTexture);
		void setDepthAttachment(FETexture* newTexture);
		void setStencilAttachment(FETexture* newTexture);
	private:
		GLuint fbo = -1;
		bool binded = false;

		int width = 0;
		int height = 0;

		FETexture* colorAttachment = nullptr; // std::make_unique( std::unique_ptr<FETexture*>
		FETexture* depthAttachment = nullptr;
		FETexture* stencilAttachment = nullptr;

		void attachTexture(GLenum attachment, GLenum textarget, FETexture* texture);
	};
}