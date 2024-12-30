#include "FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetEngine()
{
	return FEngine::GetInstancePointer();
}
#endif

FEngine::FEngine()
{
}

FEngine::~FEngine()
{
}

#include "ResourceManager/Timestamp.h"
std::string FEngine::GetEngineBuildVersion()
{
	return ENGINE_BUILD_TIMESTAMP;
}

bool FEngine::IsNotTerminated()
{
	return APPLICATION.IsNotTerminated();
}

void FEngine::InternalUpdate()
{
	CurrentDeltaTime = CPUTime + GPUTime;

	ViewportCheckForModification();
	
	INSTANCED_RENDERING_SYSTEM.Update();
	SCENE_MANAGER.Update();
	CAMERA_SYSTEM.Update(CurrentDeltaTime);
	VIRTUAL_UI_SYSTEM.Update();

	NATIVE_SCRIPT_SYSTEM.Update(CurrentDeltaTime);

	for (size_t i = 0; i < OnAfterUpdateCallbacks.size(); i++)
	{
		if (OnAfterUpdateCallbacks[i] == nullptr)
			continue;

		OnAfterUpdateCallbacks[i]();
	}

	// Instead of updating TRANSFORM_SYSTEM in the beginning of the frame, we update it here.
	// To ensure that all the other systems are updated before the TRANSFORM_SYSTEM will kick in.
	TRANSFORM_SYSTEM.Update();

	INPUT.Update();
}

void FEngine::BeginFrame(const bool InternalCall)
{
	if (!APPLICATION.IsNotTerminated())
		return;

	if (!InternalCall)
		TIME.BeginTimeStamp();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	APPLICATION.BeginFrame();
	if (APPLICATION.GetMainWindow() == nullptr)
		return;

	APPLICATION.GetMainWindow()->BeginFrame();

#ifdef FE_DEBUG_ENABLED
	std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShaderIDList();
	const std::vector<std::string> TempList = RESOURCE_MANAGER.GetEnginePrivateShaderIDList();
	for (size_t i = 0; i < TempList.size(); i++)
	{
		ShaderList.push_back(TempList[i]);
	}

	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		if (RESOURCE_MANAGER.GetShader(ShaderList[i])->IsDebugRequest())
		{
			RESOURCE_MANAGER.GetShader(ShaderList[i])->ThisFrameDebugBind = 0;
		}
	}
#endif

	InternalUpdate();
}

void FEngine::Render(const bool InternalCall)
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable);
	for (size_t i = 0; i < ActiveScenes.size(); i++)
	{
		RENDERER.Render(ActiveScenes[i]);
	}

	if (bVRActive)
	{
		OpenXR_MANAGER.Update();
		RENDERER.SetGLViewport(0, 0, ENGINE.GetDefaultViewport()->GetWidth(), ENGINE.GetDefaultViewport()->GetHeight());
	}

	APPLICATION.GetMainWindow()->Render();

	if (!InternalCall) CPUTime = TIME.EndTimeStamp();
}

void FEngine::EndFrame(const bool InternalCall)
{
	INPUT.EndFrame();

	if (!InternalCall) TIME.BeginTimeStamp();
	APPLICATION.GetMainWindow()->EndFrame();
	APPLICATION.EndFrame();
	if (!InternalCall) GPUTime = TIME.EndTimeStamp();

	// FIX ME! This is a temporary solution.
	if (RESOURCE_MANAGER.PrivateEngineAssetPackage != nullptr)
	{
		FILE_SYSTEM.DeleteDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems");
		delete RESOURCE_MANAGER.PrivateEngineAssetPackage;
		RESOURCE_MANAGER.PrivateEngineAssetPackage = nullptr;
	}
}

void FEngine::InitWindow(const int Width, const int Height, std::string WindowTitle)
{
	FEWindow* NewWindow = APPLICATION.AddWindow(Width, Height, WindowTitle);
	// Early initialization of INPUT system.
	INPUT;
	APPLICATION.GetMainWindow()->AddOnResizeCallback(&FEngine::WindowResizeCallback);
	APPLICATION.GetMainWindow()->AddOnDropCallback(&FEngine::DropCallback);
	AddViewport(NewWindow);

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	// tessellation parameter
	FE_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));

	RENDERER.Init();

	// Early initialization of the systems.
	TRANSFORM_SYSTEM;
	LIGHT_SYSTEM;
	CAMERA_SYSTEM;
	INSTANCED_RENDERING_SYSTEM;
	TERRAIN_SYSTEM;
	SKY_DOME_SYSTEM;
	PREFAB_INSTANCE_SYSTEM;
	VIRTUAL_UI_SYSTEM;
	NATIVE_SCRIPT_SYSTEM;
}

void FEngine::SetWindowCaption(const std::string NewCaption)
{
	if (APPLICATION.GetMainWindow() != nullptr)
		APPLICATION.GetMainWindow()->SetTitle(NewCaption);
}

void FEngine::AddWindowResizeCallback(void(*Func)(int, int))
{
	if (Func != nullptr)
		ClientWindowResizeCallbacks.push_back(Func);
}

void FEngine::AddWindowCloseCallback(void(*Func)())
{
	APPLICATION.GetMainWindow()->AddOnCloseCallback(Func);
}

void FEngine::WindowResizeCallback(const int Width, const int Height)
{
	for (size_t i = 0; i < ENGINE.ClientWindowResizeCallbacks.size(); i++)
	{
		if (ENGINE.ClientWindowResizeCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientWindowResizeCallbacks[i](Width, Height);
	}
}

void FEngine::RenderTo(FEFramebuffer* RenderTo)
{
	RenderTo->Bind();
	BeginFrame(true);
	Render(true);
	RenderTo->UnBind();
}

double FEngine::GetCpuTime()
{
	return CPUTime;
}

double FEngine::GetGpuTime()
{
	return GPUTime;
}

FEPostProcess* FEngine::CreatePostProcess(const std::string Name, int ScreenWidth, int ScreenHeight)
{
	if (ScreenWidth < 2 || ScreenHeight < 2)
	{
		ScreenWidth = ENGINE.GetDefaultViewport()->GetWidth();
		ScreenHeight = ENGINE.GetDefaultViewport()->GetHeight();
	}

	return RESOURCE_MANAGER.CreatePostProcess(ScreenWidth, ScreenHeight, Name);
}

void FEngine::Terminate()
{
	APPLICATION.Close();
}

void FEngine::SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith)
{
	RENDERER.SaveScreenshot(FileName, SceneToWorkWith);
}

void FEngine::DropCallback(const int Count, const char** Paths)
{
	for (size_t i = 0; i < ENGINE.ClientDropCallbacks.size(); i++)
	{
		if (ENGINE.ClientDropCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientDropCallbacks[i](Count, Paths);
	}
}

void FEngine::AddDropCallback(void(*Func)(int, const char**))
{
	if (Func != nullptr)
		ClientDropCallbacks.push_back(Func);
}

bool FEngine::IsVsyncEnabled()
{
	return bVsyncEnabled;
}

void FEngine::SetVsyncEnabled(bool NewValue)
{
	bVsyncEnabled = NewValue;
	if (bVsyncEnabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}
}

void FEngine::DisableVR()
{
	bVRActive = false;
	RENDERER.bVRActive = false;
}

bool FEngine::EnableVR()
{
	if (!bVRInitializedCorrectly)
	{
		bVRInitializedCorrectly = OpenXR_MANAGER.Init(APPLICATION.GetMainWindow()->GetTitle());
	}

	if (bVRInitializedCorrectly)
	{
		bVRActive = true;
		RENDERER.bVRActive = true;
		RENDERER.UpdateVRRenderTargetSize(static_cast<int>(OpenXR_MANAGER.EyeResolution().x), static_cast<int>(OpenXR_MANAGER.EyeResolution().y));
	}
	else
	{
		bVRActive = false;
		RENDERER.bVRActive = false;
	}

	return bVRActive;
}

bool FEngine::IsVRInitializedCorrectly()
{
	return bVRInitializedCorrectly;
}

bool FEngine::IsVREnabled()
{
	return bVRActive;
}

void FEngine::AddOnAfterUpdateCallback(std::function<void()> Callback)
{
	OnAfterUpdateCallbacks.push_back(Callback);
}

std::string FEngine::AddViewport(ImGuiWindow* ImGuiWindowPointer)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->WindowHandle == ImGuiWindowPointer)
			return Viewports[i]->ID;
	}

	FEViewport* NewViewport = new FEViewport();
	NewViewport->Type = FE_VIEWPORT_IMGUI_WINDOW;
	NewViewport->WindowHandle = ImGuiWindowPointer;
	
	Viewports.push_back(NewViewport);
	bool bMoved, bResize;
	ViewportCheckForModificationIndividual(NewViewport, bMoved, bResize);

	return NewViewport->ID;
}

std::string FEngine::AddViewport(FEWindow* FEWindowPointer)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->WindowHandle == FEWindowPointer)
			return Viewports[i]->ID;
	}

	FEViewport* NewViewport = new FEViewport();
	NewViewport->Type = FE_VIEWPORT_FEWINDOW;
	NewViewport->WindowHandle = FEWindowPointer;

	Viewports.push_back(NewViewport);
	bool bMoved, bResize;
	ViewportCheckForModificationIndividual(NewViewport, bMoved, bResize);

	return NewViewport->ID;
}

FEViewport* FEngine::GetViewport(std::string ViewportID)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->ID == ViewportID)
			return Viewports[i];
	}

	return nullptr;
}

void FEngine::ViewportCheckForModificationIndividual(FEViewport* ViewPort, bool& bMoved, bool& bResize)
{
	bMoved = false;
	bResize = false;

	switch (ViewPort->Type)
	{
		case FE_VIEWPORT_NULL:
		 return;

		case FE_VIEWPORT_OS_WINDOW:
			return;

		case FE_VIEWPORT_GLFW_WINDOW:
			return;
		
		case FE_VIEWPORT_FEWINDOW:
		{
			FEWindow* Window = static_cast<FEWindow*>(ViewPort->WindowHandle);

			if (ViewPort->X != 0 || ViewPort->Y != 0)
				bMoved = true;

			ViewPort->X = 0;
			ViewPort->Y = 0;

			if (ViewPort->Width != Window->GetWidth() || ViewPort->Height != Window->GetHeight())
				bResize = true;

			ViewPort->Width = Window->GetWidth();
			ViewPort->Height = Window->GetHeight();

			return;
		}

		case FE_VIEWPORT_IMGUI_WINDOW:
		{
			ImGuiWindow* Window = static_cast<ImGuiWindow*>(ViewPort->WindowHandle);

			if (ViewPort->X != static_cast<int>(Window->ContentRegionRect.GetTL().x) || ViewPort->Y != static_cast<int>(Window->ContentRegionRect.GetTL().y))
				bMoved = true;

			ViewPort->X = static_cast<int>(Window->ContentRegionRect.GetTL().x);
			ViewPort->Y = static_cast<int>(Window->ContentRegionRect.GetTL().y);

			if (ViewPort->Width != static_cast<int>(Window->ContentRegionRect.GetWidth()) || ViewPort->Height != static_cast<int>(Window->ContentRegionRect.GetHeight()))
				bResize = true;

			ViewPort->Width = static_cast<int>(Window->ContentRegionRect.GetWidth());
			ViewPort->Height = static_cast<int>(Window->ContentRegionRect.GetHeight());

			return;
		}
	}
}

void FEngine::AddOnViewportMovedCallback(std::function<void(std::string)> Callback)
{
	OnViewportMovedCallbacks.push_back(Callback);
}

void FEngine::ViewportCheckForModification()
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		bool bMoved, bResize;
		ViewportCheckForModificationIndividual(Viewports[i], bMoved, bResize);

		if (bMoved)
		{
			for (size_t j = 0; j < OnViewportMovedCallbacks.size(); j++)
			{
				if (OnViewportMovedCallbacks[j] != nullptr)
					OnViewportMovedCallbacks[j](Viewports[i]->ID);
			}
		}

		if (bResize)
		{
			for (size_t j = 0; j < OnViewportResizeCallbacks.size(); j++)
			{
				if (OnViewportResizeCallbacks[j] != nullptr)
					OnViewportResizeCallbacks[j](Viewports[i]->ID);
			}
		}
	}
}

void FEngine::AddOnViewportResizeCallback(std::function<void(std::string)> Callback)
{
	OnViewportResizeCallbacks.push_back(Callback);
}

FEViewport* FEngine::GetDefaultViewport()
{
	if (Viewports.size() == 0)
		return nullptr;

	return Viewports[0];
}