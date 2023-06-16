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

bool FEngine::IsWindowOpened()
{
	return APPLICATION.IsWindowOpened();
}

void FEngine::BeginFrame(const bool InternalCall)
{
	if (!InternalCall)
		TIME.BeginTimeStamp();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	APPLICATION.BeginFrame();

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

	if (!InternalCall) CPUTime = TIME.EndTimeStamp();
}

void FEngine::EndFrame(const bool InternalCall)
{
	if (!InternalCall) TIME.BeginTimeStamp();
	APPLICATION.EndFrame();
	if (!InternalCall) GPUTime = TIME.EndTimeStamp();
}

void FEngine::SetImguiStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->WindowRounding = 2.0f;
	style->ScrollbarRounding = 3.0f;
	style->GrabRounding = 2.0f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->WindowRounding = 2;
	style->ChildRounding = 2;
	style->ScrollbarSize = 16;
	style->ScrollbarRounding = 3;
	style->GrabRounding = 2;
	style->ItemSpacing.x = 10;
	style->ItemSpacing.y = 4;
	style->IndentSpacing = 22;
	style->FramePadding.x = 6;
	style->FramePadding.y = 4;
	style->Alpha = 1.0f;
	style->FrameRounding = 3.0f;

	/*colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);*/
	colors[ImGuiCol_Text] = ImVec4(1.0f, 243.0f / 255.0f, 1.0f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(158.0f / 255.0f, 158.0f / 255.0f, 158.0f / 255.0f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(43.0f / 255.0f, 43.0f / 255.0f, 43.0f / 255.0f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 0.98f);

	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.47f, 0.83f, 1.00f);

	//colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.725f, 0.805f, 0.702f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.793f, 0.900f, 0.836f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
}

void FEngine::InitWindow(const int Width, const int Height, std::string WindowTitle)
{
	WindowW = Width;
	WindowH = Height;
	this->WindowTitle = WindowTitle;

	APPLICATION.InitWindow(Width, Height, WindowTitle);
	APPLICATION.SetWindowResizeCallback(&FEngine::WindowResizeCallback);
	APPLICATION.SetMouseButtonCallback(&FEngine::MouseButtonCallback);
	APPLICATION.SetMouseMoveCallback(&FEngine::MouseMoveCallback);
	APPLICATION.SetKeyCallback(&FEngine::KeyButtonCallback);
	APPLICATION.SetDropCallback(&FEngine::DropCallback);
	APPLICATION.SetScrollCallback(&FEngine::MouseScrollCallback);

	ImGuiIO& io = ImGui::GetIO();

	const size_t PathLen = strlen((RESOURCE_MANAGER.ResourcesFolder + "imgui.ini").c_str()) + 1;
	char* ImguiIniFile = new char[PathLen];
	strcpy_s(ImguiIniFile, PathLen, (RESOURCE_MANAGER.ResourcesFolder + "imgui.ini").c_str());
	io.IniFilename = ImguiIniFile;
	io.Fonts->AddFontFromFileTTF((RESOURCE_MANAGER.ResourcesFolder + "Cousine-Regular.ttf").c_str(), 20);
	io.Fonts->AddFontFromFileTTF((RESOURCE_MANAGER.ResourcesFolder + "Cousine-Regular.ttf").c_str(), 32);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	unsigned char* TexPixels = nullptr;
	int TexW, TexH;
	io.Fonts->GetTexDataAsRGBA32(&TexPixels, &TexW, &TexH);

	io.DisplaySize = ImVec2(static_cast<float>(WindowW), static_cast<float>(WindowH));
	ImGui::StyleColorsDark();

	SetImguiStyle();

	SetClearColor(DefaultGammaCorrectedClearColor);

	// turn off v-sync
	//glfwSwapInterval(0);

	CurrentCamera = new FEFreeCamera("mainCamera");
	int FinalWidth, FinalHeight;
	APPLICATION.GetWindowSize(&FinalWidth, &FinalHeight);
	
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
	BloomEffect->Stages[0]->Shader->GetParameter("thresholdBrightness")->UpdateData(1.0f);

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
	FEFXAAEffect->Stages.back()->Shader->GetParameter("FXAASpanMax")->UpdateData(8.0f);
	FEFXAAEffect->Stages.back()->Shader->GetParameter("FXAAReduceMin")->UpdateData(1.0f / 128.0f);
	FEFXAAEffect->Stages.back()->Shader->GetParameter("FXAAReduceMul")->UpdateData(0.4f);
	FEFXAAEffect->Stages.back()->Shader->GetParameter("FXAATextuxelSize")->UpdateData(glm::vec2(1.0f / GetRenderTargetWidth(), 1.0f / GetRenderTargetHeight()));
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
	DOFEffect->Stages.back()->Shader->GetParameter("blurSize")->UpdateData(2.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("depthThreshold")->UpdateData(0.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("depthThresholdFar")->UpdateData(9000.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("zNear")->UpdateData(0.1f);
	DOFEffect->Stages.back()->Shader->GetParameter("zFar")->UpdateData(5000.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("intMult")->UpdateData(100.0f);
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	DOFEffect->Stages.back()->Shader->GetParameter("blurSize")->UpdateData(2.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("depthThreshold")->UpdateData(0.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("depthThresholdFar")->UpdateData(9000.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("zNear")->UpdateData(0.1f);
	DOFEffect->Stages.back()->Shader->GetParameter("zFar")->UpdateData(5000.0f);
	DOFEffect->Stages.back()->Shader->GetParameter("intMult")->UpdateData(100.0f);
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
	ChromaticAberrationEffect->Stages.back()->Shader->GetParameter("intensity")->UpdateData(1.0f);
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
	APPLICATION.SetWindowCaption(NewCaption);
}

void FEngine::AddWindowResizeCallback(void(*Func)(int, int))
{
	if (Func != nullptr)
		ClientWindowResizeCallbacks.push_back(Func);
}

void FEngine::AddWindowCloseCallback(void(*Func)())
{
	APPLICATION.SetWindowCloseCallback(Func);
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
	APPLICATION.Terminate();
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
		APPLICATION.GetWindowSize(&WindowWidth, &WindowHeight);
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
		FEFXAAEffect->Stages.back()->Shader->GetParameter("FXAATextuxelSize")->UpdateData(glm::vec2(1.0f / ENGINE.RenderTargetW, 1.0f / ENGINE.RenderTargetH));
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
	APPLICATION.GetWindowPosition(&xpos, &ypos);

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