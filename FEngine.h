#pragma once

#include "Renderer\FERenderer.h"

namespace FocalEngine
{
	enum FE_RENDER_TARGET_MODE
	{
		FE_GLFW_MODE = 0,
		FE_CUSTOM_MODE = 1,
	};

	class FEngine
	{
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		void InitWindow(int Width = 1920 * 2, int Height = 1080 * 2, std::string WindowTitle = "FEWindow");

		void BeginFrame(bool InternalCall = false);
		void EndFrame(bool InternalCall = false);

		void Render(bool InternalCall = false);

		bool IsWindowOpened();
		void Terminate();

		void SetCamera(FEBasicCamera* NewCamera);
		FEBasicCamera* GetCamera();

		void SetWindowCaption(std::string NewCaption);
		void AddRenderTargetResizeCallback(void(*Func)(int, int));
		void AddWindowResizeCallback(void(*Func)(int, int));
		void AddWindowCloseCallback(void(*Func)());
		void AddKeyCallback(void(*Func)(int, int, int, int));
		void AddMouseButtonCallback(void(*Func)(int, int, int));
		void AddMouseMoveCallback(void(*Func)(double, double));
		void AddDropCallback(void(*Func)(int, const char**));

		int GetWindowWidth();
		int GetWindowHeight();

		void RenderTo(FEFramebuffer* RenderTo);

		double GetCpuTime();
		double GetGpuTime();

		FEPostProcess* CreatePostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void TakeScreenshot(const char* FileName);

		void ResetCamera();

		glm::dvec3 ConstructMouseRay();

		inline FE_RENDER_TARGET_MODE GetRenderTargetMode();
		void SetRenderTargetMode(FE_RENDER_TARGET_MODE NewMode);

		inline int GetRenderTargetWidth();
		void SetRenderTargetSize(int Width, int Height);
		inline int GetRenderTargetHeight();

		inline int GetRenderTargetXShift();
		void SetRenderTargetXShift(int NewRenderTargetXShift);
		inline int GetRenderTargetYShift();
		void SetRenderTargetYShift(int NewRenderTargetYShift);

		void RenderTargetCenterForCamera(FEFreeCamera* Camera);

		void SetClearColor(glm::vec4 ClearColor);

		bool IsSimplifiedRenderingModeActive();
		void ActivateSimplifiedRenderingMode();
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		int WindowW;
		int WindowH;
		std::string WindowTitle;

		double CPUTime, GPUTime;
		double MouseX, MouseY;

		bool bSimplifiedRendering = false;

		const glm::vec4 DefaultClearColor = glm::vec4(0.55f, 0.73f, 0.87f, 1.0f);
		const glm::vec4 DefaultGammaCorrectedClearColor = glm::vec4(pow(0.55f, -2.2f), pow(0.73f, -2.2f), pow(0.87f, -2.2f), 1.0f);

		static FE_RENDER_TARGET_MODE RenderTargetMode;
		int RenderTargetW;
		int RenderTargetH;
		static int RenderTargetXShift;
		static int RenderTargetYShift;
		static void RenderTargetResize();

		std::vector<void(*)(int, int)> ClientRenderTargetResizeCallbacks;
		//void(*clientRenderTargetResizeCallbackImpl)(int, int) = nullptr;

		static void WindowResizeCallback(int Width, int Height);
		std::vector<void(*)(int, int)> ClientWindowResizeCallbacks;

		static void MouseButtonCallback(int Button, int Action, int Mods);
		std::vector<void(*)(int, int, int)> ClientMouseButtonCallbacks;

		static void MouseMoveCallback(double Xpos, double Ypos);
		std::vector<void(*)(double, double)> ClientMouseMoveCallbacks;

		static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
		std::vector<void(*)(int, int, int, int)> ClientKeyButtonCallbacks;

		static void DropCallback(int Count, const char** Paths);
		std::vector<void(*)(int, const char**)> ClientDropCallbacks;

		static void MouseScrollCallback(double Xoffset, double Yoffset);
		std::vector<void(*)(double, double)> ClientMouseScrollCallbacks;

		FEBasicCamera* CurrentCamera = nullptr;
		void SetImguiStyle();
	};

	#define ENGINE FEngine::getInstance()
}