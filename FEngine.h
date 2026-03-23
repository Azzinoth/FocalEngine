#pragma once

#include "Renderer\FERenderer.h"
#include "..\SubSystems\FEOpenXR\FEOpenXR.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEngine
	{
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		std::string GetEngineVersion();        // "1.0.0"
		std::string GetEngineBuildInfo();      // "build 231+52 (dev, ed4c7ce-dirty)"
		std::string GetFullVersion();          // "Focal Engine 1.0.0 build 231+52 (dev, ed4c7ce-dirty)"
		std::string GetEngineBuildTimestamp(); // "20260207232613"
		int GetEngineBuildNumber();            // 231

		void InitWindow(int Width = 1920 * 2, int Height = 1080 * 2, std::string WindowTitle = "FEWindow");

		void BeginFrame(bool bInternalCall = false);
		void EndFrame(bool bInternalCall = false);

		void Render(bool bInternalCall = false);

		bool IsNotTerminated();
		void Terminate();

		void SetWindowCaption(std::string NewCaption);
		void AddWindowResizeCallback(void(*Func)(int, int));
		void AddWindowCloseCallback(void(*Func)());
		void AddDropCallback(void(*Func)(int, const char**));

		void RenderTo(FEFramebuffer* RenderTo);

		double GetCpuTime();
		double GetGpuTime();

		FEPostProcess* CreatePostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith);

		FEViewport* GetDefaultViewport();
		FEViewport* GetViewport(std::string ViewportID);

		bool IsVsyncEnabled();
		void SetVsyncEnabled(bool NewValue);

		void DisableVR();
		bool EnableVR(FERenderingPipeline VRRenderingPipeline = FERenderingPipeline::Deferred);
		FERenderingPipeline GetVRRenderingPipeline() const;
		bool IsVRInitializedCorrectly();
		bool IsVREnabled();

		void AddOnAfterUpdateCallback(std::function<void()> Callback);

		// Returns Viewport ID
		std::string CreateViewport(ImGuiWindow* ImGuiWindowPointer);
		// Returns Viewport ID
		std::string CreateViewport(FEWindow* FEWindowPointer);

		void AddOnViewportMovedCallback(std::function<void(std::string)> Callback);
		void AddOnViewportResizeCallback(std::function<void(std::string)> Callback);

		unsigned long long GetCurrentFrameIndex();

		std::string GetVRApplicationVisibleName() const;
		void SetVRApplicationVisibleName(const std::string& NewName);
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		double CPUTime = 0.0, GPUTime = 0.0;
		double CurrentDeltaTime = 0.0;

		bool bVsyncEnabled = true;

		bool bVRInitializedCorrectly = false;
		bool bVRActive = false;
		std::string VRApplicationVisibleName = "VR Focal Engine Application";

		static void WindowResizeCallback(int Width, int Height);
		std::vector<void(*)(int, int)> ClientWindowResizeCallbacks;

		static void DropCallback(int Count, const char** Paths);
		std::vector<void(*)(int, const char**)> ClientDropCallbacks;

		void InternalUpdate();
		std::vector<std::function<void()>> OnAfterUpdateCallbacks;

		std::vector<FEViewport*> Viewports;
		std::vector<std::function<void(std::string)>> OnViewportMovedCallbacks;
		std::vector<std::function<void(std::string)>> OnViewportResizeCallbacks;

		void ViewportCheckForModification();
		void ViewportCheckForModificationIndividual(FEViewport* ViewPort, bool& bMoved, bool& bResize);

		unsigned long long CurentFrameIndex = 0;
		FERenderingPipeline VRRenderingPipeline = FERenderingPipeline::Deferred;
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetEngine();
	#define ENGINE (*static_cast<FEngine*>(GetEngine()))
#else
	#define ENGINE FEngine::GetInstance()
#endif
}