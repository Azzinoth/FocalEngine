#include "FEScreenSpaceEffect.h"
using namespace FocalEngine;

FEScreenQuadShader::FEScreenQuadShader() : FEShader(FEScreenQuadVS, FEScreenQuadFS)
{

}

FEScreenSpaceEffect::FEScreenSpaceEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
{
	screenWidth = ScreenWidth;
	screenHeight = ScreenHeight;
	screenQuad = ScreenQuad;
	screenQuadShader = new FEScreenQuadShader();
	intermediateFramebuffer = new FEFramebuffer(FocalEngine::FE_COLOR_ATTACHMENT, screenWidth, screenHeight);
}

FEScreenSpaceEffect::~FEScreenSpaceEffect()
{
}

FETexture* FEScreenSpaceEffect::getInTexture()
{
	return finalTexture;
}

void FEScreenSpaceEffect::setInTexture(FETexture* InTexture)
{
	inTexture = InTexture;
	finalTexture = new FETexture(InTexture->internalFormat, InTexture->format, InTexture->width, InTexture->height);
}

void FEScreenSpaceEffect::addStage(FEScreenSpaceEffectStage* newStage)
{
	if (!newStage->outTexture)
		newStage->outTexture = finalTexture;
	stages.push_back(newStage);
}