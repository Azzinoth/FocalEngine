#pragma once

#include "../Scene/FESceneManager.h"

// Forward declarations, so that LeiaSR SDK headers are only needed in Leia_3D_Manager.cpp.
namespace SR
{
	class SRContext;
	class SwitchableLensHint;
	class IGLWeaver1;
}

namespace FocalEngine
{
	class FOCAL_ENGINE_API Leia3DManager
	{
		friend class FEngine;
	public:
		SINGLETON_PUBLIC_PART(Leia3DManager)

		bool Set3DModeEnabled(bool bNewValue);
		bool Is3DModeEnabled() const;

		// View port is needed because LeiaSR SDK creates image that is valid for specific position on screen and window size.
		bool Initialize(const std::string& SceneID, const std::string& ViewPortID);
		FEScene* GetCurrentScene() const;
		
		void Render();

		float GetWorldUnitsPerMillimeter() const;
		void SetWorldUnitsPerMillimeter(float NewValue);

		// Eye frustums are drawn with debug lines, which are rendered for every camera, so they are also visible in 3D view itself.
		bool AreEyeFrustumsVisible() const;
		void SetEyeFrustumsVisible(bool bNewValue);

		// Outlines of the physical display and of the 3D area on it, drawn with debug lines like eye frustums.
		bool IsMonitorVisible() const;
		void SetMonitorVisible(bool bNewValue);

		FETexture* GetFinalResult() const;
	private:
		SINGLETON_PRIVATE_PART(Leia3DManager)

		SR::SRContext* Context = nullptr;
		SR::SwitchableLensHint* LensHint = nullptr;
		SR::IGLWeaver1* Weaver = nullptr;
		bool b3DModeEnabled = false;

		std::string SceneID = "";
		std::string ViewPortID = "";
		void InitializeScene();

		FEEntity* MonitorEntity = nullptr;
		FEEntity* LeftEye = nullptr;
		FEEntity* RightEye = nullptr;

		float WorldUnitsPerMillimeter = 0.01f;
		bool bEyeFrustumsVisible = false;
		bool bMonitorVisible = false;
		// Places eye cameras according to eye tracker and sets their off-axis projections.
		void UpdateEyeCameras();
		// EyePosition is in millimeters, relative to the center of the 3D area.
		void UpdateEyeCamera(FEEntity* Eye, glm::vec3 EyePosition, float AreaWidth, float AreaHeight);
		// Rectangle in the plane of the 3D monitor area.
		void DebugDrawRectangleOnMonitor(glm::vec2 Center, glm::vec2 Size, glm::vec3 Color);

		// Left eye view in the left half, right eye view in the right half.
		FEFramebuffer* StereoViewsFramebuffer = nullptr;
		GLuint EyeResultFramebuffer = 0;
		int StereoViewsViewWidth = 0;
		int StereoViewsViewHeight = 0;
		void UpdateStereoViewsTexture(int ViewWidth, int ViewHeight);

		// Same size as window framebuffer, so "3D" pixels have the same coordinates as they would have in window framebuffer.
		// Its color attachment is the final result, framebuffer owns it and deletes it together with itself.
		FEFramebuffer* FinalResultFramebuffer = nullptr;
		void UpdateFinalResult(int Width, int Height);
		// Weaves stereo views into the final result, at the viewport rectangle that eye cameras were rendered for.
		void RenderFinalResult();

		bool InitializeContext(bool bInitialLensState);
		void CreateWeaver();
		void Shutdown();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetLeia3DManager();
	#define LEIA_3D_MANAGER (*static_cast<Leia3DManager*>(GetLeia3DManager()))
#else
	#define LEIA_3D_MANAGER Leia3DManager::GetInstance()
#endif
}