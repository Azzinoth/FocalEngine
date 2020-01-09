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

FEScreenQuadShader::FEScreenQuadShader() : FEShader(FEScreenQuadVS, FEScreenQuadFS)
{
}

FEPostProcess::FEPostProcess(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight, std::string Name)
{
	name = Name;
	screenWidth = ScreenWidth;
	screenHeight = ScreenHeight;
	screenQuad = ScreenQuad;
	screenQuadShader = new FEScreenQuadShader();
	intermediateFramebuffer = new FEFramebuffer(FocalEngine::FE_COLOR_ATTACHMENT, screenWidth, screenHeight);
}

FEPostProcess::~FEPostProcess()
{
}

FETexture* FEPostProcess::getInTexture()
{
	return finalTexture;
}

void FEPostProcess::setInTexture(FETexture* InTexture)
{
	inTexture = InTexture;
	finalTexture = InTexture->createSameFormatTexture();
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