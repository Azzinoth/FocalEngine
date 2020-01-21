#include "FEBloomEffect.h"
using namespace FocalEngine;

FEBloomEffect::FEBloomEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "Bloom")
{
	addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, new FEShader(FEBloomEffectVS, FEBloomEffectThresholdFS)));
	stages[0]->shader->getParameter("thresholdBrightness")->updateData(1.0f);

	for (size_t i = 0; i < 1; i++)
	{
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEBloomEffectHorizontalFS)));
		stages.back()->shader->getParameter("BloomSize")->updateData(10.0f);
		// "randomness" for better effect
		stages.back()->shader->getParameter("BloomSize")->updateData(4.6f);
		/*if (i == 1)
			stages.back()->shader->getParameter("BloomSize")->updateData(6.8f);*/
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEBloomEffectVerticalFS)));
		stages.back()->shader->getParameter("BloomSize")->updateData(10.0f);
		// "randomness" for better effect
		stages.back()->shader->getParameter("BloomSize")->updateData(5.9f);
		/*if (i == 0)
			stages.back()->shader->getParameter("BloomSize")->updateData(8.0f);*/
	}

	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEWeakBloomEffectHorizontalFS)));
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEBloomEffectVS, FEWeakBloomEffectVerticalFS)));

	addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, new FEShader(FEBloomEffectVS, FEBloomEffectFinalFS)));
}

FEBloomEffect::~FEBloomEffect()
{
}