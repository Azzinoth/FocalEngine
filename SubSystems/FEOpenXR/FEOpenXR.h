#pragma once

#include "FEOpenXRInput.h"
#include "FEOpenXRRendering.h"

namespace FocalEngine
{
	class FEOpenXR
	{
	public:
		SINGLETON_PUBLIC_PART(FEOpenXR)

		bool Init(std::string VRAppName = "");
		void Update();

		glm::vec2 EyeResolution();
	private:
		SINGLETON_PRIVATE_PART(FEOpenXR)

		void PollEvents();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetOpenXR();
	#define OpenXR_MANAGER (*static_cast<FEOpenXR*>(GetOpenXR()))
#else
	#define OpenXR_MANAGER FEOpenXR::GetInstance()
#endif
}