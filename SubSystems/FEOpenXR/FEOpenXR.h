#pragma once

#include "FEOpenXRInput.h"
#include "FEOpenXRRendering.h"

namespace FocalEngine
{
	class FEOpenXR
	{
		friend class FEOpenXRRendering;
	public:
		SINGLETON_PUBLIC_PART(FEOpenXR)

		bool Init();
		void Update();

		glm::vec2 EyeResolution();

		FEEntity* GetVRRigEntity() const;
		FEEntity* GetVRHeadsetEntity() const;

		bool SetCustomVRControllerModel(FEGameModel* CustomGameModel, bool bLeftController);
	private:
		SINGLETON_PRIVATE_PART(FEOpenXR)

		void PollEvents();

		// Scene representation of VR rig, like headset, controllers, etc.
		FEEntity* VRRigEntity = nullptr;
		FEEntity* VRHeadsetEntity = nullptr;
		FEEntity* LeftController = nullptr;
		FEEntity* RightController = nullptr;

		void SceneNodesUpdate();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetOpenXR();
	#define OpenXR_MANAGER (*static_cast<FEOpenXR*>(GetOpenXR()))
#else
	#define OpenXR_MANAGER FEOpenXR::GetInstance()
#endif
}