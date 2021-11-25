#pragma once

#include "Renderer\FERenderer.h"
#include "SubSystems\FEInput.h"

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

		void setWindowCaption(const char* text);
		void setRenderTargetResizeCallback(void(*func)(int, int));
		void setWindowResizeCallback(void(*func)(int, int));
		void setWindowCloseCallback(void(*func)());
		void setKeyCallback(void(*func)(int, int, int, int));
		void setMouseButtonCallback(void(*func)(int, int, int));
		void setMouseMoveCallback(void(*func)(double, double));
		void setDropCallback(void(*func)(int, const char**));

		int getWindowWidth();
		int getWindowHeight();

		void renderTo(FEFramebuffer* renderTo);

		float getCpuTime();
		float getGpuTime();

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

		float cpuTime, gpuTime;
		double mouseX, mouseY;

		static FERenderTargetMode renderTargetMode;
		int renderTargetW;
		int renderTargetH;
		static int renderTargetXShift;
		static int renderTargetYShift;
		static void renderTargetResize();

		GLFWwindow* window;
		static void windowCloseCallback(GLFWwindow* window);
		void(*clientWindowCloseCallbackImpl)() = nullptr;

		void(*clientRenderTargetResizeCallbackImpl)(int, int) = nullptr;

		static void windowResizeCallback(GLFWwindow* window, int width, int height);
		void(*clientWindowResizeCallbackImpl)(int, int) = nullptr;

		static void mouseButtonCallback(int button, int action, int mods);
		void(*clientMouseButtonCallbackImpl)(int, int, int) = nullptr;

		static void mouseMoveCallback(double xpos, double ypos);
		void(*clientMouseMoveCallbackImpl)(double, double) = nullptr;

		static void keyButtonCallback(int key, int scancode, int action, int mods);
		void(*clientKeyButtonCallbackImpl)(int, int, int, int) = nullptr;

		static void dropCallback(int count, const char** paths);
		void(*clientDropCallbackImpl)(int, const char**) = nullptr;

		FEBasicCamera* currentCamera = nullptr;
		void setImguiStyle();
	};
}

#define ENGINE FEngine::getInstance()
#define RENDERER FERenderer::getInstance()
#define RESOURCE_MANAGER FEResourceManager::getInstance()
#define SCENE FEScene::getInstance()