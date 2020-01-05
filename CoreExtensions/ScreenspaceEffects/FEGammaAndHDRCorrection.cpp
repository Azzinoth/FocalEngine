#include "FEGammaAndHDRCorrection.h"
using namespace FocalEngine;

FEGammaAndHDRCorrection::FEGammaAndHDRCorrection(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight, FETexture* sceneTexture)
	: FEScreenSpaceEffect(ScreenQuad, ScreenWidth, ScreenHeight)
{
	setInTexture(sceneTexture);
	addStage(new FEScreenSpaceEffectStage(sceneTexture, new FEShader(FEGammaAndHDRVS, FEGammaAndHDRFS)));
}

FEGammaAndHDRCorrection::~FEGammaAndHDRCorrection()
{
}