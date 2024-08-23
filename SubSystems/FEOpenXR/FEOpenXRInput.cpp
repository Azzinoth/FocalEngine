#include "FEOpenXRInput.h"

using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetOpenXRInput()
{
    return FEOpenXRInput::GetInstancePointer();
}
#endif

FEOpenXRInput::FEOpenXRInput()
{
    FEVRControllerActionBindings ValveIndex;
    ValveIndex.ControllerType = FE_VR_CONTROLLER_TYPE::FE_VR_CONTROLLER_TYPE_VALVE_INDEX;
    ValveIndex.OpenXRPath = "/interaction_profiles/valve/index_controller";
    SupportedControllersBindings[FE_VR_CONTROLLER_TYPE_VALVE_INDEX] = ValveIndex;

    FEVRControllerActionBindings Vive;
    Vive.ControllerType = FE_VR_CONTROLLER_TYPE::FE_VR_CONTROLLER_TYPE_VIVE;
    Vive.OpenXRPath = "/interaction_profiles/htc/vive_controller";
    SupportedControllersBindings[FE_VR_CONTROLLER_TYPE_VIVE] = Vive;

    FEVRActionPoseData* ControllerPose = new FEVRActionPoseData();
    ControllerPose->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_ANY);
    ControllerPose->Name = "controller_pose";
    ControllerPose->LeftComponentPath = "/user/hand/left/input/grip/pose";
    ControllerPose->RightComponentPath = "/user/hand/right/input/grip/pose";
    AllActions.push_back(ControllerPose);

    FEVRActionBooleanData* TriggerClick = new FEVRActionBooleanData();
    TriggerClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_ANY);
    TriggerClick->Name = "trigger_click";
    TriggerClick->LeftComponentPath = "/user/hand/left/input/trigger/click";
    TriggerClick->RightComponentPath = "/user/hand/right/input/trigger/click";
    AllActions.push_back(TriggerClick);

    FEVRActionBooleanData* AButtonClick = new FEVRActionBooleanData();
    AButtonClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    AButtonClick->Name = "a_button_click";
    AButtonClick->LeftComponentPath = "/user/hand/left/input/a/click";
    AButtonClick->RightComponentPath = "/user/hand/right/input/a/click";
    AllActions.push_back(AButtonClick);

    FEVRActionBooleanData* BButtonClick = new FEVRActionBooleanData();
    BButtonClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    BButtonClick->Name = "b_button_click";
    BButtonClick->LeftComponentPath = "/user/hand/left/input/b/click";
    BButtonClick->RightComponentPath = "/user/hand/right/input/b/click";
    AllActions.push_back(BButtonClick);

    FEVRActionBooleanData* AButtonTouch = new FEVRActionBooleanData();
    AButtonTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    AButtonTouch->Name = "a_button_touch";
    AButtonTouch->LeftComponentPath = "/user/hand/left/input/a/touch";
    AButtonTouch->RightComponentPath = "/user/hand/right/input/a/touch";
    AllActions.push_back(AButtonTouch);

    FEVRActionBooleanData* BButtonTouch = new FEVRActionBooleanData();
    BButtonTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    BButtonTouch->Name = "b_button_touch";
    BButtonTouch->LeftComponentPath = "/user/hand/left/input/b/touch";
    BButtonTouch->RightComponentPath = "/user/hand/right/input/b/touch";
    AllActions.push_back(BButtonTouch);

    FEVRActionBooleanData* TriggerTouch = new FEVRActionBooleanData();
    TriggerTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    TriggerTouch->Name = "trigger_touch";
    TriggerTouch->LeftComponentPath = "/user/hand/left/input/trigger/touch";
    TriggerTouch->RightComponentPath = "/user/hand/right/input/trigger/touch";
    AllActions.push_back(TriggerTouch);

    FEVRActionFloatData* TriggerValue = new FEVRActionFloatData();
    TriggerValue->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    TriggerValue->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    TriggerValue->Name = "trigger_value";
    TriggerValue->LeftComponentPath = "/user/hand/left/input/trigger/value";
    TriggerValue->RightComponentPath = "/user/hand/right/input/trigger/value";
    AllActions.push_back(TriggerValue);

    FEVRActionBooleanData* ThumbstickTouch = new FEVRActionBooleanData();
    ThumbstickTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ThumbstickTouch->Name = "thumbstick_touch";
    ThumbstickTouch->LeftComponentPath = "/user/hand/left/input/thumbstick/touch";
    ThumbstickTouch->RightComponentPath = "/user/hand/right/input/thumbstick/touch";
    AllActions.push_back(ThumbstickTouch);

    FEVRActionBooleanData* ThumbstickClick = new FEVRActionBooleanData();
    ThumbstickClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ThumbstickClick->Name = "thumbstick_click";
    ThumbstickClick->LeftComponentPath = "/user/hand/left/input/thumbstick/click";
    ThumbstickClick->RightComponentPath = "/user/hand/right/input/thumbstick/click";
    AllActions.push_back(ThumbstickClick);

    FEVRActionFloatData* ValveThumbstickX = new FEVRActionFloatData();
    ValveThumbstickX->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ValveThumbstickX->Name = "valve_thumbstick_x";
    ValveThumbstickX->LeftComponentPath = "/user/hand/left/input/thumbstick/x";
    ValveThumbstickX->RightComponentPath = "/user/hand/right/input/thumbstick/x";
    AllActions.push_back(ValveThumbstickX);

    FEVRActionFloatData* ValveThumbstickY = new FEVRActionFloatData();
    ValveThumbstickY->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ValveThumbstickY->Name = "valve_thumbstick_y";
    ValveThumbstickY->LeftComponentPath = "/user/hand/left/input/thumbstick/y";
    ValveThumbstickY->RightComponentPath = "/user/hand/right/input/thumbstick/y";
    AllActions.push_back(ValveThumbstickY);

    FEVRActionBooleanData* TrackpadTouch = new FEVRActionBooleanData();
    TrackpadTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    TrackpadTouch->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    TrackpadTouch->Name = "trackpad_touch";
    TrackpadTouch->LeftComponentPath = "/user/hand/left/input/trackpad/touch";
    TrackpadTouch->RightComponentPath = "/user/hand/right/input/trackpad/touch";
    AllActions.push_back(TrackpadTouch);

    FEVRActionFloatData* ValveTrackpadForce = new FEVRActionFloatData();
    ValveTrackpadForce->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ValveTrackpadForce->Name = "valve_trackpad_force";
    ValveTrackpadForce->LeftComponentPath = "/user/hand/left/input/trackpad/force";
    ValveTrackpadForce->RightComponentPath = "/user/hand/right/input/trackpad/force";
    AllActions.push_back(ValveTrackpadForce);

    FEVRActionFloatData* TrackpadX = new FEVRActionFloatData();
    TrackpadX->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    TrackpadX->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    TrackpadX->Name = "trackpad_x";
    TrackpadX->LeftComponentPath = "/user/hand/left/input/trackpad/x";
    TrackpadX->RightComponentPath = "/user/hand/right/input/trackpad/x";
    AllActions.push_back(TrackpadX);

    FEVRActionFloatData* TrackpadY = new FEVRActionFloatData();
    TrackpadY->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    TrackpadX->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    TrackpadY->Name = "trackpad_y";
    TrackpadY->LeftComponentPath = "/user/hand/left/input/trackpad/y";
    TrackpadY->RightComponentPath = "/user/hand/right/input/trackpad/y";
    AllActions.push_back(TrackpadY);

    FEVRActionFloatData* ValveSqueezeValue = new FEVRActionFloatData();
    ValveSqueezeValue->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ValveSqueezeValue->Name = "valve_squeeze_value";
    ValveSqueezeValue->LeftComponentPath = "/user/hand/left/input/squeeze/value";
    ValveSqueezeValue->RightComponentPath = "/user/hand/right/input/squeeze/value";
    AllActions.push_back(ValveSqueezeValue);

    FEVRActionFloatData* ValveSqueezeForce = new FEVRActionFloatData();
    ValveSqueezeForce->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    ValveSqueezeForce->Name = "valve_squeeze_force";
    ValveSqueezeForce->LeftComponentPath = "/user/hand/left/input/squeeze/force";
    ValveSqueezeForce->RightComponentPath = "/user/hand/right/input/squeeze/force";
    AllActions.push_back(ValveSqueezeForce);

    FEVRActionVibrationOutputData* Vibrate = new FEVRActionVibrationOutputData();
    Vibrate->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VALVE_INDEX);
    Vibrate->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    Vibrate->Name = "vibrate_hand";
    Vibrate->LeftComponentPath = "/user/hand/left/output/haptic";
    Vibrate->RightComponentPath = "/user/hand/right/output/haptic";
    AllActions.push_back(Vibrate);

    FEVRActionBooleanData* ViveSqueezeClick = new FEVRActionBooleanData();
    ViveSqueezeClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    ViveSqueezeClick->Name = "vive_squeeze_click";
    ViveSqueezeClick->LeftComponentPath = "/user/hand/left/input/squeeze/click";
    ViveSqueezeClick->RightComponentPath = "/user/hand/right/input/squeeze/click";
    AllActions.push_back(ViveSqueezeClick);

    FEVRActionBooleanData* ViveMenuClick = new FEVRActionBooleanData();
    ViveMenuClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    ViveMenuClick->Name = "vive_menu_click";
    ViveMenuClick->LeftComponentPath = "/user/hand/left/input/menu/click";
    ViveMenuClick->RightComponentPath = "/user/hand/right/input/menu/click";
    AllActions.push_back(ViveMenuClick);

    FEVRActionBooleanData* TrackpadClick = new FEVRActionBooleanData();
    TrackpadClick->WorksWith.push_back(FE_VR_CONTROLLER_TYPE_VIVE);
    TrackpadClick->Name = "trackpad_click";
    TrackpadClick->LeftComponentPath = "/user/hand/left/input/trackpad/click";
    TrackpadClick->RightComponentPath = "/user/hand/right/input/trackpad/click";
    AllActions.push_back(TrackpadClick);
}

FEOpenXRInput::~FEOpenXRInput() {}

void FEOpenXRInput::RegisterActionsAndControllers()
{
    for (size_t i = 0; i < AllActions.size(); i++)
    {
        RegisterActionInOpenXR(*AllActions[i]);
    }

    RegisterAllControllersInOpenXR();
}

void FEOpenXRInput::RegisterAllControllersInOpenXR()
{
    auto ControllerIterator = SupportedControllersBindings.begin();
    while (ControllerIterator != SupportedControllersBindings.end())
    {
        for (size_t i = 0; i < AllActions.size(); i++)
        {
            bool bWorksWith = false;
            for (size_t j = 0; j < AllActions[i]->WorksWith.size(); j++)
            {
                if (AllActions[i]->WorksWith[j] == FE_VR_CONTROLLER_TYPE_ANY || AllActions[i]->WorksWith[j] == ControllerIterator->first)
				{
					bWorksWith = true;
					break;
				}
            }

            if (bWorksWith)
            {
                std::array<XrPath, Side::COUNT> Path;
                FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, AllActions[i]->LeftComponentPath.c_str(), &Path[Side::LEFT]));
                FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, AllActions[i]->RightComponentPath.c_str(), &Path[Side::RIGHT]));
                ControllerIterator->second.Bindings.push_back({ AllActions[i]->ActionHandle, Path[Side::LEFT] });
                ControllerIterator->second.Bindings.push_back({ AllActions[i]->ActionHandle, Path[Side::RIGHT] });
            }
        }

        FEVRControllerActionBindings& CurrentController = ControllerIterator->second;

        XrPath CurrentControllerInteractionProfilePath;
        FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, CurrentController.OpenXRPath.c_str(), &CurrentControllerInteractionProfilePath));

        XrInteractionProfileSuggestedBinding SuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        SuggestedBindings.interactionProfile = CurrentControllerInteractionProfilePath;
        SuggestedBindings.suggestedBindings = CurrentController.Bindings.data();
        SuggestedBindings.countSuggestedBindings = (uint32_t)CurrentController.Bindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &SuggestedBindings));

        ControllerIterator++;
    }
}

void FEOpenXRInput::RegisterActionInOpenXR(FEVRActionData& Action)
{
    XrActionCreateInfo ActionInfo{ XR_TYPE_ACTION_CREATE_INFO };
    ActionInfo.actionType = Action.ActionType;
    strcpy_s(ActionInfo.actionName, Action.Name.c_str());
    strcpy_s(ActionInfo.localizedActionName, Action.Name.c_str());
    ActionInfo.countSubactionPaths = uint32_t(HandSubactionPath.size());
    ActionInfo.subactionPaths = HandSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.ActionSet, &ActionInfo, &Action.ActionHandle));
}

FEOpenXRInput::FEVRActionData* FEOpenXRInput::GetActionDataByName(std::string Name)
{
    for (size_t i = 0; i < AllActions.size(); i++)
    {
        if (AllActions[i]->Name == Name)
        {
            return AllActions[i];
        }
    }

    return nullptr;
}

void FEOpenXRInput::TriggerHapticFeedback(float Amplitude, float Frequency, float Duration, bool bLeftHand)
{
	XrHapticVibration Vibration{ XR_TYPE_HAPTIC_VIBRATION };
	Vibration.amplitude = Amplitude;
	Vibration.frequency = Frequency;
    Vibration.duration = static_cast<XrDuration>(Duration);

    XrHapticActionInfo HapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
    FEVRActionData* HapticAction = GetActionDataByName("vibrate_hand");
    HapticActionInfo.action = HapticAction->ActionHandle;
    HapticActionInfo.subactionPath = bLeftHand ? HandSubactionPath[Side::LEFT] : HandSubactionPath[Side::RIGHT];
	FE_OPENXR_ERROR(xrApplyHapticFeedback(FEOpenXR_CORE.Session, &HapticActionInfo, (XrHapticBaseHeader*)&Vibration));
}

void FEOpenXRInput::InitializeActionsAndControllers()
{
    // Create an action set.
    XrActionSetCreateInfo ActionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy_s(ActionSetInfo.actionSetName, "gameplay");
    strcpy_s(ActionSetInfo.localizedActionSetName, "Gameplay");
    ActionSetInfo.priority = 0;
    FE_OPENXR_ERROR(xrCreateActionSet(FEOpenXR_CORE.OpenXRInstance, &ActionSetInfo, &CurrentInputState.ActionSet));

    // Get the XrPath for the left and right hands - we will use them as subaction paths.
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left", &HandSubactionPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right", &HandSubactionPath[Side::RIGHT]));

    RegisterActionsAndControllers();

    XrActionSpaceCreateInfo ActionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    ActionSpaceInfo.action = GetActionDataByName("controller_pose")->ActionHandle;
    ActionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
    ActionSpaceInfo.subactionPath = HandSubactionPath[Side::LEFT];
    FE_OPENXR_ERROR(xrCreateActionSpace(FEOpenXR_CORE.Session, &ActionSpaceInfo, &CurrentInputState.HandSpace[Side::LEFT]));
    ActionSpaceInfo.subactionPath = HandSubactionPath[Side::RIGHT];
    FE_OPENXR_ERROR(xrCreateActionSpace(FEOpenXR_CORE.Session, &ActionSpaceInfo, &CurrentInputState.HandSpace[Side::RIGHT]));

    XrSessionActionSetsAttachInfo AttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    AttachInfo.countActionSets = 1;
    AttachInfo.actionSets = &CurrentInputState.ActionSet;
    FE_OPENXR_ERROR(xrAttachSessionActionSets(FEOpenXR_CORE.Session, &AttachInfo));
}

void FEOpenXRInput::Init()
{
    if (!FEOpenXR_CORE.bInitializedCorrectly)
        return;

    InitializeActionsAndControllers();
}

void FEOpenXRInput::HandleBooleanAction(FEVRActionBooleanData& CurrentActionBoolean)
{
    for (int Hand = 0; Hand < 2; Hand++)
    {
        XrActionStateGetInfo ActionGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        ActionGetInfo.action = CurrentActionBoolean.ActionHandle;
        ActionGetInfo.subactionPath = (Hand == 0) ? HandSubactionPath[0] : HandSubactionPath[1];

        XrActionStateBoolean ActionValue{ XR_TYPE_ACTION_STATE_BOOLEAN };
        FE_OPENXR_ERROR(xrGetActionStateBoolean(FEOpenXR_CORE.Session, &ActionGetInfo, &ActionValue));

        bool& bPressed = (Hand == 0) ? CurrentActionBoolean.bLeftPressed : CurrentActionBoolean.bRightPressed;

        if (ActionValue.currentState == XR_FALSE && bPressed)
        {
            bPressed = false;
            if (Hand == 0)
            {
                for (size_t i = 0; i < CurrentActionBoolean.LeftDeactivateUserCallBacks.size(); i++)
                {
                    if (CurrentActionBoolean.LeftDeactivateUserCallBacks[i] != nullptr)
                        CurrentActionBoolean.LeftDeactivateUserCallBacks[i]();
                }
            }
            else if (Hand == 1)
            {
                for (size_t i = 0; i < CurrentActionBoolean.RightDeactivateUserCallBacks.size(); i++)
                {
                    if (CurrentActionBoolean.RightDeactivateUserCallBacks[i] != nullptr)
                        CurrentActionBoolean.RightDeactivateUserCallBacks[i]();
                }
            }

#if FE_DEBUG_VR_CONTROLLER_LOGGING
            std::string HandMessage = Hand == 0 ? "Left" : "Right";
            std::string LogMessage = HandMessage + " controller " + CurrentActionBoolean.Name + " was deactivated.";
            LOG.Add(LogMessage, "FE_LOG_OPENXR", FE_LOG_DEBUG);
#endif // FE_DEBUG_VR_CONTROLLER_LOGGING
        }

        if (ActionValue.changedSinceLastSync && ActionValue.currentState == XR_TRUE)
        {
            bPressed = true;
            if (Hand == 0)
            {
                for (size_t i = 0; i < CurrentActionBoolean.LeftActivateUserCallBacks.size(); i++)
                {
                    if (CurrentActionBoolean.LeftActivateUserCallBacks[i] != nullptr)
                        CurrentActionBoolean.LeftActivateUserCallBacks[i]();
                }
            }
            else if (Hand == 1)
            {
                for (size_t i = 0; i < CurrentActionBoolean.RightActivateUserCallBacks.size(); i++)
                {
                    if (CurrentActionBoolean.RightActivateUserCallBacks[i] != nullptr)
                        CurrentActionBoolean.RightActivateUserCallBacks[i]();
                }
            }

#if FE_DEBUG_VR_CONTROLLER_LOGGING
            std::string HandMessage = Hand == 0 ? "Left" : "Right";
            std::string LogMessage = HandMessage + " controller " + CurrentActionBoolean.Name + " was activated.";
            LOG.Add(LogMessage, "FE_LOG_OPENXR", FE_LOG_DEBUG);
#endif // FE_DEBUG_VR_CONTROLLER_LOGGING
        }
    }
}

void FEOpenXRInput::Update()
{
    CurrentInputState.HandActive = { XR_FALSE, XR_FALSE };

    // Sync actions
    const XrActiveActionSet ActiveActionSet{ CurrentInputState.ActionSet, XR_NULL_PATH };
    XrActionsSyncInfo SyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
    SyncInfo.countActiveActionSets = 1;
    SyncInfo.activeActionSets = &ActiveActionSet;
    FE_OPENXR_ERROR(xrSyncActions(FEOpenXR_CORE.Session, &SyncInfo));

    for (size_t i = 0; i < AllActions.size(); i++)
    {
        HandleAction(AllActions[i]);
    }

    UpdateControllerSpaceLocation();
}

void FEOpenXRInput::HandleAction(FEVRActionData* Action)
{
    switch(Action->ActionType)
	{
        case XR_ACTION_TYPE_POSE_INPUT:
		{
			HandlePoseAction(*static_cast<FEVRActionPoseData*>(Action));
			break;
		}
		case XR_ACTION_TYPE_BOOLEAN_INPUT:
		{
			HandleBooleanAction(*static_cast<FEVRActionBooleanData*>(Action));
			break;
		}
		case XR_ACTION_TYPE_FLOAT_INPUT:
		{
			HandleFloatAction(*static_cast<FEVRActionFloatData*>(Action));
			break;
		}
		default:
			break;
	}
}

void FEOpenXRInput::HandlePoseAction(FEVRActionPoseData& CurrentActionPose)
{
    for (int Hand = 0; Hand < 2; Hand++)
    {
        XrActionStateGetInfo ActionGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        ActionGetInfo.action = CurrentActionPose.ActionHandle;
        ActionGetInfo.subactionPath = HandSubactionPath[Hand];

        XrActionStatePose PoseState{ XR_TYPE_ACTION_STATE_POSE };
        FE_OPENXR_ERROR(xrGetActionStatePose(FEOpenXR_CORE.Session, &ActionGetInfo, &PoseState));
        CurrentInputState.HandActive[Hand] = PoseState.isActive;
    }
}

void FEOpenXRInput::HandleFloatAction(FEVRActionFloatData& CurrentActionFloat)
{
    for (int Hand = 0; Hand < 2; Hand++)
    {
        XrActionStateGetInfo ActionGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        ActionGetInfo.action = CurrentActionFloat.ActionHandle;
        ActionGetInfo.subactionPath = HandSubactionPath[Hand];

        XrActionStateFloat ActionValue{ XR_TYPE_ACTION_STATE_FLOAT };
        FE_OPENXR_ERROR(xrGetActionStateFloat(FEOpenXR_CORE.Session, &ActionGetInfo, &ActionValue));

        if (ActionValue.changedSinceLastSync)
        {
#if FE_DEBUG_VR_CONTROLLER_LOGGING
            std::string HandMessage = Hand == 0 ? "Left" : "Right";
            std::string LogMessage = HandMessage + " controller float value of " + CurrentActionFloat.Name + " changed from: " + std::to_string(CurrentActionFloat.CurrentValue) + " to: " + std::to_string(ActionValue.currentState);
            LOG.Add(LogMessage, "FE_LOG_OPENXR", FE_LOG_DEBUG);
#endif // FE_DEBUG_VR_CONTROLLER_LOGGING

            CurrentActionFloat.CurrentValue = ActionValue.currentState;

            if (Hand == 0)
			{
				for (size_t i = 0; i < CurrentActionFloat.LeftChangedValueUserCallBacks.size(); i++)
				{
					if (CurrentActionFloat.LeftChangedValueUserCallBacks[i] != nullptr)
						CurrentActionFloat.LeftChangedValueUserCallBacks[i](CurrentActionFloat.CurrentValue);
				}
			}
			else if (Hand == 1)
			{
				for (size_t i = 0; i < CurrentActionFloat.RightChangedValueUserCallBacks.size(); i++)
				{
					if (CurrentActionFloat.RightChangedValueUserCallBacks[i] != nullptr)
						CurrentActionFloat.RightChangedValueUserCallBacks[i](CurrentActionFloat.CurrentValue);
				}
			}
        }
    }
}

void FEOpenXRInput::UpdateControllerSpaceLocation()
{
    LeftControllerLocation = { XR_TYPE_SPACE_LOCATION };
    RightControllerLocation = { XR_TYPE_SPACE_LOCATION };

    for (auto Hand : { Side::LEFT, Side::RIGHT })
    {
        XrSpaceLocation* CurrentLocation = Hand == 0 ? &LeftControllerLocation : &RightControllerLocation;
        XrResult Result = xrLocateSpace(FEOpenXR_INPUT.CurrentInputState.HandSpace[Hand], FEOpenXR_CORE.ApplicationSpace, FrameState->predictedDisplayTime, CurrentLocation);

        if (!XR_UNQUALIFIED_SUCCESS(Result))
        {
            // Tracking loss is expected when the hand is not active so only log a message
            // if the hand is active.
            if (FEOpenXR_INPUT.CurrentInputState.HandActive[Hand] == XR_TRUE)
            {
                LOG.Add("Unable to locate hand action space in app space!", "FE_LOG_OPENXR");
            }
        }
    }
}

glm::vec3 FEOpenXRInput::GetLeftControllerPosition()
{
    glm::vec3 Result = glm::vec3(0.0f);

    if ((LeftControllerLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
        (LeftControllerLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
    {
        Result = glm::vec3(LeftControllerLocation.pose.position.x, LeftControllerLocation.pose.position.y, LeftControllerLocation.pose.position.z);
    }
    
    return Result;
}

glm::vec3 FEOpenXRInput::GetRightControllerPosition()
{
    glm::vec3 Result = glm::vec3(0.0f);

    if ((RightControllerLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
        (RightControllerLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
    {
        Result = glm::vec3(RightControllerLocation.pose.position.x, RightControllerLocation.pose.position.y, RightControllerLocation.pose.position.z);
    }

    return Result;
}

glm::quat FEOpenXRInput::GetLeftControllerOrientation()
{
    glm::quat Result = glm::quat();

    if ((LeftControllerLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
        (LeftControllerLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
    {
        Result = glm::quat(LeftControllerLocation.pose.orientation.w, LeftControllerLocation.pose.orientation.x, LeftControllerLocation.pose.orientation.y, LeftControllerLocation.pose.orientation.z);
    }

    return Result;
}

glm::quat FEOpenXRInput::GetRightControllerOrientation()
{
    glm::quat Result = glm::quat();

    if ((RightControllerLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
        (RightControllerLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
    {
        Result = glm::quat(RightControllerLocation.pose.orientation.w, RightControllerLocation.pose.orientation.x, RightControllerLocation.pose.orientation.y, RightControllerLocation.pose.orientation.z);
    }

    return Result;
}

void FEOpenXRInput::SetLeftTriggerPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("trigger_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("trigger_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftTriggerReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("trigger_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("trigger_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftTriggerTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("trigger_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftTriggerTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("trigger_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("trigger_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("trigger_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftAButtonPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("a_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("a_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftAButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("a_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("a_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

// FIX ME! Introduce simpler function to get controller type.
std::string FEOpenXRInput::CurrentlyActiveInteractionProfile(bool bLeftController)
{
    XrPath TopLevelUserPath;
    std::string Hand = bLeftController ? "left" : "right";
    xrStringToPath(FEOpenXR_CORE.OpenXRInstance, ("/user/hand/" + Hand).c_str(), &TopLevelUserPath);

    XrInteractionProfileState InteractionProfileState{ XR_TYPE_INTERACTION_PROFILE_STATE };
    xrGetCurrentInteractionProfile(FEOpenXR_CORE.Session, TopLevelUserPath, &InteractionProfileState);

    char InteractionProfileStr[XR_MAX_PATH_LENGTH];
    uint32_t StrLength = 0;
    xrPathToString(FEOpenXR_CORE.OpenXRInstance, InteractionProfileState.interactionProfile, XR_MAX_PATH_LENGTH, &StrLength, InteractionProfileStr);

    if (!StrLength)
        return "";

    return std::string(InteractionProfileStr);
}

void FEOpenXRInput::SetLeftBButtonPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightBButtonPressCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftBButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightBButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_click");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftAButtonTouchActivateCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("a_button_touch");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}
void FEOpenXRInput::SetLeftAButtonTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("a_button_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("a_button_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("a_button_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftBButtonTouchActivateCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_touch");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftBButtonTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("b_button_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightBButtonTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("b_button_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightBButtonTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
    FEVRActionData* Action = GetActionDataByName("b_button_touch");
    reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftThumbstickPressCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightThumbstickPressCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftThumbstickReleaseCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightThumbstickReleaseCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftThumbstickTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftThumbstickTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightThumbstickTouchDeactivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightThumbstickTouchActivateCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("thumbstick_touch");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftViveSqueezePressCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("vive_squeeze_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightViveSqueezePressCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("vive_squeeze_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightActivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetLeftViveSqueezeReleaseCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("vive_squeeze_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->LeftDeactivateUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightViveSqueezeReleaseCallBack(std::function<void()> UserCallBack)
{
	FEVRActionData* Action = GetActionDataByName("vive_squeeze_click");
	reinterpret_cast<FEVRActionBooleanData*>(Action)->RightDeactivateUserCallBacks.push_back(UserCallBack);
}