#include "FEPostProcess.h"
using namespace FocalEngine;

FEPostProcessStage::FEPostProcessStage(int InTextureSource, FEShader* Shader)
{
	inTextureSource.push_back(InTextureSource);
	shader = Shader;
}

FEPostProcessStage::FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader)
{
	inTextureSource = std::move(InTextureSource);
	shader = Shader;
}

FEPostProcess::FEPostProcess(std::string Name)
{
	name = Name;
}

FEPostProcess::~FEPostProcess()
{
}

void FocalEngine::FEPostProcess::renderResult()
{
	screenQuadShader->start();
	screenQuadShader->loadDataToGPU();
	stages.back()->outTexture->bind(0);

	FE_GL_ERROR(glBindVertexArray(screenQuad->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	stages.back()->outTexture->unBind();
	screenQuadShader->stop();
}

FETexture* FEPostProcess::getInTexture()
{
	return finalTexture;
}

void FEPostProcess::addStage(FEPostProcessStage* newStage)
{
	if (!newStage->outTexture)
		newStage->outTexture = finalTexture;
	stages.push_back(newStage);
}

std::string FEPostProcess::getName()
{
	return name;
}

void FEPostProcess::setName(std::string newName)
{
	name = newName;
}