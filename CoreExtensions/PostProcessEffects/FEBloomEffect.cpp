#include "FEBloomEffect.h"
using namespace FocalEngine;

FEBloomEffect::FEBloomEffect(FEMesh* ScreenQuad, FEShader* screenQuadShader, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "Bloom", screenQuadShader)
{
	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEBloomEffectThresholdFS;
	shaderBluePrints.back().name = "FEBloomEffectShader";
	
	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEBloomEffectHorizontalFS;
	shaderBluePrints.back().name = "FEBloomEffectHorizontalShader";

	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEBloomEffectVerticalFS;
	shaderBluePrints.back().name = "FEBloomEffectVerticalShader";

	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEWeakBloomEffectHorizontalFS;
	shaderBluePrints.back().name = "FEWeakBloomEffectHorizontalShader";

	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEWeakBloomEffectVerticalFS;
	shaderBluePrints.back().name = "FEWeakBloomEffectVerticalShader";

	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEBloomEffectVS;
	shaderBluePrints.back().fragmentShaderText = FEBloomEffectFinalFS;
	shaderBluePrints.back().name = "FEBloomEffectFinalShader";
}

void FEBloomEffect::initialize()
{
	addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, shaderBluePrints[0].pointerToShaderStorage));
	stages[0]->shader->getParameter("thresholdBrightness")->updateData(1.0f);

	for (size_t i = 0; i < 1; i++)
	{
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[1].pointerToShaderStorage));
		stages.back()->shader->getParameter("BloomSize")->updateData(10.0f);
		stages.back()->shader->getParameter("BloomSize")->updateData(5.0f);
		addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[2].pointerToShaderStorage));
		stages.back()->shader->getParameter("BloomSize")->updateData(10.0f);
		stages.back()->shader->getParameter("BloomSize")->updateData(5.0f);
	}

	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[3].pointerToShaderStorage));
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[4].pointerToShaderStorage));

	addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, shaderBluePrints[5].pointerToShaderStorage));
}

FEBloomEffect::~FEBloomEffect()
{
}