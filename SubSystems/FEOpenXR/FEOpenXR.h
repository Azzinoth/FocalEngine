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
		FEEntity* GetLeftControllerEntity() const;
		FEEntity* GetRightControllerEntity() const;

		bool SetCustomVRControllerModel(FEGameModel* CustomGameModel, bool bLeftController);
	private:
		SINGLETON_PRIVATE_PART(FEOpenXR)

		void PollEvents();

		const glm::vec3 StandardControllerScale = glm::vec3(5.0f);

		// Scene representation of VR rig, like headset, controllers, etc.
		FEEntity* VRRigEntity = nullptr;
		FEEntity* VRHeadsetEntity = nullptr;
		FEEntity* LeftController = nullptr;
		FEEntity* RightController = nullptr;

		void SceneNodesUpdate();

		bool TryToAddVRRigToScene(FEScene* Scene);
		static void OnControllerConnectionChanges(bool bLeftController, FE_VR_CONTROLLER_STATE_CHANGE Change);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetOpenXR();
	#define OpenXR_MANAGER (*static_cast<FEOpenXR*>(GetOpenXR()))
#else
	#define OpenXR_MANAGER FEOpenXR::GetInstance()
#endif
}