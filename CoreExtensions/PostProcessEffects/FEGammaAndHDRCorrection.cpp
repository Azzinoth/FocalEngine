#include "FEGammaAndHDRCorrection.h"
using namespace FocalEngine;

FEGammaAndHDRCorrection::FEGammaAndHDRCorrection(FEMesh* ScreenQuad, FEShader* screenQuadShader, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "GammaAndHDR", screenQuadShader)
{
	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEGammaAndHDRVS;
	shaderBluePrints.back().fragmentShaderText = FEGammaAndHDRFS;
	shaderBluePrints.back().name = "FEGammaAndHDRShader";
}

void FEGammaAndHDRCorrection::initialize()
{
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[0].pointerToShaderStorage));
}

FEGammaAndHDRCorrection::~FEGammaAndHDRCorrection()
{
}