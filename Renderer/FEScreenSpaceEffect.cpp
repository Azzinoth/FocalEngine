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

FETexture* FEScreenSpaceEffect::getFinalTexture()
{
	return finalTexture;
}

void FEScreenSpaceEffect::setFinalTexture(FETexture* FinalTexture)
{
	finalTexture = FinalTexture;
}

void FEScreenSpaceEffect::render()
{
	if (!finalTexture)
		return;

	for (size_t i = 0; i < stages.size(); i++)
	{
		stages[i]->shader->start();
		stages[i]->shader->loadDataToGPU();
		stages[i]->inTexture->bind(0);

		FE_GL_ERROR(glBindVertexArray(screenQuad->getVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		stages[i]->inTexture->unBind();
		stages[i]->shader->stop();
	}

	screenQuadShader->start();
	screenQuadShader->loadDataToGPU();
	finalTexture->bind(0);

	FE_GL_ERROR(glBindVertexArray(screenQuad->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	finalTexture->unBind();
	screenQuadShader->stop();
}

void FEScreenSpaceEffect::addStage(FEScreenSpaceEffectStage* newStage)
{
	if (!newStage->outTexture)
		newStage->outTexture = finalTexture;
	stages.push_back(newStage);
}