#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

FEEditorHaloSelectionEffect* FEEditorHaloSelectionEffect::Instance = nullptr;
FEEditorHaloSelectionEffect::FEEditorHaloSelectionEffect() {}
FEEditorHaloSelectionEffect::~FEEditorHaloSelectionEffect() {}

void FEEditorHaloSelectionEffect::InitializeResources()
{
	HaloObjectsFb = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());

	HaloMaterial = RESOURCE_MANAGER.CreateMaterial("haloMaterial");
	RESOURCE_MANAGER.MakeMaterialStandard(HaloMaterial);

	HaloDrawObjectShader = RESOURCE_MANAGER.CreateShader("HaloDrawObjectShader", RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_HaloDrawObject_VS.glsl").c_str(),
																				 RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 "E4F5165B2E1B05321A182C77"/*"HaloDrawObjectShader"*/);
	

	RESOURCE_MANAGER.MakeShaderStandard(HaloDrawObjectShader);
	HaloMaterial->Shader = HaloDrawObjectShader;

	HaloDrawInstancedObjectShader = RESOURCE_MANAGER.CreateShader("HaloDrawInstancedObjectShader", RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_HaloDrawObject_INSTANCED_VS.glsl").c_str(),
																								   RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   "16A2A65B2C1B013217219C67"/*"HaloDrawInstancedObjectShader"*/);
	RESOURCE_MANAGER.MakeShaderStandard(HaloDrawInstancedObjectShader);

	PostProcess = ENGINE.CreatePostProcess("selectionHaloEffect", ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	FEShader* BlurShader = RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(HaloObjectsFb->GetColorAttachment());
	PostProcess->Stages.back()->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->Stages.back()->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->Stages.back()->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->Stages.back()->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	HaloFinalShader = RESOURCE_MANAGER.CreateShader("HaloFinalShader", RESOURCE_MANAGER.LoadGLSL("CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str(),
																	   RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_HaloSelectionEffect_FS.glsl").c_str(),
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   "4AC7365B2C1B07324721A127"/*"HaloFinalShader"*/);

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, HaloFinalShader));
	RESOURCE_MANAGER.MakeShaderStandard(HaloFinalShader);
	PostProcess->Stages.back()->InTexture.push_back(RENDERER.PostProcessEffects[RENDERER.PostProcessEffects.size() - 1]->Stages.back()->OutTexture);
	PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[3]->OutTexture);
	PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->InTexture[0]);
	PostProcess->Stages.back()->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());

	RENDERER.AddPostProcess(PostProcess, true);
}

void FEEditorHaloSelectionEffect::ReInitializeResources()
{
	delete HaloObjectsFb;
	HaloObjectsFb = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
	PostProcess = ENGINE.CreatePostProcess("selectionHaloEffect", ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4);

	FEShader* BlurShader = RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(HaloObjectsFb->GetColorAttachment());
	PostProcess->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4));

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->ReplaceOutTexture(1, RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4));

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->ReplaceOutTexture(2, RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4));

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->OutTexture);
	PostProcess->ReplaceOutTexture(3, RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth() / 4, ENGINE.GetRenderTargetHeight() / 4));

	PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, HaloFinalShader));
	PostProcess->Stages.back()->InTexture.push_back(RENDERER.PostProcessEffects[RENDERER.PostProcessEffects.size() - 1]->Stages.back()->OutTexture);
	PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[3]->OutTexture);
	PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PostProcess->Stages.back()->InTexture.push_back(PostProcess->Stages[0]->InTexture[0]);
	PostProcess->ReplaceOutTexture(4, RESOURCE_MANAGER.CreateSameFormatTexture(RENDERER.SceneToTextureFB->GetColorAttachment(), ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight()));

	RENDERER.AddPostProcess(PostProcess, true);
}