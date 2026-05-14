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

	enum class FE_VR_CONTROLLER_STATE_CHANGE
	{
		CONNECTED = 0,
		DISCONNECTED = 1,
		RECONNECTED = 2
	};

	enum class FE_VR_CONTROLLER_TYPE
	{
		NONE = 0,
		ANY = 1,
		VALVE_INDEX = 2,
		VIVE = 3
	};

#define FE_DEBUG_VR_CONTROLLER_LOGGING 0

	class FEOpenXRInput
	{
		friend class FEOpenXR;
		friend class FEOpenXRRendering;
	public:
		SINGLETON_PUBLIC_PART(FEOpenXRInput)

		void Init();
		void Shutdown();
		void Update();

		std::pair<glm::vec3, glm::vec3> GetEyeGazeOriginAndDirection();

		bool IsLeftControllerConnectedAndTracked() const;
		bool IsRightControllerConnectedAndTracked() const;

		glm::vec3 GetLeftControllerPosition();
		glm::vec3 GetRightControllerPosition();

		glm::quat GetLeftControllerOrientation();
		glm::quat GetRightControllerOrientation();

		std::string CurrentlyActiveInteractionProfile(bool bLeftController);

		// "Trigger" Button
		void SetLeftTriggerPressCallBack(std::function<void()> UserCallBack);
		void SetRightTriggerPressCallBack(std::function<void()> UserCallBack);

		void SetLeftTriggerReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightTriggerReleaseCallBack(std::function<void()> UserCallBack);

		void SetLeftTriggerTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetLeftTriggerTouchDeactivateCallBack(std::function<void()> UserCallBack);

		void SetRightTriggerTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetRightTriggerTouchDeactivateCallBack(std::function<void()> UserCallBack);

		// "A" Button
		void SetLeftAButtonPressCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonPressCallBack(std::function<void()> UserCallBack);

		void SetLeftAButtonReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonReleaseCallBack(std::function<void()> UserCallBack);

		void SetLeftAButtonTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetLeftAButtonTouchDeactivateCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetRightAButtonTouchDeactivateCallBack(std::function<void()> UserCallBack);

		// "B" Button
		void SetLeftBButtonPressCallBack(std::function<void()> UserCallBack);
		void SetRightBButtonPressCallBack(std::function<void()> UserCallBack);

		void SetLeftBButtonReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightBButtonReleaseCallBack(std::function<void()> UserCallBack);

		void SetLeftBButtonTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetLeftBButtonTouchDeactivateCallBack(std::function<void()> UserCallBack);
		void SetRightBButtonTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetRightBButtonTouchDeactivateCallBack(std::function<void()> UserCallBack);

		// Thumbstick
		void SetLeftThumbstickPressCallBack(std::function<void()> UserCallBack);
		void SetRightThumbstickPressCallBack(std::function<void()> UserCallBack);

		void SetLeftThumbstickReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightThumbstickReleaseCallBack(std::function<void()> UserCallBack);

		void SetLeftThumbstickTouchActivateCallBack(std::function<void()> UserCallBack);
		void SetLeftThumbstickTouchDeactivateCallBack(std::function<void()> UserCallBack);
		void SetRightThumbstickTouchDeactivateCallBack(std::function<void()> UserCallBack);
		void SetRightThumbstickTouchActivateCallBack(std::function<void()> UserCallBack);

		void SetLeftViveSqueezePressCallBack(std::function<void()> UserCallBack);
		void SetRightViveSqueezePressCallBack(std::function<void()> UserCallBack);

		void SetLeftViveSqueezeReleaseCallBack(std::function<void()> UserCallBack);
		void SetRightViveSqueezeReleaseCallBack(std::function<void()> UserCallBack);

		void SetLeftValveSqueezeValueCallBack(std::function<void(float)> UserCallBack);

		void TriggerHapticFeedback(float Amplitude, float Duration, float Frequency, bool bLeftHand);

		void SetLeftViveMenuClickCallBack(std::function<void()> UserCallBack);
		void SetRightViveMenuClickCallBack(std::function<void()> UserCallBack);

		void AddControllerStateChangeCallback(std::function<void(bool, FE_VR_CONTROLLER_STATE_CHANGE)> UserCallBack);
	private:
		SINGLETON_PRIVATE_PART(FEOpenXRInput)

		XrFrameState* FrameState = nullptr;

		struct FEVRControllerActionBindings
		{
			FE_VR_CONTROLLER_TYPE ControllerType;
			std::string OpenXRPath;
			std::vector<XrActionSuggestedBinding> Bindings;
		};

		std::unordered_map<FE_VR_CONTROLLER_TYPE, FEVRControllerActionBindings> SupportedControllersBindings;

		struct InputState
		{
			XrActionSet ActionSet{ nullptr };

			std::array<XrSpace, Side::COUNT> HandSpace = { nullptr, nullptr };
			std::array<XrBool32, Side::COUNT> HandActive;
		};

		std::array<XrPath, Side::COUNT> HandSubactionPath;

		InputState CurrentInputState;

		void CreateaActionSet();
		void InitializeActionsAndControllers();
		void RegisterActionsAndControllers();
		void CreateEyeGazeAction();

		XrSpaceLocation LeftControllerLocation{ XR_TYPE_SPACE_LOCATION };
		XrSpaceLocation RightControllerLocation{ XR_TYPE_SPACE_LOCATION };

		void UpdateControllerSpaceLocation();

		XrSpace EyeSpace = nullptr;
		//XrSpace ViewSpace;
		XrAction EyeGazeAction{ XR_NULL_HANDLE };

		glm::vec3 EyeGazePosition;
		glm::quat EyeGazeOrientation;
		void UpdateEyeGaze();

		struct FEVRActionData
		{
			virtual ~FEVRActionData() = default;

			XrAction ActionHandle{ XR_NULL_HANDLE };
			XrActionType ActionType;
			std::vector<FE_VR_CONTROLLER_TYPE> WorksWith;
			std::string Name = "None";

			std::string LeftComponentPath;
			std::string RightComponentPath;
		};

		struct FEVRActionBooleanData : public FEVRActionData
		{
			FEVRActionBooleanData() : FEVRActionData()
			{
				ActionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
			}

			bool bLeftPressed = false;
			bool bRightPressed = false;

			std::vector<std::function<void()>> LeftActivateUserCallBacks;
			std::vector<std::function<void()>> RightActivateUserCallBacks;

			std::vector<std::function<void()>> LeftDeactivateUserCallBacks;
			std::vector<std::function<void()>> RightDeactivateUserCallBacks;
		};

		void HandleBooleanAction(FEVRActionBooleanData& CurrentActionBoolean);

		struct FEVRActionFloatData : public FEVRActionData
		{
			FEVRActionFloatData() : FEVRActionData()
			{
				ActionType = XR_ACTION_TYPE_FLOAT_INPUT;
			}

			float CurrentValue = 0.0f;

			std::vector<std::function<void(float)>> LeftChangedValueUserCallBacks;
			std::vector<std::function<void(float)>> RightChangedValueUserCallBacks;
		};
		void HandleFloatAction(FEVRActionFloatData& CurrentActionFloat);

		struct FEVRActionPoseData : public FEVRActionData
		{
			FEVRActionPoseData() : FEVRActionData()
			{
				ActionType = XR_ACTION_TYPE_POSE_INPUT;
			}
		};
		void HandlePoseAction(FEVRActionPoseData& CurrentActionPose);

		struct FEVRActionVibrationOutputData : public FEVRActionData
		{
			FEVRActionVibrationOutputData() : FEVRActionData()
			{
				ActionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
			}
		};
		
		std::vector<FEVRActionData*> AllActions;
		void RegisterActionInOpenXR(FEVRActionData& Action);
		FEVRActionData* GetActionDataByName(std::string Name);
		void HandleAction(FEVRActionData* Action);

		void RegisterAllControllersInOpenXR();

		unsigned long long LastFrameInsexLeftControllerWasActive = -1;
		unsigned long long LastFrameInsexRightControllerWasActive = -1;
		void CheckControllerConnectionStatusChanges();
		void TriggerControllerConnectionStatusChange(bool bLeftController, FE_VR_CONTROLLER_STATE_CHANGE Change);
		std::vector<std::function<void(bool, FE_VR_CONTROLLER_STATE_CHANGE)>> ControllerConnectionStatusChangeUserCallBacks;
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetOpenXRInput();
	#define FEOpenXR_INPUT (*static_cast<FEOpenXRInput*>(GetOpenXRInput()))
#else
	#define FEOpenXR_INPUT FEOpenXRInput::GetInstance()
#endif
}