#include "FEGammaAndHDRCorrection.h"
using namespace FocalEngine;

FEGammaAndHDRCorrection::FEGammaAndHDRCorrection(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight)
{
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEGammaAndHDRVS, FEGammaAndHDRFS)));
}

FEGammaAndHDRCorrection::~FEGammaAndHDRCorrection()
{
}