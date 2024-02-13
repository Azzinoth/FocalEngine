#include "FEngine.h"
using namespace FocalEngine;

FEngine* FEngine::Instance = nullptr;
FE_RENDER_TARGET_MODE FEngine::RenderTargetMode = FE_GLFW_MODE;
int FEngine::RenderTargetXShift = 0;
int FEngine::RenderTargetYShift = 0;

FEngine::FEngine()
{
}

FEngine::~FEngine()
{
}

bool FEngine::IsNotTerminated()
{
	return APPLICATION.IsNotTerminated();
}

void FEngine::BeginFrame(const bool InternalCall)
{
	if (!APPLICATION.IsNotTerminated())
		return;

	if (!InternalCall)
		TIME.BeginTimeStamp();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	APPLICATION.BeginFrame();
	if (APPLICATION.GetMainWindow() == nullptr)
		return;

	APPLICATION.GetMainWindow()->BeginFrame();

#ifdef FE_DEBUG_ENABLED
	std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShadersList();
	const std::vector<std::string> TempList = RESOURCE_MANAGER.GetStandardShadersList();
	for (size_t i = 0; i < TempList.size(); i++)
	{
		ShaderList.push_back(TempList[i]);
	}

	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		if (RESOURCE_MANAGER.GetShader(ShaderList[i])->IsDebugRequest())
		{
			RESOURCE_MANAGER.GetShader(ShaderList[i])->ThisFrameDebugBind = 0;
		}
	}
#endif
}

void FEngine::Render(const bool InternalCall)
{
	RENDERER.EngineMainCamera = ENGINE.CurrentCamera;
	RENDERER.MouseRay = ENGINE.ConstructMouseRay();
	ENGINE.CurrentCamera->Move(static_cast<float>(CPUTime + GPUTime));
	RENDERER.Render(CurrentCamera);

	if (bVRActive)
	{
		OpenXR_MANAGER.Update();
		glViewport(static_cast<GLint>(0.0),
					static_cast<GLint>(0.0),
					static_cast<GLsizei>(ENGINE.GetRenderTargetWidth()),
					static_cast<GLsizei>(ENGINE.GetRenderTargetHeight()));
	}

	APPLICATION.GetMainWindow()->Render();

	if (!InternalCall) CPUTime = TIME.EndTimeStamp();
}

void FEngine::EndFrame(const bool InternalCall)
{
	if (!InternalCall) TIME.BeginTimeStamp();
	APPLICATION.GetMainWindow()->EndFrame();
	APPLICATION.EndFrame();
	if (!InternalCall) GPUTime = TIME.EndTimeStamp();
}

void FEngine::InitWindow(const int Width, const int Height, std::string WindowTitle)
{
	WindowW = Width;
	WindowH = Height;
	this->WindowTitle = WindowTitle;

	APPLICATION.AddWindow(Width, Height, WindowTitle);
	APPLICATION.GetMainWindow()->AddOnResizeCallback(&FEngine::WindowResizeCallback);
	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(&FEngine::MouseButtonCallback);
	APPLICATION.GetMainWindow()->AddOnMouseMoveCallback(&FEngine::MouseMoveCallback);
	APPLICATION.GetMainWindow()->AddOnKeyCallback(&FEngine::KeyButtonCallback);
	APPLICATION.GetMainWindow()->AddOnDropCallback(&FEngine::DropCallback);
	APPLICATION.GetMainWindow()->AddOnScrollCallback(&FEngine::MouseScrollCallback);

	SetClearColor(DefaultGammaCorrectedClearColor);

	CurrentCamera = new FEFreeCamera("mainCamera");
	int FinalWidth, FinalHeight;
	APPLICATION.GetMainWindow()->GetSize(&FinalWidth, &FinalHeight);
	
	WindowW = FinalWidth;
	WindowH = FinalHeight;
	RenderTargetW = FinalWidth;
	RenderTargetH = FinalHeight;
	CurrentCamera->SetAspectRatio(static_cast<float>(GetRenderTargetWidth()) / static_cast<float>(GetRenderTargetHeight()));

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	// tessellation parameter
	FE_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));

	RENDERER.Init();
	RENDERER.InstancedLineShader = RESOURCE_MANAGER.CreateShader("instancedLine", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str()).c_str(),
																				  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(RENDERER.InstancedLineShader->GetObjectID());
	RENDERER.InstancedLineShader->SetID("7E0826291010377D564F6115"/*"instancedLine"*/);
	RESOURCE_MANAGER.Shaders[RENDERER.InstancedLineShader->GetObjectID()] = RENDERER.InstancedLineShader;

	FEShader* FEScreenQuadShader = RESOURCE_MANAGER.CreateShader("FEScreenQuadShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(FEScreenQuadShader->GetObjectID());
	FEScreenQuadShader->SetID("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	RESOURCE_MANAGER.Shaders[FEScreenQuadShader->GetObjectID()] = FEScreenQuadShader;
	RESOURCE_MANAGER.MakeShaderStandard(FEScreenQuadShader);

	RENDERER.StandardFBInit(GetRenderTargetWidth(), GetRenderTargetHeight());
	
	// ************************************ Bloom ************************************
	FEPostProcess* BloomEffect = ENGINE.CreatePostProcess("bloom", static_cast<int>(GetRenderTargetWidth() / 4.0f), static_cast<int>(GetRenderTargetHeight() / 4.0f));
	BloomEffect->SetID("451C48791871283D372C5938"/*"bloom"*/);

	FEShader* BloomThresholdShader =
		RESOURCE_MANAGER.CreateShader("FEBloomThreshold", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
														  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomThreshold_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(BloomThresholdShader->GetObjectID());
	BloomThresholdShader->SetID("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/);
	RESOURCE_MANAGER.Shaders[BloomThresholdShader->GetObjectID()] = BloomThresholdShader;

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_SCENE_HDR_COLOR, BloomThresholdShader));
	BloomEffect->Stages[0]->Shader->UpdateParameterData("thresholdBrightness", 1.0f);

	FEShader* BloomBlurShader =
		RESOURCE_MANAGER.CreateShader("FEBloomBlur", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
												     RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomBlur_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(BloomBlurShader->GetObjectID());
	BloomBlurShader->SetID("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	RESOURCE_MANAGER.Shaders[BloomBlurShader->GetObjectID()] = BloomBlurShader;

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));
	
	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	FEShader* BloomCompositionShader =
		RESOURCE_MANAGER.CreateShader("FEBloomComposition", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
														    RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomComposition_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(BloomCompositionShader->GetObjectID());
	BloomCompositionShader->SetID("1833272551376C2E5645200E"/*"FEBloomComposition"*/);
	RESOURCE_MANAGER.Shaders[BloomCompositionShader->GetObjectID()] = BloomCompositionShader;

	BloomEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_HDR_COLOR}, BloomCompositionShader));

	RENDERER.AddPostProcess(BloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ gammaHDR ************************************
	FEPostProcess* GammaHDR = ENGINE.CreatePostProcess("GammaAndHDR", GetRenderTargetWidth(), GetRenderTargetHeight());
	GammaHDR->SetID("2374462A7B0E78141B5F5D79"/*"GammaAndHDR"*/);

	FEShader* GammaHDRShader =
		RESOURCE_MANAGER.CreateShader("FEGammaAndHDRShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_VS.glsl").c_str()).c_str(),
															 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(GammaHDRShader->GetObjectID());
	GammaHDRShader->SetID("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/);
	RESOURCE_MANAGER.Shaders[GammaHDRShader->GetObjectID()] = GammaHDRShader;

	GammaHDR->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, GammaHDRShader));
	RENDERER.AddPostProcess(GammaHDR);
	// ************************************ gammaHDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.CreatePostProcess("FE_FXAA", GetRenderTargetWidth(), GetRenderTargetHeight());
	FEFXAAEffect->SetID("0A3F10643F06525D70016070"/*"FE_FXAA"*/);

	FEShader* FEFXAAShader =
		RESOURCE_MANAGER.CreateShader("FEFXAAShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_VS.glsl").c_str()).c_str(),
													  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(FEFXAAShader->GetObjectID());
	FEFXAAShader->SetID("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/);
	RESOURCE_MANAGER.Shaders[FEFXAAShader->GetObjectID()] = FEFXAAShader;

	FEFXAAEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FEFXAAShader));
	FEFXAAEffect->Stages.back()->Shader->UpdateParameterData("FXAASpanMax", 8.0f);
	FEFXAAEffect->Stages.back()->Shader->UpdateParameterData("FXAAReduceMin", 1.0f / 128.0f);
	FEFXAAEffect->Stages.back()->Shader->UpdateParameterData("FXAAReduceMul", 0.4f);
	FEFXAAEffect->Stages.back()->Shader->UpdateParameterData("FXAATextuxelSize", glm::vec2(1.0f / GetRenderTargetWidth(), 1.0f / GetRenderTargetHeight()));
	RENDERER.AddPostProcess(FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, GetRenderTargetWidth(), GetRenderTargetHeight()));

	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FEPostProcess* DOFEffect = ENGINE.CreatePostProcess("DOF");
	DOFEffect->SetID("217C4E80482B6C650D7B492F"/*"DOF"*/);

	FEShader* DOFShader = RESOURCE_MANAGER.CreateShader("DOF", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_VS.glsl").c_str()).c_str(),
															   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(DOFShader->GetObjectID());
	DOFShader->SetID("7800253C244442155D0F3C7B"/*"DOF"*/);
	RESOURCE_MANAGER.Shaders[DOFShader->GetObjectID()] = DOFShader;

	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	DOFEffect->Stages.back()->Shader->UpdateParameterData("blurSize", 2.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("depthThreshold", 0.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("depthThresholdFar", 9000.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("zNear", 0.1f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("zFar", 5000.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("intMult", 100.0f);
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	DOFEffect->Stages.back()->Shader->UpdateParameterData("blurSize", 2.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("depthThreshold", 0.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("depthThresholdFar", 9000.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("zNear", 0.1f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("zFar", 5000.0f);
	DOFEffect->Stages.back()->Shader->UpdateParameterData("intMult", 100.0f);
	RENDERER.AddPostProcess(DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ chromaticAberrationEffect ************************************
	FEPostProcess* ChromaticAberrationEffect = ENGINE.CreatePostProcess("chromaticAberration");
	ChromaticAberrationEffect->SetID("506D804162647749060C3E68"/*"chromaticAberration"*/);

	FEShader* ChromaticAberrationShader = RESOURCE_MANAGER.CreateShader("chromaticAberrationShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_VS.glsl").c_str()).c_str(),
																									 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(ChromaticAberrationShader->GetObjectID());
	ChromaticAberrationShader->SetID("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/);
	RESOURCE_MANAGER.Shaders[ChromaticAberrationShader->GetObjectID()] = ChromaticAberrationShader;
	
	ChromaticAberrationEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 }, ChromaticAberrationShader));
	ChromaticAberrationEffect->Stages.back()->Shader->UpdateParameterData("intensity", 1.0f);
	RENDERER.AddPostProcess(ChromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, GetRenderTargetWidth(), GetRenderTargetHeight()));
	// ************************************ chromaticAberrationEffect END ************************************

	// ************************************ SSAO ************************************
	FEShader* FESSAOShader = RESOURCE_MANAGER.CreateShader("FESSAOShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_VS.glsl").c_str()).c_str(),
																		   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_FS.glsl").c_str()).c_str());

	RESOURCE_MANAGER.Shaders.erase(FESSAOShader->GetObjectID());
	FESSAOShader->SetID("1037115B676E383E36345079"/*"FESSAOShader"*/);
	RESOURCE_MANAGER.Shaders[FESSAOShader->GetObjectID()] = FESSAOShader;

	RESOURCE_MANAGER.MakeShaderStandard(FESSAOShader);

	FEShader* FESSAOBlurShader = RESOURCE_MANAGER.CreateShader("FESSAOBlurShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																				   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_Blur_FS.glsl").c_str()).c_str());

	RESOURCE_MANAGER.Shaders.erase(FESSAOBlurShader->GetObjectID());
	FESSAOBlurShader->SetID("0B5770660B6970800D776542"/*"FESSAOBlurShader"*/);
	RESOURCE_MANAGER.Shaders[FESSAOBlurShader->GetObjectID()] = FESSAOBlurShader;

	RESOURCE_MANAGER.MakeShaderStandard(FESSAOBlurShader);
	// ************************************ SSAO END ************************************

	RENDERER.ShadowMapMaterial = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterial");
	RENDERER.ShadowMapMaterial->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_VS.glsl").c_str()).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str()).c_str());
	RENDERER.ShadowMapMaterial->Shader->SetID("7C41565B2E2B05321A182D89"/*"FEShadowMapShader"*/);
	
	RESOURCE_MANAGER.MakeShaderStandard(RENDERER.ShadowMapMaterial->Shader);
	RESOURCE_MANAGER.MakeMaterialStandard(RENDERER.ShadowMapMaterial);

	RENDERER.ShadowMapMaterialInstanced = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterialInstanced");
	RENDERER.ShadowMapMaterialInstanced->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShaderInstanced", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_INSTANCED_VS.glsl").c_str()).c_str(),
																											  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str()).c_str());
	RENDERER.ShadowMapMaterialInstanced->Shader->SetID("5634765B2E2A05321A182D1A"/*"FEShadowMapShaderInstanced"*/);
	
	RESOURCE_MANAGER.MakeShaderStandard(RENDERER.ShadowMapMaterialInstanced->Shader);
	RESOURCE_MANAGER.MakeMaterialStandard(RENDERER.ShadowMapMaterialInstanced);
}

void FEngine::SetWindowCaption(const std::string NewCaption)
{
	if (APPLICATION.GetMainWindow() != nullptr)
		APPLICATION.GetMainWindow()->SetTitle(NewCaption);
}

void FEngine::AddWindowResizeCallback(void(*Func)(int, int))
{
	if (Func != nullptr)
		ClientWindowResizeCallbacks.push_back(Func);
}

void FEngine::AddWindowCloseCallback(void(*Func)())
{
	APPLICATION.GetMainWindow()->AddOnCloseCallback(Func);
}

void FEngine::AddKeyCallback(void(*Func)(int, int, int, int))
{
	if (Func != nullptr)
		ClientKeyButtonCallbacks.push_back(Func);
}

void FEngine::AddMouseButtonCallback(void(*Func)(int, int, int))
{
	if (Func != nullptr)
		ClientMouseButtonCallbacks.push_back(Func);
}

void FEngine::AddMouseMoveCallback(void(*Func)(double, double))
{
	if (Func != nullptr)
		ClientMouseMoveCallbacks.push_back(Func);
}

void FEngine::WindowResizeCallback(const int Width, const int Height)
{
	ENGINE.WindowW = Width;
	ENGINE.WindowH = Height;

	if (RenderTargetMode == FE_GLFW_MODE)
	{
		ENGINE.RenderTargetW = Width;
		ENGINE.RenderTargetH = Height;

		RenderTargetResize();
	}

	for (size_t i = 0; i < ENGINE.ClientWindowResizeCallbacks.size(); i++)
	{
		if (ENGINE.ClientWindowResizeCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientWindowResizeCallbacks[i](ENGINE.WindowW, ENGINE.WindowH);
	}
}

void FEngine::MouseButtonCallback(const int Button, const int Action, const int Mods)
{
	for (size_t i = 0; i < ENGINE.ClientMouseButtonCallbacks.size(); i++)
	{
		if (ENGINE.ClientMouseButtonCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientMouseButtonCallbacks[i](Button, Action, Mods);
	}
}

void FEngine::MouseMoveCallback(double Xpos, double Ypos)
{
	if (RenderTargetMode == FE_CUSTOM_MODE)
	{
		Xpos -= RenderTargetXShift;
		Ypos -= RenderTargetYShift;
	}

	for (size_t i = 0; i < ENGINE.ClientMouseMoveCallbacks.size(); i++)
	{
		if (ENGINE.ClientMouseMoveCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientMouseMoveCallbacks[i](Xpos, Ypos);
	}

	ENGINE.CurrentCamera->MouseMoveInput(Xpos, Ypos);

	ENGINE.MouseX = Xpos;
	ENGINE.MouseY = Ypos;
}

void FEngine::KeyButtonCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	for (size_t i = 0; i < ENGINE.ClientKeyButtonCallbacks.size(); i++)
	{
		if (ENGINE.ClientKeyButtonCallbacks[i] == nullptr)
			continue;
		ENGINE.ClientKeyButtonCallbacks[i](Key, Scancode, Action, Mods);
	}

	ENGINE.CurrentCamera->KeyboardInput(Key, Scancode, Action, Mods);
}

void FEngine::SetCamera(FEBasicCamera* NewCamera)
{
	CurrentCamera = NewCamera;
}

FEBasicCamera* FEngine::GetCamera()
{
	return CurrentCamera;
}

int FEngine::GetWindowWidth()
{
	return WindowW;
}

int FEngine::GetWindowHeight()
{
	return WindowH;
}

void FEngine::RenderTo(FEFramebuffer* RenderTo)
{
	RenderTo->Bind();
	BeginFrame(true);
	Render(true);
	RenderTo->UnBind();
}

double FEngine::GetCpuTime()
{
	return CPUTime;
}

double FEngine::GetGpuTime()
{
	return GPUTime;
}

FEPostProcess* FEngine::CreatePostProcess(const std::string Name, int ScreenWidth, int ScreenHeight)
{
	if (ScreenWidth < 2 || ScreenHeight < 2)
	{
		ScreenWidth = GetRenderTargetWidth();
		ScreenHeight = GetRenderTargetHeight();
	}

	return RESOURCE_MANAGER.CreatePostProcess(ScreenWidth, ScreenHeight, Name);
}

void FEngine::Terminate()
{
	APPLICATION.Close();
}

void FEngine::TakeScreenshot(const char* FileName)
{
	RENDERER.TakeScreenshot(FileName, GetRenderTargetWidth(), GetRenderTargetHeight());
}

void FEngine::ResetCamera()
{
	CurrentCamera->Reset();
	CurrentCamera->SetAspectRatio(static_cast<float>(GetRenderTargetWidth()) / static_cast<float>(GetRenderTargetHeight()));
}

glm::dvec3 FEngine::ConstructMouseRay()
{
	glm::dvec2 NormalizedMouseCoords;
	NormalizedMouseCoords.x = (2.0f * MouseX) / GetRenderTargetWidth() - 1;
	NormalizedMouseCoords.y = 1.0f - (2.0f * (MouseY)) / GetRenderTargetHeight();

	const glm::dvec4 ClipCoords = glm::dvec4(NormalizedMouseCoords.x, NormalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 EyeCoords = glm::inverse(GetCamera()->GetProjectionMatrix()) * ClipCoords;
	EyeCoords.z = -1.0f;
	EyeCoords.w = 0.0f;
	glm::dvec3 WorldRay = glm::inverse(GetCamera()->GetViewMatrix()) * EyeCoords;
	WorldRay = glm::normalize(WorldRay);

	return WorldRay;
}

FE_RENDER_TARGET_MODE FEngine::GetRenderTargetMode()
{
	return RenderTargetMode;
}

void FEngine::SetRenderTargetMode(const FE_RENDER_TARGET_MODE NewMode)
{
	if (RenderTargetMode != NewMode && NewMode == FE_GLFW_MODE)
	{
		RenderTargetMode = NewMode;
		int WindowWidth, WindowHeight;
		APPLICATION.GetMainWindow()->GetSize(&WindowWidth, &WindowHeight);
		WindowResizeCallback(WindowWidth, WindowHeight);
	}
	else
	{
		RenderTargetMode = NewMode;
	}
}

void FEngine::SetRenderTargetSize(const int Width, const int Height)
{
	if (Width <= 0 || Height <= 0 || RenderTargetMode == FE_GLFW_MODE)
		return;

	bool NeedReInitialization = false;
	if (RenderTargetW != Width || RenderTargetH != Height)
		NeedReInitialization = true;

	RenderTargetW = Width;
	RenderTargetH = Height;

	if (NeedReInitialization)
		RenderTargetResize();
}

int FEngine::GetRenderTargetWidth()
{
	return RenderTargetW;
}

int FEngine::GetRenderTargetHeight()
{
	return RenderTargetH;
}

void FEngine::RenderTargetResize()
{
	ENGINE.CurrentCamera->SetAspectRatio(static_cast<float>(ENGINE.RenderTargetW) / static_cast<float>(ENGINE.RenderTargetH));

	RENDERER.RenderTargetResize(ENGINE.RenderTargetW, ENGINE.RenderTargetH);

	if (!ENGINE.bSimplifiedRendering)
	{
		// ************************************ Bloom ************************************
		FEPostProcess* BloomEffect = ENGINE.CreatePostProcess("bloom", static_cast<int>(ENGINE.RenderTargetW / 4.0f), static_cast<int>(ENGINE.RenderTargetH / 4.0f));
		BloomEffect->SetID("451C48791871283D372C5938"/*"bloom"*/);

		BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_SCENE_HDR_COLOR, RESOURCE_MANAGER.GetShader("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/)));

		BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

		BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

		BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

		BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
		BloomEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

		BloomEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_HDR_COLOR}, RESOURCE_MANAGER.GetShader("1833272551376C2E5645200E"/*"FEBloomComposition"*/)));

		RENDERER.AddPostProcess(BloomEffect);
		// ************************************ Bloom END ************************************

		// ************************************ gammaHDR ************************************
		FEPostProcess* GammaHDR = ENGINE.CreatePostProcess("GammaAndHDR", ENGINE.RenderTargetW, ENGINE.RenderTargetH);
		GammaHDR->SetID("2374462A7B0E78141B5F5D79"/*"GammaAndHDR"*/);
		GammaHDR->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/)));
		RENDERER.AddPostProcess(GammaHDR);
		// ************************************ gammaHDR END ************************************

		// ************************************ FXAA ************************************
		FEPostProcess* FEFXAAEffect = ENGINE.CreatePostProcess("FE_FXAA", ENGINE.RenderTargetW, ENGINE.RenderTargetH);
		FEFXAAEffect->SetID("0A3F10643F06525D70016070"/*"FE_FXAA"*/);
		FEFXAAEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/)));
		FEFXAAEffect->Stages.back()->Shader->UpdateParameterData("FXAATextuxelSize", glm::vec2(1.0f / ENGINE.RenderTargetW, 1.0f / ENGINE.RenderTargetH));
		RENDERER.AddPostProcess(FEFXAAEffect);

		//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
		RENDERER.PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.RenderTargetW, ENGINE.RenderTargetH));
		// ************************************ FXAA END ************************************

		// ************************************ DOF ************************************
		FEPostProcess* DOFEffect = ENGINE.CreatePostProcess("DOF");
		DOFEffect->SetID("217C4E80482B6C650D7B492F"/*"DOF"*/);

		DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
		DOFEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
		DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
		DOFEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
		RENDERER.AddPostProcess(DOFEffect);
		// ************************************ DOF END ************************************

		// ************************************ chromaticAberrationEffect ************************************
		FEPostProcess* ChromaticAberrationEffect = ENGINE.CreatePostProcess("chromaticAberration");
		ChromaticAberrationEffect->SetID("506D804162647749060C3E68"/*"chromaticAberration"*/);
		ChromaticAberrationEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 }, RESOURCE_MANAGER.GetShader("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/)));
		RENDERER.AddPostProcess(ChromaticAberrationEffect);
		//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
		RENDERER.PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.RenderTargetW, ENGINE.RenderTargetH));
		// ************************************ chromaticAberrationEffect END ************************************
	}

	for (size_t i = 0; i < ENGINE.ClientRenderTargetResizeCallbacks.size(); i++)
	{
		if (ENGINE.ClientRenderTargetResizeCallbacks[i] == nullptr)
			continue;
		ENGINE.ClientRenderTargetResizeCallbacks[i](ENGINE.RenderTargetW, ENGINE.RenderTargetH);
	}
}

void FEngine::AddRenderTargetResizeCallback(void(*Func)(int, int))
{
	if (Func != nullptr)
		ClientRenderTargetResizeCallbacks.push_back(Func);
}

inline int FEngine::GetRenderTargetXShift()
{
	return RenderTargetXShift;
}

void FEngine::SetRenderTargetXShift(const int NewRenderTargetXShift)
{
	RenderTargetXShift = NewRenderTargetXShift;
}

inline int FEngine::GetRenderTargetYShift()
{
	return RenderTargetYShift;
}

void FEngine::SetRenderTargetYShift(const int NewRenderTargetYShift)
{
	RenderTargetYShift = NewRenderTargetYShift;
}

void FEngine::RenderTargetCenterForCamera(FEFreeCamera* Camera)
{
	int CenterX, CenterY = 0;
	int ShiftX, ShiftY = 0;

	int xpos, ypos;
	APPLICATION.GetMainWindow()->GetPosition(&xpos, &ypos);

	if (RenderTargetMode == FE_GLFW_MODE)
	{
		CenterX = xpos + (WindowW / 2);
		CenterY = ypos + (WindowH / 2);

		ShiftX = xpos;
		ShiftY = ypos;
	}
	else if (RenderTargetMode == FE_CUSTOM_MODE)
	{
		CenterX = xpos + RenderTargetXShift + (RenderTargetW / 2);
		CenterY = ypos + RenderTargetYShift + (RenderTargetH / 2);

		ShiftX = RenderTargetXShift + xpos;
		ShiftY = RenderTargetYShift + ypos;
	}
	
	Camera->SetRenderTargetCenterX(CenterX);
	Camera->SetRenderTargetCenterY(CenterY);

	Camera->SetRenderTargetShiftX(ShiftX);
	Camera->SetRenderTargetShiftY(ShiftY);
}

void FEngine::DropCallback(const int Count, const char** Paths)
{
	for (size_t i = 0; i < ENGINE.ClientDropCallbacks.size(); i++)
	{
		if (ENGINE.ClientDropCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientDropCallbacks[i](Count, Paths);
	}
}

void FEngine::AddDropCallback(void(*Func)(int, const char**))
{
	if (Func != nullptr)
		ClientDropCallbacks.push_back(Func);
}

void FEngine::MouseScrollCallback(const double Xoffset, const double Yoffset)
{
	for (size_t i = 0; i < ENGINE.ClientMouseScrollCallbacks.size(); i++)
	{
		if (ENGINE.ClientMouseScrollCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientMouseScrollCallbacks[i](Xoffset, Yoffset);
	}

	ENGINE.CurrentCamera->MouseScrollInput(Xoffset, Yoffset);
}

glm::vec4 FEngine::GetClearColor()
{
	return CurrentClearColor;
}

void FEngine::SetClearColor(glm::vec4 ClearColor)
{
	CurrentClearColor = ClearColor;
	glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
}

bool FEngine::IsSimplifiedRenderingModeActive()
{
	return bSimplifiedRendering;
}

void FEngine::ActivateSimplifiedRenderingMode()
{
	bSimplifiedRendering = true;
	RENDERER.bSimplifiedRendering = true;
}

bool FEngine::IsVsyncEnabled()
{
	return bVsyncEnabled;
}

void FEngine::SetVsyncEnabled(bool NewValue)
{
	bVsyncEnabled = NewValue;
	if (bVsyncEnabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}
}

void FEngine::DisableVR()
{
	bVRActive = false;
	RENDERER.bVRActive = false;
}

bool FEngine::EnableVR()
{
	if (!bVRInitializedCorrectly)
	{
		OpenXR_MANAGER.Init(WindowTitle);

		auto test = LOG.GetLogItems("OpenXR");
		// Fix this!
		bVRInitializedCorrectly = true/*LOG.GetLogItems("OpenXR").empty()*/;
	}

	if (bVRInitializedCorrectly)
	{
		bVRActive = true;
		RENDERER.bVRActive = true;
		RENDERER.UpdateVRRenderTargetSize(static_cast<int>(OpenXR_MANAGER.EyeResolution().x), static_cast<int>(OpenXR_MANAGER.EyeResolution().y));
	}

	return bVRActive;
}

bool FEngine::IsVRInitializedCorrectly()
{
	return bVRInitializedCorrectly;
}

bool FEngine::IsVREnabled()
{
	return bVRActive;
}