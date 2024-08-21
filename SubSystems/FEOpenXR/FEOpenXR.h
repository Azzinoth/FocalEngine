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

#define OpenXR_MANAGER FEOpenXR::GetInstance()
}