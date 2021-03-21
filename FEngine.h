#pragma once

#include "Renderer\FERenderer.h"
#include "SubSystems\FEInput.h"

namespace FocalEngine
{
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
		void setWindowResizeCallback(void(*func)(int, int));
		void setWindowCloseCallback(void(*func)());
		void setKeyCallback(void(*func)(int, int, int, int));
		void setMouseButtonCallback(void(*func)(int, int, int));
		void setMouseMoveCallback(void(*func)(double, double));

		int getWindowWidth();
		int getWindowHeight();

		void renderTo(FEFramebuffer* renderTo);

		float getCpuTime();
		float getGpuTime();

		FEPostProcess* createPostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void takeScreenshot(const char* fileName);

		void resetCamera();

		glm::dvec3 constructMouseRay();
		//GLFWwindow* window;

		//GLFWwindow* test_offscreen_context = nullptr;
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		int windowW;
		int windowH;
		std::string windowTitle;

		float cpuTime, gpuTime;
		double mouseX, mouseY;

		GLFWwindow* window;
		static void windowCloseCallback(GLFWwindow* window);
		void(*clientWindowCloseCallbackImpl)() = nullptr;

		static void windowResizeCallback(GLFWwindow* window, int width, int height);
		void(*clientWindowResizeCallbackImpl)(int, int) = nullptr;

		static void mouseButtonCallback(int button, int action, int mods);
		void(*clientMouseButtonCallbackImpl)(int, int, int) = nullptr;

		static void mouseMoveCallback(double xpos, double ypos);
		void(*clientMouseMoveCallbackImpl)(double, double) = nullptr;

		static void keyButtonCallback(int key, int scancode, int action, int mods);
		void(*clientKeyButtonCallbackImpl)(int, int, int, int) = nullptr;

		FEBasicCamera* currentCamera = nullptr;
		void setImguiStyle();
	};
}

#define ENGINE FEngine::getInstance()
#define RENDERER FERenderer::getInstance()
#define RESOURCE_MANAGER FEResourceManager::getInstance()
#define SCENE FEScene::getInstance()