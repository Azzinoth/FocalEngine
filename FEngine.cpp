#include "FEngine.h"
using namespace FocalEngine;

FEngine* FEngine::_instance = nullptr;
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
	if (!internalCall) TIME.beginTimeStamp();

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
	style->Alpha = 0.8f;
	style->FrameRounding = 3.0f;

	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	//colors[ImGuiCol_WindowBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
	// old grey window titels
	//colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
	//colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
	//colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.47f, 0.83f, 1.00f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
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
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
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
	//if (!glfwInit())
		//return -1;

	window = glfwCreateWindow(windowW, windowH, windowTitle.c_str(), NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(window);
	glewInit();

	glfwSetWindowCloseCallback(window, windowCloseCallback);
	glfwSetMouseButtonCallback(window, &FEInput::mouseButtonCallback);
	glfwSetCursorPosCallback(window, &FEInput::mouseMoveCallback);
	glfwSetKeyCallback(window, &FEInput::keyButtonCallback);

	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);

	// turn off v-sync
	//glfwSwapInterval(0);

	currentCamera = new FEFreeCamera(window);
	currentCamera->setAspectRatio(float(width) / float(height));

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));
	//FE_GL_ERROR(glEnable(GL_CULL_FACE));
	//FE_GL_ERROR(glCullFace(GL_BACK));

	// tessellation parameter
	FE_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));

	RENDERER.instancedLineShader = RESOURCE_MANAGER.createShader("instancedLine", RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str(),
																				  RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str());

	FEShader* FEScreenQuadShader = RESOURCE_MANAGER.createShader("FEScreenQuadShader", FEScreenQuadVS, FEScreenQuadFS);
	RESOURCE_MANAGER.makeShaderStandard(FEScreenQuadShader);

	RENDERER.standardFBInit(windowW, windowH);
	
	// ************************************ Bloom ************************************
	FEPostProcess* bloomEffect = ENGINE.createPostProcess("bloom", int(windowW / 4.0f), int(windowH / 4.0f));

	FocalEngine::FEShader* BloomThresholdShader =
		RESOURCE_MANAGER.createShader("FEBloomThreshold", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
														  RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomThreshold_FS.glsl").c_str());

	bloomEffect->addStage(new FEPostProcessStage(FEPP_SCENE_HDR_COLOR, BloomThresholdShader));
	bloomEffect->stages[0]->shader->getParameter("thresholdBrightness")->updateData(1.0f);

	FocalEngine::FEShader* BloomBlurShader =
		RESOURCE_MANAGER.createShader("FEBloomBlur", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
												     RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomBlur_FS.glsl").c_str());
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
		RESOURCE_MANAGER.createShader("FEBloomThreshold", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str(),
														  RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomComposition_FS.glsl").c_str());
	bloomEffect->addStage(new FEPostProcessStage(std::vector<int> { FEPP_PREVIOUS_STAGE_RESULT0, FEPP_SCENE_HDR_COLOR}, BloomCompositionShader));

	RENDERER.addPostProcess(bloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ gammaHDR ************************************
	FEPostProcess* gammaHDR = ENGINE.createPostProcess("GammaAndHDR", windowW, windowH);
	FocalEngine::FEShader* gammaHDRShader =
		RESOURCE_MANAGER.createShader("FEGammaAndHDRShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_VS.glsl").c_str(),
															 RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_FS.glsl").c_str());

	gammaHDR->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, gammaHDRShader));
	RENDERER.addPostProcess(gammaHDR);
	// ************************************ gammaHDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.createPostProcess("FE_FXAA", windowW, windowH);
	FocalEngine::FEShader* FEFXAAShader =
		RESOURCE_MANAGER.createShader("FEFXAAShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_VS.glsl").c_str(),
													  RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_FS.glsl").c_str());

	FEFXAAEffect->addStage(new FEPostProcessStage(FEPP_PREVIOUS_STAGE_RESULT0, FEFXAAShader));
	FEFXAAEffect->stages.back()->shader->getParameter("FXAASpanMax")->updateData(8.0f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAAReduceMin")->updateData(1.0f / 128.0f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAAReduceMul")->updateData(0.4f);
	FEFXAAEffect->stages.back()->shader->getParameter("FXAATextuxelSize")->updateData(glm::vec2(1.0f / windowW, 1.0f / windowH));
	RENDERER.addPostProcess(FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	delete RENDERER.postProcessEffects.back()->stages[0]->outTexture;
	RENDERER.postProcessEffects.back()->stages[0]->outTexture = RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, windowW, windowH);
	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FocalEngine::FEPostProcess* DOFEffect = ENGINE.createPostProcess("DOF");
	FocalEngine::FEShader* DOFShader = RESOURCE_MANAGER.createShader("DOF", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_VS.glsl").c_str(),
																			RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_FS.glsl").c_str());

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

	FEPostProcess* chromaticAberrationEffect = ENGINE.createPostProcess("chromaticAberration");
	FEShader* chromaticAberrationShader = RESOURCE_MANAGER.createShader("chromaticAberrationShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_VS.glsl").c_str(),
																									 RESOURCE_MANAGER.loadGLSL("CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_FS.glsl").c_str());
	chromaticAberrationEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0 }, chromaticAberrationShader));
	chromaticAberrationEffect->stages.back()->shader->getParameter("intensity")->updateData(1.0f);
	RENDERER.addPostProcess(chromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	delete RENDERER.postProcessEffects.back()->stages[0]->outTexture;
	RENDERER.postProcessEffects.back()->stages[0]->outTexture = RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, windowW, windowH);
	
	RENDERER.shadowMapMaterial = RESOURCE_MANAGER.createMaterial("shadowMapMaterial");
	RENDERER.shadowMapMaterial->shader = RESOURCE_MANAGER.createShader("FEShadowMapShader", RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_VS.glsl").c_str(),
																							RESOURCE_MANAGER.loadGLSL("CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str());
	RESOURCE_MANAGER.makeShaderStandard(RENDERER.shadowMapMaterial->shader);
	RESOURCE_MANAGER.makeMaterialStandard(RENDERER.shadowMapMaterial);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
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

void FEngine::setWindowCloseCallback(void(*func)())
{
	windowCloseCallbackImpl = func;
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
	if (engineObj.windowCloseCallbackImpl != nullptr)
	{
		engineObj.windowCloseCallbackImpl();
	}
	else
	{
		glfwSetWindowShouldClose(window, true);
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
		ScreenWidth = windowW;
		ScreenHeight = windowH;
	}

	return RESOURCE_MANAGER.createPostProcess(ScreenWidth, ScreenHeight, Name);
}

void FEngine::terminate()
{
	glfwSetWindowShouldClose(window, true);
}

void FEngine::takeScreenshot(const char* fileName)
{
	RENDERER.takeScreenshot(fileName, windowW, windowH);
}

void FEngine::resetCamera()
{
	currentCamera->reset();
	currentCamera->setAspectRatio(float(windowW) / float(windowH));
}

glm::dvec3 FEngine::constructMouseRay()
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / getWindowWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / getWindowHeight();

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(getCamera()->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(getCamera()->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}
