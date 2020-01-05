#include "FEBlurEffect.h"
using namespace FocalEngine;

FEBlurEffect::FEBlurEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight, FETexture* sceneTexture)
	: FEScreenSpaceEffect(ScreenQuad, ScreenWidth, ScreenHeight)
{
	setInTexture(sceneTexture);
	addStage(new FEScreenSpaceEffectStage(sceneTexture, new FEShader(FEBlurEffectVS, FEBlurEffectThresholdFS)));
	addStage(new FEScreenSpaceEffectStage(sceneTexture, new FEShader(FEBlurEffectVS, FEBlurEffectHorizontalFS)));
	addStage(new FEScreenSpaceEffectStage(sceneTexture, new FEShader(FEBlurEffectVS, FEBlurEffectVerticalFS)));
	addStage(new FEScreenSpaceEffectStage(sceneTexture, new FEShader(FEBlurEffectVS, FEBlurEffectFinalFS)));
}

FEBlurEffect::~FEBlurEffect()
{
}