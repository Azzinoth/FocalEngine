#pragma once

#include "Renderer\FERenderer.h"
#include "SubSystems\FEInput.h"

// CoreExtensions
//#include "../CoreExtensions/StandardMaterial/SolidColorMaterial/FESolidColorMaterial.h"
//#include "../CoreExtensions/StandardMaterial/PhongMaterial/FEPhongMaterial.h"

namespace FocalEngine
{
	class FEngine
	{
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		void createWindow(int width = 1280, int height = 720, std::string WindowTitle = "FEWindow");

		void beginFrame(bool internalCall = false);
		void endFrame(bool internalCall = false);

		void render(bool internalCall = false);

		bool isWindowOpened();

		void setCamera(FEBasicCamera* newCamera);
		FEBasicCamera* getCamera();

		void setWindowCaption(const char* text);
		void setKeyCallback(void(*func)(int, int, int, int));
		void setMouseButtonCallback(void(*func)(int, int, int));
		void setMouseMoveCallback(void(*func)(double, double));

		int getWindowWidth();
		int getWindowHeight();

		void renderTo(FEFramebuffer* renderTo);

		float getCpuTime();
		float getGpuTime();

		FEPostProcess* createPostProcess(int ScreenWidth = -1, int ScreenHeight = -1);
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		int windowW;
		int windowH;
		std::string windowTitle;

		float cpuTime, gpuTime;

		GLFWwindow* window;
		static void mouseButtonCallback(int button, int action, int mods);
		void(*clientMouseButtonCallbackImpl)(int, int, int) = nullptr;

		static void mouseMoveCallback(double xpos, double ypos);
		void(*clientMouseMoveCallbackImpl)(double, double) = nullptr;

		static void keyButtonCallback(int key, int scancode, int action, int mods);
		void(*clientKeyButtonCallbackImpl)(int, int, int, int) = nullptr;

		FEBasicCamera* currentCamera = nullptr;
	};
}