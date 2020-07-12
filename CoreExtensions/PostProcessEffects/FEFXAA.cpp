#include "FEFXAA.h"
using namespace FocalEngine;

FEFXAA::FEFXAA(FEMesh* ScreenQuad, FEShader* screenQuadShader, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "FE_FXAA", screenQuadShader)
{
	shaderBluePrints.push_back(FEShaderBlueprint());
	shaderBluePrints.back().vertexShaderText = FEFXAAVS;
	shaderBluePrints.back().fragmentShaderText = FEFXAAFS;
	shaderBluePrints.back().name = "FEFXAAShader";

	screenW = ScreenWidth;
	screenH = ScreenHeight;
}

void FEFXAA::initialize()
{
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, shaderBluePrints[0].pointerToShaderStorage));
	stages.back()->shader->getParameter("FXAASpanMax")->updateData(8.0f);
	stages.back()->shader->getParameter("FXAAReduceMin")->updateData(1.0f / 128.0f);
	stages.back()->shader->getParameter("FXAAReduceMul")->updateData(0.4f);
	stages.back()->shader->getParameter("FXAATextuxelSize")->updateData(glm::vec2(1.0f / screenW, 1.0f / screenH));
}

FEFXAA::~FEFXAA()
{
}