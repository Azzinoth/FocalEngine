#include "FEFXAA.h"
using namespace FocalEngine;

FEFXAA::FEFXAA(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight)
	: FEPostProcess(ScreenQuad, ScreenWidth, ScreenHeight, "FE_FXAA")
{
	addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, new FEShader(FEFXAAVS, FEFXAAFS)));
	stages.back()->shader->getParameter("FXAASpanMax")->updateData(8.0f);
	stages.back()->shader->getParameter("FXAAReduceMin")->updateData(1.0f / 128.0f);
	stages.back()->shader->getParameter("FXAAReduceMul")->updateData(0.4f);
	stages.back()->shader->getParameter("FXAATextuxelSize")->updateData(glm::vec2(1.0f / ScreenWidth, 1.0f / ScreenHeight));
}

FEFXAA::~FEFXAA()
{
}