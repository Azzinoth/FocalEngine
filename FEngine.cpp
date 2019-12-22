#include "FEngine.h"
using namespace FocalEngine;

FEngine* FEngine::_instance = nullptr;
#define RENDERER_OBJ FERenderer::getInstance()
#define RESOURCE_MANAGER_OBJ FEResourceManager::getInstance()
#define ENGINE_OBJ FEngine::getInstance()

FEngine::FEngine()
{
	currentCamera = new FEBasicCamera();
	FEInput::getInstance().mouseButtonCallbackImpl = &FEngine::mouseButtonCallback;
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

void FEngine::beginFrame()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RENDERER_OBJ.render(currentCamera);
}

void FEngine::endFrame()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void FEngine::createWindow(int width, int height, std::string WindowTitle)
{
	windowW = width;
	windowH = height;
	windowTitle = WindowTitle;

	currentCamera->setAspectRatio(float(width) / float(height));

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
	glfwSetKeyCallback(window, &FEInput::keyButtonCallback);

	glClearColor(0.7f, 0.0f, 0.7f, 1.0f);

	// Force subsystems start up
	//FEResourceManager resourceManager = FEResourceManager::getInstance();
	//FERenderer renderer = FERenderer::getInstance();
}

void FEngine::setKeyCallback(void(*func)(int, int, int, int))
{
	clientKeyButtonCallbackImpl = func;
}

void FEngine::setMouseCallback(void(*func)(int, int, int))
{
	clientMouseButtonCallbackImpl = func;
}

void FEngine::mouseButtonCallback(int button, int action, int mods)
{
	FEngine& engineObj = getInstance();
	if (engineObj.clientMouseButtonCallbackImpl != nullptr)
		engineObj.clientMouseButtonCallbackImpl(button, action, mods);
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
