#pragma once

#include "FEOpenXRCore.h"

namespace FocalEngine
{
	namespace Side
	{
		const int LEFT = 0;
		const int RIGHT = 1;
		const int COUNT = 2;
	}

	class FEOpenXRInput
	{
		friend class FEOpenXR;
		friend class FEOpenXRRendering;
	public:
		SINGLETON_PUBLIC_PART(FEOpenXRInput)

		void Init();
		void Update();

		glm::vec3 GetLeftControllerPosition();
		glm::vec3 GetRightControllerPosition();

		glm::quat GetLeftControllerOrientation();
		glm::quat GetRightControllerOrientation();

		std::string CurrentlyActiveInteractionProfile(bool bLeftController);

		void SetLeftTriggerPressCallBack(std::function<void ()> UserCallBack);
		void SetRightTriggerPressCallBack(std::function<void()> UserCallBack);

		void SetLeftTriggerReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightTriggerReleaseCallBack(std::function<void()> UserCallBack);

		// A Button
		void SetLeftAButtonPressCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonPressCallBack(std::function<void()> UserCallBack);

		void SetLeftAButtonReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonReleaseCallBack(std::function<void()> UserCallBack);

		// Squeeze button click
	private:
		SINGLETON_PRIVATE_PART(FEOpenXRInput)

		XrFrameState* FrameState = nullptr;

		std::vector<XrActionSuggestedBinding> ValveIndexBindings;

		struct InputState
		{
			XrActionSet actionSet{ XR_NULL_HANDLE };
			XrAction grabAction{ XR_NULL_HANDLE };
			XrAction poseAction{ XR_NULL_HANDLE };
			XrAction triggerClickAction{ XR_NULL_HANDLE };
			XrAction triggerValue{ XR_NULL_HANDLE };

			XrAction AButtonClickAction{ XR_NULL_HANDLE };
			XrAction BButtonClickAction{ XR_NULL_HANDLE };

			XrAction HapticAction{ XR_NULL_HANDLE };

			XrAction SqueezeButtonClick{ XR_NULL_HANDLE };
			

			std::array<XrPath, Side::COUNT> handSubactionPath;
			std::array<XrSpace, Side::COUNT> handSpace;
			std::array<float, Side::COUNT> handScale = { {1.0f, 1.0f} };
			std::array<XrBool32, Side::COUNT> handActive;
		};

		InputState CurrentInputState;

		void InitializeActions();
		void CreateActions();

		XrSpaceLocation LeftControllerLocation{ XR_TYPE_SPACE_LOCATION };
		XrSpaceLocation RightControllerLocation{ XR_TYPE_SPACE_LOCATION };

		void UpdateControllerSpaceLocation();

		bool bLeftTriggerPressed = false;
		bool bRightTriggerPressed = false;

		std::vector<std::function<void()>> LeftTriggerPressUserCallBacks;
		std::vector<std::function<void()>> RightTriggerPressUserCallBacks;
		void ActivateLeftTriggerPressUserCallBacks();
		void ActivateRightTriggerPressUserCallBacks();

		std::vector<std::function<void()>> LeftTriggerReleaseUserCallBacks;
		std::vector<std::function<void()>> RightTriggerReleaseUserCallBacks;
		void ActivateLeftTriggerReleaseUserCallBacks();
		void ActivateRightTriggerReleaseUserCallBacks();

		// A Button
		bool bLeftAButtonPressed = false;
		bool bRightAButtonPressed = false;

		std::vector<std::function<void()>> LeftAButtonPressUserCallBacks;
		std::vector<std::function<void()>> RightAButtonPressUserCallBacks;
		void ActivateLeftAButtonPressUserCallBacks();
		void ActivateRightAButtonPressUserCallBacks();

		std::vector<std::function<void()>> LeftAButtonReleaseUserCallBacks;
		std::vector<std::function<void()>> RightAButtonReleaseUserCallBacks;
		void ActivateLeftAButtonReleaseUserCallBacks();
		void ActivateRightAButtonReleaseUserCallBacks();
	};

#define FEOpenXR_INPUT FEOpenXRInput::getInstance()
}