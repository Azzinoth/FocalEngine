#include "FEngine.h"
using namespace FocalEngine;

FEngine* FEngine::_instance = nullptr;
FERenderTargetMode FEngine::renderTargetMode = FE_GLFW_MODE;
int FEngine::renderTargetXShift = 0;
int FEngine::renderTargetYShift = 0;

#define RENDERER FERenderer::getInstance()
#define RESOURCE_MANAGER FEResourceManager::getInstance()
#define ENGINE FEngine::getInstance()
#define SCENE FEScene::getInstance()
#define TIME FETime::getInstance()

FEngine::FEngine()
{
	FEInput::getInstance().mouseButtonCallbackImpl = &FEngine::mouseButtonCallback;
	FEInput::getInstance().mouseMoveCallbackImpl = &FEngine::mouseMoveCallback;
	FEInput::getInstance().keyButtonCallbackImpl = &FEngine::keyButtonCallback;
	FEInput::getInstance().dropCallbackImpl = &FEngine::dropCallback;
}

FEngine::~FEngine()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool FEngine::isWindowOpened()
{
	return !glfwWindowShouldClose(window);
}

void FEngine::beginFrame(bool internalCall)
{
	if (!internalCall)
	{
		TIME.beginTimeStamp();
		RESOURCE_MANAGER.updateAsyncLoadedResources();
	}

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

#ifdef FE_DEBUG_ENABLED
	std::vector<std::string> shaderList = RESOURCE_MANAGER.getShadersList();
	std::vector<std::string> tempList = RESOURCE_MANAGER.getStandardShadersList();
	for (size_t i = 0; i < tempList.size(); i++)
	{
		shaderList.push_back(tempList[i]);
	}

	for (size_t i = 0; i < shaderList.size(); i++)
	{
		if (RESOURCE_MANAGER.getShader(shaderList[i])->isDebugRequest())
		{
			RESOURCE_MANAGER.getShader(shaderList[i])->thisFrameDebugBind = 0;
		}
	}
#endif
}

void FEngine::render(bool internalCall)
{
	RENDERER.engineMainCamera = ENGINE.currentCamera;
	RENDERER.mouseRay = ENGINE.constructMouseRay();

	ENGINE.currentCamera->move(cpuTime + gpuTime);
	RENDERER.render(currentCamera);

	if (!internalCall) cpuTime = TIME.endTimeStamp();
}

void FEngine::endFrame(bool internalCall)
{
	if (!internalCall) TIME.beginTimeStamp();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	glfwPollEvents();
	if (!internalCall) gpuTime = TIME.endTimeStamp();
}

void FEngine::setImguiStyle()
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
	colors[ImGuiCol_Text] = ImVec4(255.0f/255.0f, 243.0f / 255.0f, 255.0f / 255.0f, 1.00f);
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

void FEngine::createWindow(int width, int height, std::string WindowTitle)
{
	windowW = width;
	windowH = height;
	windowTitle = WindowTitle;

	glfwInit();

	window = glfwCreateWindow(windowW, windowH, windowTitle.c_str(), NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		//return -1;
	}

	//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	//test_offscreen_context = glfwCreateWindow(640, 480, "", NULL, window);

	glfwMakeContextCurrent(window);
	glewInit();

	glfwSetWindowCloseCallback(window, windowCloseCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetMouseButtonCallback(window, &FEInput::mouseButtonCallback);
	glfwSetCursorPosCallback(window, &FEInput::mouseMoveCallback);
	glfwSetKeyCallback(window, &FEInput::keyButtonCallback);
	glfwSetDropCallback(window, &FEInput::dropCallback);

	glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w);

	// turn off v-sync
	glfwSwapInterval(0);

	currentCamera = new FEFreeCamera(window, "mainCamera");
	int finalWidth, finalHeight;
	glfwGetWindowSize(window, &finalWidth, &finalHeight);
	windowW = finalWidth;
	windowH = finalHeight;
	renderTargetW = finalWidth;
	renderTargetH = finalHeight;
	currentCamera->setAspectRatio(float(getRenderTargetWidth()) / float(getRenderTargetHeight()));

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));
	/*FE_GL_ERROR(glEnable(GL_CULL_FACE));
	FE_GL_ERROR(glCullFace(GL_BACK));*/

	// tessellation parameter
	FE_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));

	RENDERER.instancedLineShader = RESOURCE_MANAGER.createShader("instancedLine", RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str(),
																				  RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(RENDERER.instancedLineShader->getObjectID());
	RENDERER.instancedLineShader->setID("7E0826291010377D564F6115"/*"instancedLine"*/);
	RESOURCE_MANAGER.shaders[RENDERER.instancedLineShader->getObjectID()] = RENDERER.instancedLineShader;

	FEShader* FEScreenQuadShader = RESOURCE_MANAGER.createShader("FEScreenQuadShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str(),
																					   RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ScreenQuad_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(FEScreenQuadShader->getObjectID());
	FEScreenQuadShader->setID("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	RESOURCE_MANAGER.shaders[FEScreenQuadShader->getObjectID()] = FEScreenQuadShader;
	RESOURCE_MANAGER.makeShaderStandard(FEScreenQuadShader);

	RENDERER.standardFBInit(getRenderTargetWidth(), getRenderTargetHeight());
	
	// ************************************ Bloom ************************************
	FEPostProcess* bloomEffect = ENGINE.createPostProcess("bloom", int(getRenderTargetWidth() / 4.0f), int(getRenderTargetHeight() / 4.0f));
	bloomEffect->setID("451C48791871283D372C5938"/*"bloom"*/);

	FocalEngine::FEShader* BloomThresholdShader =
		RESOURCE_MANAGER.createShader("FEBloomThreshold", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
														  RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomThreshold_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(BloomThresholdShader->getObjectID());
	BloomThresholdShader->setID("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/);
	RESOURCE_MANAGER.shaders[BloomThresholdShader->getObjectID()] = BloomThresholdShader;

	bloomEffect->addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, BloomThresholdShader));
	bloomEffect->stages[0]->shader->getParameter("thresholdBrightness")->updateData(1.0f);

	FocalEngine::FEShader* BloomBlurShader =
		RESOURCE_MANAGER.createShader("FEBloomBlur", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
												     RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomBlur_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(BloomBlurShader->getObjectID());
	BloomBlurShader->setID("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	RESOURCE_MANAGER.shaders[BloomBlurShader->getObjectID()] = BloomBlurShader;

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));
	
	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	FocalEngine::FEShader* BloomCompositionShader =
		RESOURCE_MANAGER.createShader("FEBloomComposition", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
														    RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomComposition_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(BloomCompositionShader->getObjectID());
	BloomCompositionShader->setID("1833272551376C2E5645200E"/*"FEBloomComposition"*/);
	RESOURCE_MANAGER.shaders[BloomCompositionShader->getObjectID()] = BloomCompositionShader;

	bloomEffect->addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, BloomCompositionShader));

	RENDERER.addPostProcess(bloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ gammaHDR ************************************
	FEPostProcess* gammaHDR = ENGINE.createPostProcess("GammaAndHDR", getRenderTargetWidth(), getRenderTargetHeight());
	gammaHDR->setID("2374462A7B0E78141B5F5D79"/*"GammaAndHDR"*/);

	FocalEngine::FEShader* gammaHDRShader =
		RESOURCE_MANAGER.createShader("FEGammaAndHDRShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_VS.glsl").c_str(),
															 RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(gammaHDRShader->getObjectID());
	gammaHDRShader->setID("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/);
	RESOURCE_MANAGER.shaders[gammaHDRShader->getObjectID()] = gammaHDRShader;

	gammaHDR->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, gammaHDRShader));
	RENDERER.addPostProcess(gammaHDR);
	// ************************************ gammaHDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.createPostProcess("FE_FXAA", getRenderTargetWidth(), getRenderTargetHeight());
	FEFXAAEffect->setID("0A3F10643F06525D70016070"/*"FE_FXAA"*/);

	FocalEngine::FEShader* FEFXAAShader =
		RESOURCE_MANAGER.createShader("FEFXAAShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_VS.glsl").c_str(),
													  RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(FEFXAAShader->getObjectID());
	FEFXAAShader->setID("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/);
	RESOURCE_MANAGER.shaders[FEFXAAShader->getObjectID()] = FEFXAAShader;

	FEFXAAEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, FEFXAAShader));
	FEFXAAEffect->stages.back()->shader->getParameter("FXAASpanMax")->updateData(8.0f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAAReduceMin")->updateData(1.0f / 128.0f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAAReduceMul")->updateData(0.4f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAATextuxelSize")->updateData(glm::vec2(1.0f / getRenderTargetWidth(), 1.0f / getRenderTargetHeight()));
	RENDERER.addPostProcess(FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.postProcessEffects.back()->replaceOutTexture(0, RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, getRenderTargetWidth(), getRenderTargetHeight()));

	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FocalEngine::FEPostProcess* DOFEffect = ENGINE.createPostProcess("DOF");
	DOFEffect->setID("217C4E80482B6C650D7B492F"/*"DOF"*/);

	FocalEngine::FEShader* DOFShader = RESOURCE_MANAGER.createShader("DOF", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_VS.glsl").c_str(),
																			RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(DOFShader->getObjectID());
	DOFShader->setID("7800253C244442155D0F3C7B"/*"DOF"*/);
	RESOURCE_MANAGER.shaders[DOFShader->getObjectID()] = DOFShader;

	DOFEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, DOFShader));
	DOFEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	DOFEffect->stages.back()->shader->getParameter("blurSize")->updateData(2.0f);
	DOFEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(0.0f);
	DOFEffect->stages.back()->shader->getParameter("depthThresholdFar")->updateData(9000.0f);
	DOFEffect->stages.back()->shader->getParameter("zNear")->updateData(0.1f);
	DOFEffect->stages.back()->shader->getParameter("zFar")->updateData(5000.0f);
	DOFEffect->stages.back()->shader->getParameter("intMult")->updateData(100.0f);
	DOFEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, DOFShader));
	DOFEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	DOFEffect->stages.back()->shader->getParameter("blurSize")->updateData(2.0f);
	DOFEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(0.0f);
	DOFEffect->stages.back()->shader->getParameter("depthThresholdFar")->updateData(9000.0f);
	DOFEffect->stages.back()->shader->getParameter("zNear")->updateData(0.1f);
	DOFEffect->stages.back()->shader->getParameter("zFar")->updateData(5000.0f);
	DOFEffect->stages.back()->shader->getParameter("intMult")->updateData(100.0f);
	RENDERER.addPostProcess(DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ chromaticAberrationEffect ************************************
	FEPostProcess* chromaticAberrationEffect = ENGINE.createPostProcess("chromaticAberration");
	chromaticAberrationEffect->setID("506D804162647749060C3E68"/*"chromaticAberration"*/);

	FEShader* chromaticAberrationShader = RESOURCE_MANAGER.createShader("chromaticAberrationShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_VS.glsl").c_str(),
																									 RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_FS.glsl").c_str());
	RESOURCE_MANAGER.shaders.erase(chromaticAberrationShader->getObjectID());
	chromaticAberrationShader->setID("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/);
	RESOURCE_MANAGER.shaders[chromaticAberrationShader->getObjectID()] = chromaticAberrationShader;
	
	chromaticAberrationEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0 }, chromaticAberrationShader));
	chromaticAberrationEffect->stages.back()->shader->getParameter("intensity")->updateData(1.0f);
	RENDERER.addPostProcess(chromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.postProcessEffects.back()->replaceOutTexture(0, RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, getRenderTargetWidth(), getRenderTargetHeight()));
	// ************************************ chromaticAberrationEffect END ************************************

	// ************************************ SSAO ************************************
#ifdef USE_DEFERRED_RENDERER
	FEShader* FESSAOShader = RESOURCE_MANAGER.createShader("FESSAOShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_VS.glsl").c_str(),
																		   RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_FS.glsl").c_str());

	RESOURCE_MANAGER.shaders.erase(FESSAOShader->getObjectID());
	FESSAOShader->setID("1037115B676E383E36345079"/*"FESSAOShader"*/);
	RESOURCE_MANAGER.shaders[FESSAOShader->getObjectID()] = FESSAOShader;

	RESOURCE_MANAGER.makeShaderStandard(FESSAOShader);
#endif // USE_DEFERRED_RENDERER
	// ************************************ SSAO END ************************************

	RENDERER.shadowMapMaterial = RESOURCE_MANAGER.createMaterial("shadowMapMaterial");
	RENDERER.shadowMapMaterial->shader = RESOURCE_MANAGER.createShader("FEShadowMapShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_VS.glsl").c_str(),
																							RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str());
	RENDERER.shadowMapMaterial->shader->setID("7C41565B2E2B05321A182D89"/*"FEShadowMapShader"*/);
	
	RESOURCE_MANAGER.makeShaderStandard(RENDERER.shadowMapMaterial->shader);
	RESOURCE_MANAGER.makeMaterialStandard(RENDERER.shadowMapMaterial);

	RENDERER.shadowMapMaterialInstanced = RESOURCE_MANAGER.createMaterial("shadowMapMaterialInstanced");
	RENDERER.shadowMapMaterialInstanced->shader = RESOURCE_MANAGER.createShader("FEShadowMapShaderInstanced", RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_INSTANCED_VS.glsl").c_str(),
																											  RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str());
	RENDERER.shadowMapMaterialInstanced->shader->setID("5634765B2E2A05321A182D1A"/*"FEShadowMapShaderInstanced"*/);
	
	RESOURCE_MANAGER.makeShaderStandard(RENDERER.shadowMapMaterialInstanced->shader);
	RESOURCE_MANAGER.makeMaterialStandard(RENDERER.shadowMapMaterialInstanced);


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	size_t pathLen = strlen((RESOURCE_MANAGER.defaultResourcesFolder + "imgui.ini").c_str()) + 1;
	char* imguiIniFile = new char[pathLen];
	strcpy_s(imguiIniFile, pathLen, (RESOURCE_MANAGER.defaultResourcesFolder + "imgui.ini").c_str());
	io.IniFilename = imguiIniFile;
	io.Fonts->AddFontFromFileTTF((RESOURCE_MANAGER.defaultResourcesFolder + "Cousine-Regular.ttf").c_str(), 20);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	unsigned char* tex_pixels = NULL;
	int tex_w, tex_h;
	io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

	io.DisplaySize = ImVec2(float(windowW), float(windowH));
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");

	setImguiStyle();
}

void FEngine::setWindowCaption(const char* text)
{
	glfwSetWindowTitle(window, text);
}

void FEngine::setWindowResizeCallback(void(*func)(int, int))
{
	clientWindowResizeCallbackImpl = func;
}

void FEngine::setWindowCloseCallback(void(*func)())
{
	clientWindowCloseCallbackImpl = func;
}

void FEngine::setKeyCallback(void(*func)(int, int, int, int))
{
	clientKeyButtonCallbackImpl = func;
}

void FEngine::setMouseButtonCallback(void(*func)(int, int, int))
{
	clientMouseButtonCallbackImpl = func;
}

void FEngine::setMouseMoveCallback(void(*func)(double, double))
{
	clientMouseMoveCallbackImpl = func;
}

void FEngine::windowCloseCallback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, false);
	FEngine& engineObj = getInstance();
	if (engineObj.clientWindowCloseCallbackImpl != nullptr)
	{
		engineObj.clientWindowCloseCallbackImpl();
	}
	else
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void FEngine::windowResizeCallback(GLFWwindow* window, int width, int height)
{
	int finalWidth, finalHeight;
	glfwGetWindowSize(window, &finalWidth, &finalHeight);

	if (finalWidth == 0 || finalHeight == 0)
		return;

	FEngine& engineObj = getInstance();
	engineObj.windowW = finalWidth;
	engineObj.windowH = finalHeight;

	ImGui::GetIO().DisplaySize = ImVec2(float(engineObj.renderTargetW), float(engineObj.renderTargetH));

	if (renderTargetMode == FE_GLFW_MODE)
	{
		engineObj.renderTargetW = finalWidth;
		engineObj.renderTargetH = finalHeight;

		renderTargetResize();
	}

	if (engineObj.clientWindowResizeCallbackImpl != nullptr)
	{
		engineObj.clientWindowResizeCallbackImpl(engineObj.windowW, engineObj.windowH);
	}
}

void FEngine::mouseButtonCallback(int button, int action, int mods)
{
	FEngine& engineObj = getInstance();
	if (engineObj.clientMouseButtonCallbackImpl != nullptr)
		engineObj.clientMouseButtonCallbackImpl(button, action, mods);
}

void FEngine::mouseMoveCallback(double xpos, double ypos)
{
	if (renderTargetMode == FE_CUSTOM_MODE)
	{
		xpos -= renderTargetXShift;
		ypos -= renderTargetYShift;
	}

	FEngine& engineObj = getInstance();
	if (engineObj.clientMouseMoveCallbackImpl != nullptr)
		engineObj.clientMouseMoveCallbackImpl(xpos, ypos);

	engineObj.currentCamera->mouseMoveInput(xpos, ypos);

	engineObj.mouseX = xpos;
	engineObj.mouseY = ypos;
}

void FEngine::keyButtonCallback(int key, int scancode, int action, int mods)
{
	FEngine& engineObj = getInstance();
	if (engineObj.clientKeyButtonCallbackImpl != nullptr)
		engineObj.clientKeyButtonCallbackImpl(key, scancode, action, mods);

	engineObj.currentCamera->keyboardInput(key, scancode, action, mods);
}

void FEngine::setCamera(FEBasicCamera* newCamera)
{
	currentCamera = newCamera;
}

FEBasicCamera* FEngine::getCamera()
{
	return currentCamera;
}

int FEngine::getWindowWidth()
{
	return windowW;
}

int FEngine::getWindowHeight()
{
	return windowH;
}

void FEngine::renderTo(FEFramebuffer* renderTo)
{
	renderTo->bind();
	beginFrame(true);
	render(true);
	renderTo->unBind();
}

float FEngine::getCpuTime()
{
	return cpuTime;
}

float FEngine::getGpuTime()
{
	return gpuTime;
}

FEPostProcess* FEngine::createPostProcess(std::string Name, int ScreenWidth, int ScreenHeight)
{
	if (ScreenWidth < 2 || ScreenHeight < 2)
	{
		ScreenWidth = getRenderTargetWidth();
		ScreenHeight = getRenderTargetHeight();
	}

	return RESOURCE_MANAGER.createPostProcess(ScreenWidth, ScreenHeight, Name);
}

void FEngine::terminate()
{
	glfwSetWindowShouldClose(window, true);
}

void FEngine::takeScreenshot(const char* fileName)
{
	RENDERER.takeScreenshot(fileName, getRenderTargetWidth(), getRenderTargetHeight());
}

void FEngine::resetCamera()
{
	currentCamera->reset();
	currentCamera->setAspectRatio(float(getRenderTargetWidth()) / float(getRenderTargetHeight()));
}

glm::dvec3 FEngine::constructMouseRay()
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / getRenderTargetWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / getRenderTargetHeight();

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(getCamera()->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(getCamera()->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

FERenderTargetMode FEngine::getRenderTargetMode()
{
	return renderTargetMode;
}

void FEngine::setRenderTargetMode(FERenderTargetMode newMode)
{
	if (renderTargetMode != newMode && newMode == FE_GLFW_MODE)
	{
		renderTargetMode = newMode;
		windowResizeCallback(window, 0, 0);
	}
	else
	{
		renderTargetMode = newMode;
	}
}

void FEngine::setRenderTargetWidth(int newRenderTargetWidth)
{
	if (newRenderTargetWidth <= 0 || renderTargetMode == FE_GLFW_MODE)
		return;

	bool needReInitialization = false;
	if (renderTargetW != newRenderTargetWidth)
		needReInitialization = true;

	renderTargetW = newRenderTargetWidth;

	if (needReInitialization)
		renderTargetResize();
}

void FEngine::setRenderTargetHeight(int newRenderTargetHeight)
{
	if (newRenderTargetHeight <= 0 || renderTargetMode == FE_GLFW_MODE)
		return;

	bool needReInitialization = false;
	if (renderTargetH != newRenderTargetHeight)
		needReInitialization = true;

	renderTargetH = newRenderTargetHeight;

	if (needReInitialization)
		renderTargetResize();
}

int FEngine::getRenderTargetWidth()
{
	return renderTargetW;
}

int FEngine::getRenderTargetHeight()
{
	return renderTargetH;
}

void FEngine::renderTargetResize()
{
	FEngine& engineObj = getInstance();
	engineObj.currentCamera->setAspectRatio(float(engineObj.renderTargetW) / float(engineObj.renderTargetH));

	FERenderer& renderer = RENDERER;

	delete RENDERER.sceneToTextureFB;
	RENDERER.sceneToTextureFB = FEResourceManager::getInstance().createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, engineObj.renderTargetW, engineObj.renderTargetH);

#ifdef USE_DEFERRED_RENDERER
	RENDERER.GBuffer->renderTargetResize(RENDERER.sceneToTextureFB);
#endif // USE_DEFERRED_RENDERER

	for (size_t i = 0; i < RENDERER.postProcessEffects.size(); i++)
	{
		delete RENDERER.postProcessEffects[i];
	}
	RENDERER.postProcessEffects.clear();

	// ************************************ Bloom ************************************
	FEPostProcess* bloomEffect = ENGINE.createPostProcess("bloom", int(engineObj.renderTargetW / 4.0f), int(engineObj.renderTargetH / 4.0f));
	bloomEffect->setID("451C48791871283D372C5938"/*"bloom"*/);

	bloomEffect->addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, RESOURCE_MANAGER.getShader("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/)));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(5.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	bloomEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));

	bloomEffect->addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, RESOURCE_MANAGER.getShader("1833272551376C2E5645200E"/*"FEBloomComposition"*/)));

	RENDERER.addPostProcess(bloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ gammaHDR ************************************
	FEPostProcess* gammaHDR = ENGINE.createPostProcess("GammaAndHDR", engineObj.renderTargetW, engineObj.renderTargetH);
	gammaHDR->setID("2374462A7B0E78141B5F5D79"/*"GammaAndHDR"*/);
	gammaHDR->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/)));
	RENDERER.addPostProcess(gammaHDR);
	// ************************************ gammaHDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.createPostProcess("FE_FXAA", engineObj.renderTargetW, engineObj.renderTargetH);
	FEFXAAEffect->setID("0A3F10643F06525D70016070"/*"FE_FXAA"*/);
	FEFXAAEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.getShader("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/)));
	FEFXAAEffect->stages.back()->shader->getParameter("FXAATextuxelSize")->updateData(glm::vec2(1.0f / engineObj.renderTargetW, 1.0f / engineObj.renderTargetH));
	RENDERER.addPostProcess(FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.postProcessEffects.back()->replaceOutTexture(0, RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, engineObj.renderTargetW, engineObj.renderTargetH));
	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FocalEngine::FEPostProcess* DOFEffect = ENGINE.createPostProcess("DOF");
	DOFEffect->setID("217C4E80482B6C650D7B492F"/*"DOF"*/);

	DOFEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, RESOURCE_MANAGER.getShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
	DOFEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	DOFEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, RESOURCE_MANAGER.getShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
	DOFEffect->stages.back()->stageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	RENDERER.addPostProcess(DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ chromaticAberrationEffect ************************************
	FEPostProcess* chromaticAberrationEffect = ENGINE.createPostProcess("chromaticAberration");
	chromaticAberrationEffect->setID("506D804162647749060C3E68"/*"chromaticAberration"*/);
	chromaticAberrationEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0 }, RESOURCE_MANAGER.getShader("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/)));
	RENDERER.addPostProcess(chromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	RENDERER.postProcessEffects.back()->replaceOutTexture(0, RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, engineObj.renderTargetW, engineObj.renderTargetH));
	// ************************************ chromaticAberrationEffect END ************************************

	// ************************************ SSAO ************************************
#ifdef USE_DEFERRED_RENDERER
	RENDERER.SSAOFB = FEResourceManager::getInstance().createFramebuffer(FE_COLOR_ATTACHMENT, engineObj.renderTargetW, engineObj.renderTargetH, false);
#endif // USE_DEFERRED_RENDERER
	// ************************************ SSAO END ************************************

	if (engineObj.clientRenderTargetResizeCallbackImpl != nullptr)
	{
		engineObj.clientRenderTargetResizeCallbackImpl(engineObj.renderTargetW, engineObj.renderTargetH);
	}
}

void FEngine::setRenderTargetResizeCallback(void(*func)(int, int))
{
	clientRenderTargetResizeCallbackImpl = func;
}

inline int FEngine::getRenderTargetXShift()
{
	return renderTargetXShift;
}

void FEngine::setRenderTargetXShift(int newRenderTargetXShift)
{
	renderTargetXShift = newRenderTargetXShift;
}

inline int FEngine::getRenderTargetYShift()
{
	return renderTargetYShift;
}

void FEngine::setRenderTargetYShift(int newRenderTargetYShift)
{
	renderTargetYShift = newRenderTargetYShift;
}

void FEngine::renderTargetCenterForCamera(FEFreeCamera* camera)
{
	int centerX, centerY = 0;
	int shiftX, shiftY = 0;

	int xpos, ypos;
	glfwGetWindowPos(window, &xpos, &ypos);

	if (renderTargetMode == FE_GLFW_MODE)
	{
		centerX = xpos + (windowW / 2);
		centerY = ypos + (windowH / 2);

		shiftX = xpos;
		shiftY = ypos;
	}
	else if (renderTargetMode == FE_CUSTOM_MODE)
	{
		centerX = xpos + renderTargetXShift + (renderTargetW / 2);
		centerY = ypos + renderTargetYShift + (renderTargetH / 2);

		shiftX = renderTargetXShift + xpos;
		shiftY = renderTargetYShift + ypos;
	}
	
	camera->setRenderTargetCenterX(centerX);
	camera->setRenderTargetCenterY(centerY);

	camera->setRenderTargetShiftX(shiftX);
	camera->setRenderTargetShiftY(shiftY);
}

void FEngine::dropCallback(int count, const char** paths)
{
	FEngine& engineObj = getInstance();
	if (engineObj.clientDropCallbackImpl != nullptr)
		engineObj.clientDropCallbackImpl(count, paths);
}

void FEngine::setDropCallback(void(*func)(int, const char**))
{
	clientDropCallbackImpl = func;
}