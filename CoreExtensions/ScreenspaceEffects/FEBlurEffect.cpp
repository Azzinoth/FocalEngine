#include "FEBlurEffect.h"
using namespace FocalEngine;

FEBlurEffect::FEBlurEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight)
{
	addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, new FEShader(FEBlurEffectVS, FEBlurEffectThresholdFS)));
	stages[0]->shader->getParam("thresholdBrightness").updateData(1.0f);

	for (size_t i = 0; i < 4; i++)
	{
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBlurEffectVS, FEBlurEffectHorizontalFS)));
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBlurEffectVS, FEBlurEffectVerticalFS)));
	}
	addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, new FEShader(FEBlurEffectVS, FEBlurEffectFinalFS)));
}

FEBlurEffect::~FEBlurEffect()
{
}