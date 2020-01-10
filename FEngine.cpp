#include "FEngine.h"
using namespace FocalEngine;

FEngine* FEngine::_instance = nullptr;
#define RENDERER_OBJ FERenderer::getInstance()
#define RESOURCE_MANAGER_OBJ FEResourceManager::getInstance()
#define ENGINE_OBJ FEngine::getInstance()
#define SCENE_OBJ FocalEngine::FEScene::getInstance()
#define TIME_OBJ FocalEngine::FETime::getInstance()

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
	if (!internalCall) TIME_OBJ.beginTimeStamp();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void FEngine::render(bool internalCall)
{
	ENGINE_OBJ.currentCamera->move(cpuTime + gpuTime);
	RENDERER_OBJ.render(currentCamera);

	if (!internalCall) cpuTime = TIME_OBJ.endTimeStamp();
}

void FEngine::endFrame(bool internalCall)
{
	if (!internalCall) TIME_OBJ.beginTimeStamp();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	glfwPollEvents();
	if (!internalCall) gpuTime = TIME_OBJ.endTimeStamp();
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
	colors[ImGuiCol_WindowBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
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
   
	glfwSetMouseButtonCallback(window, &FEInput::mouseButtonCallback);
	glfwSetCursorPosCallback(window, &FEInput::mouseMoveCallback);
	glfwSetKeyCallback(window, &FEInput::keyButtonCallback);

	glClearColor(0.7f, 0.0f, 0.7f, 1.0f);

	// turn off v-sync
	glfwSwapInterval(0);

	currentCamera = new FEFreeCamera(window);
	currentCamera->setAspectRatio(float(width) / float(height));

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));
	FE_GL_ERROR(glEnable(GL_CULL_FACE));
	FE_GL_ERROR(glCullFace(GL_BACK));

	RENDERER_OBJ.standardFBInit(windowW, windowH);
	RENDERER_OBJ.addPostProcess(new FEBloomEffect(RESOURCE_MANAGER_OBJ.getSimpleMesh("plane"), windowW, windowH));
	RENDERER_OBJ.addPostProcess(new FEGammaAndHDRCorrection(RESOURCE_MANAGER_OBJ.getSimpleMesh("plane"), windowW, windowH));

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

	return RESOURCE_MANAGER_OBJ.createPostProcess(ScreenWidth, ScreenHeight, Name);
}
