#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

FEEditorHaloSelectionEffect* FEEditorHaloSelectionEffect::_instance = nullptr;
FEEditorHaloSelectionEffect::FEEditorHaloSelectionEffect() {}
FEEditorHaloSelectionEffect::~FEEditorHaloSelectionEffect() {}

void FEEditorHaloSelectionEffect::initializeResources()
{
	haloObjectsFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight());

	haloMaterial = RESOURCE_MANAGER.createMaterial("haloMaterial");
	RESOURCE_MANAGER.makeMaterialStandard(haloMaterial);

	HaloDrawObjectShader = RESOURCE_MANAGER.createShader("HaloDrawObjectShader", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_HaloDrawObject_VS.glsl").c_str(),
																				 RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 "E4F5165B2E1B05321A182C77"/*"HaloDrawObjectShader"*/);
	

	RESOURCE_MANAGER.makeShaderStandard(HaloDrawObjectShader);
	haloMaterial->shader = HaloDrawObjectShader;

	HaloDrawInstancedObjectShader = RESOURCE_MANAGER.createShader("HaloDrawInstancedObjectShader", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_HaloDrawObject_INSTANCED_VS.glsl").c_str(),
																								   RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   "16A2A65B2C1B013217219C67"/*"HaloDrawInstancedObjectShader"*/);
	RESOURCE_MANAGER.makeShaderStandard(HaloDrawInstancedObjectShader);

	postProcess = ENGINE.createPostProcess("selectionHaloEffect", ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	FEShader* blurShader = RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	postProcess->addStage(new FEPostProcessStage(FEPP_OWN_TEXTURE, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(haloObjectsFB->getColorAttachment());
	postProcess->stages.back()->outTexture = RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->stages.back()->outTexture = RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->stages.back()->outTexture = RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->stages.back()->outTexture = RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	HaloFinalShader = RESOURCE_MANAGER.createShader("HaloFinalShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str(),
																	   RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_HaloSelectionEffect_FS.glsl").c_str(),
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   "4AC7365B2C1B07324721A127"/*"HaloFinalShader"*/);

	postProcess->addStage(new FEPostProcessStage(FEPP_OWN_TEXTURE, HaloFinalShader));
	RESOURCE_MANAGER.makeShaderStandard(HaloFinalShader);
	postProcess->stages.back()->inTexture.push_back(RENDERER.postProcessEffects[RENDERER.postProcessEffects.size() - 1]->stages.back()->outTexture);
	postProcess->stages.back()->inTextureSource.push_back(FEPP_OWN_TEXTURE);
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[3]->outTexture);
	postProcess->stages.back()->inTextureSource.push_back(FEPP_OWN_TEXTURE);
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->inTexture[0]);
	postProcess->stages.back()->outTexture = RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight());

	RENDERER.addPostProcess(postProcess, true);
}

void FEEditorHaloSelectionEffect::reInitializeResources()
{
	delete haloObjectsFB;
	haloObjectsFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight());
	postProcess = ENGINE.createPostProcess("selectionHaloEffect", ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4);

	FEShader* blurShader = RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	postProcess->addStage(new FEPostProcessStage(FEPP_OWN_TEXTURE, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(haloObjectsFB->getColorAttachment());
	postProcess->replaceOutTexture(0, RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4));

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->replaceOutTexture(1, RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4));

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->replaceOutTexture(2, RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4));

	postProcess->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, blurShader));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	postProcess->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->outTexture);
	postProcess->replaceOutTexture(3, RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth() / 4, ENGINE.getRenderTargetHeight() / 4));

	postProcess->addStage(new FEPostProcessStage(FEPP_OWN_TEXTURE, HaloFinalShader));
	postProcess->stages.back()->inTexture.push_back(RENDERER.postProcessEffects[RENDERER.postProcessEffects.size() - 1]->stages.back()->outTexture);
	postProcess->stages.back()->inTextureSource.push_back(FEPP_OWN_TEXTURE);
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[3]->outTexture);
	postProcess->stages.back()->inTextureSource.push_back(FEPP_OWN_TEXTURE);
	postProcess->stages.back()->inTexture.push_back(postProcess->stages[0]->inTexture[0]);
	postProcess->replaceOutTexture(4, RESOURCE_MANAGER.createSameFormatTexture(RENDERER.sceneToTextureFB->getColorAttachment(), ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight()));

	RENDERER.addPostProcess(postProcess, true);
}