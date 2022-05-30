#pragma once

#include "Renderer\FERenderer.h"
#include "SubSystems\FELog.h"

namespace FocalEngine
{
	enum FERenderTargetMode
	{
		FE_GLFW_MODE = 0,
		FE_CUSTOM_MODE = 1,
	};

	class FEngine
	{
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		void createWindow(int width = 1920 * 2, int height = 1080 * 2, std::string WindowTitle = "FEWindow");

		void beginFrame(bool internalCall = false);
		void endFrame(bool internalCall = false);

		void render(bool internalCall = false);

		bool isWindowOpened();
		void terminate();

		void setCamera(FEBasicCamera* newCamera);
		FEBasicCamera* getCamera();

		void setWindowCaption(std::string newCaption);
		void addRenderTargetResizeCallback(void(*func)(int, int));
		void addWindowResizeCallback(void(*func)(int, int));
		void addWindowCloseCallback(void(*func)());
		void addKeyCallback(void(*func)(int, int, int, int));
		void addMouseButtonCallback(void(*func)(int, int, int));
		void addMouseMoveCallback(void(*func)(double, double));
		void addDropCallback(void(*func)(int, const char**));

		int getWindowWidth();
		int getWindowHeight();

		void renderTo(FEFramebuffer* renderTo);

		double getCpuTime();
		double getGpuTime();

		FEPostProcess* createPostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void takeScreenshot(const char* fileName);

		void resetCamera();

		glm::dvec3 constructMouseRay();

		inline FERenderTargetMode getRenderTargetMode();
		void setRenderTargetMode(FERenderTargetMode newMode);

		inline int getRenderTargetWidth();
		void setRenderTargetSize(int width, int height);
		inline int getRenderTargetHeight();

		inline int getRenderTargetXShift();
		void setRenderTargetXShift(int newRenderTargetXShift);
		inline int getRenderTargetYShift();
		void setRenderTargetYShift(int newRenderTargetYShift);

		void renderTargetCenterForCamera(FEFreeCamera* camera);
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		int windowW;
		int windowH;
		std::string windowTitle;

		double cpuTime, gpuTime;
		double mouseX, mouseY;

		static FERenderTargetMode renderTargetMode;
		int renderTargetW;
		int renderTargetH;
		static int renderTargetXShift;
		static int renderTargetYShift;
		static void renderTargetResize();

		std::vector<void(*)(int, int)> clientRenderTargetResizeCallbacks;
		//void(*clientRenderTargetResizeCallbackImpl)(int, int) = nullptr;

		static void windowResizeCallback(int width, int height);
		std::vector<void(*)(int, int)> clientWindowResizeCallbacks;

		static void mouseButtonCallback(int button, int action, int mods);
		std::vector<void(*)(int, int, int)> clientMouseButtonCallbacks;

		static void mouseMoveCallback(double xpos, double ypos);
		std::vector<void(*)(double, double)> clientMouseMoveCallbacks;

		static void keyButtonCallback(int key, int scancode, int action, int mods);
		std::vector<void(*)(int, int, int, int)> clientKeyButtonCallbacks;

		static void dropCallback(int count, const char** paths);
		std::vector<void(*)(int, const char**)> clientDropCallbacks;

		FEBasicCamera* currentCamera = nullptr;
		void setImguiStyle();
	};
}

#define ENGINE FEngine::getInstance()
#define RENDERER FERenderer::getInstance()
#define RESOURCE_MANAGER FEResourceManager::getInstance()
#define SCENE FEScene::getInstance()