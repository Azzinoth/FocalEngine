#include "FEBloomEffect.h"
using namespace FocalEngine;

FEBloomEffect::FEBloomEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "Bloom")
{
	addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, new FEShader(FEBloomEffectVS, FEBloomEffectThresholdFS)));
	stages[0]->shader->getParameter("thresholdBrightness")->updateData(1.0f);

	for (size_t i = 0; i < 4; i++)
	{
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEBloomEffectHorizontalFS)));
		stages.back()->shader->getParameter("BloomSize")->updateData(8.0f);
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEBloomEffectVerticalFS)));
		stages.back()->shader->getParameter("BloomSize")->updateData(8.0f);
	}

	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEWeakBloomEffectHorizontalFS)));
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEWeakBloomEffectVerticalFS)));

	addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, new FEShader(FEBloomEffectVS, FEBloomEffectFinalFS)));
}

FEBloomEffect::~FEBloomEffect()
{
}