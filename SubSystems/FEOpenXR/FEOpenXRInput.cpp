#include "FEOpenXRInput.h"

using namespace FocalEngine;

FEOpenXRInput* FEOpenXRInput::Instance = nullptr;
FEOpenXRInput::FEOpenXRInput() {}
FEOpenXRInput::~FEOpenXRInput() {}

void FEOpenXRInput::CreateActions()
{
    // Create an input action for grabbing objects with the left and right hands.
    XrActionCreateInfo ActionInfo{ XR_TYPE_ACTION_CREATE_INFO };
    ActionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy_s(ActionInfo.actionName, "grab_object");
    strcpy_s(ActionInfo.localizedActionName, "Grab Object");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.grabAction));

    // Create an input action getting the left and right hand poses.
    ActionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    strcpy_s(ActionInfo.actionName, "hand_pose");
    strcpy_s(ActionInfo.localizedActionName, "Hand Pose");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.poseAction));

    // Create output actions for vibrating the left and right controller.
    ActionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
    strcpy_s(ActionInfo.actionName, "vibrate_hand");
    strcpy_s(ActionInfo.localizedActionName, "Vibrate Hand");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.HapticAction));

    // Create input actions for quitting the session using the left and right controller.
    // Since it doesn't matter which hand did this, we do not specify subaction paths for it.
    // We will just suggest bindings for both hands, where possible.
    ActionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy_s(ActionInfo.actionName, "quit_session");
    strcpy_s(ActionInfo.localizedActionName, "Quit Session");
    ActionInfo.countSubactionPaths = 0;
    ActionInfo.subactionPaths = nullptr;
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.BButtonClickAction));

    // Create an input action for triggering a click with the left and right hands.
    ActionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy_s(ActionInfo.actionName, "trigger_click");
    strcpy_s(ActionInfo.localizedActionName, "Trigger Click");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.triggerClickAction));

    // /input/a/click action
    ActionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy_s(ActionInfo.actionName, "a_button_click");
    strcpy_s(ActionInfo.localizedActionName, "A Button Click");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.AButtonClickAction));

    // Squeeze button click
    ActionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy_s(ActionInfo.actionName, "Squeeze_button_click");
    strcpy_s(ActionInfo.localizedActionName, "Squeeze Button Click");
    ActionInfo.countSubactionPaths = uint32_t(CurrentInputState.handSubactionPath.size());
    ActionInfo.subactionPaths = CurrentInputState.handSubactionPath.data();
    FE_OPENXR_ERROR(xrCreateAction(CurrentInputState.actionSet, &ActionInfo, &CurrentInputState.SqueezeButtonClick));
}

void FEOpenXRInput::InitializeActions()
{
    // Create an action set.
    {
        XrActionSetCreateInfo ActionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
        strcpy_s(ActionSetInfo.actionSetName, "gameplay");
        strcpy_s(ActionSetInfo.localizedActionSetName, "Gameplay");
        ActionSetInfo.priority = 0;
        FE_OPENXR_ERROR(xrCreateActionSet(FEOpenXR_CORE.OpenXRInstance, &ActionSetInfo, &CurrentInputState.actionSet));
    }

    // Get the XrPath for the left and right hands - we will use them as subaction paths.
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left", &CurrentInputState.handSubactionPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right", &CurrentInputState.handSubactionPath[Side::RIGHT]));

    CreateActions();

    std::array<XrPath, Side::COUNT> selectPath;
    std::array<XrPath, Side::COUNT> squeezeValuePath;
    std::array<XrPath, Side::COUNT> squeezeForcePath;
    std::array<XrPath, Side::COUNT> squeezeClickPath;
    std::array<XrPath, Side::COUNT> posePath;
    std::array<XrPath, Side::COUNT> hapticPath;
    std::array<XrPath, Side::COUNT> menuClickPath;
    std::array<XrPath, Side::COUNT> bClickPath;
    std::array<XrPath, Side::COUNT> triggerValuePath;
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/select/click", &selectPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/select/click", &selectPath[Side::RIGHT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[Side::RIGHT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/squeeze/force", &squeezeForcePath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/squeeze/force", &squeezeForcePath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.grabAction, squeezeForcePath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.grabAction, squeezeForcePath[Side::RIGHT] });

    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[Side::RIGHT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/grip/pose", &posePath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/grip/pose", &posePath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.poseAction, posePath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.poseAction, posePath[Side::RIGHT] });

    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/output/haptic", &hapticPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/output/haptic", &hapticPath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.HapticAction, hapticPath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.HapticAction, hapticPath[Side::RIGHT] });


    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/menu/click", &menuClickPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/menu/click", &menuClickPath[Side::RIGHT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/b/click", &bClickPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/b/click", &bClickPath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.BButtonClickAction, bClickPath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.BButtonClickAction, bClickPath[Side::RIGHT] });

    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/trigger/value", &triggerValuePath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/trigger/value", &triggerValuePath[Side::RIGHT]));


    // Define the trigger click paths
    std::array<XrPath, Side::COUNT> triggerClickPath;
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/trigger/click", &triggerClickPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/trigger/click", &triggerClickPath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.triggerClickAction, triggerClickPath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.triggerClickAction, triggerClickPath[Side::RIGHT] });

    std::array<XrPath, Side::COUNT> AButtonClickPath;
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/left/input/a/click", &AButtonClickPath[Side::LEFT]));
    FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/user/hand/right/input/a/click", &AButtonClickPath[Side::RIGHT]));
    ValveIndexBindings.push_back({ CurrentInputState.AButtonClickAction, AButtonClickPath[Side::LEFT] });
    ValveIndexBindings.push_back({ CurrentInputState.AButtonClickAction, AButtonClickPath[Side::RIGHT] });

    // Suggest bindings for KHR Simple.
    {
        XrPath khrSimpleInteractionProfilePath;
        FE_OPENXR_ERROR(
            xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/interaction_profiles/khr/simple_controller", &khrSimpleInteractionProfilePath));
        std::vector<XrActionSuggestedBinding> bindings{{// Fall back to a click input for the grab action.
            { CurrentInputState.grabAction, selectPath[Side::LEFT] },
            { CurrentInputState.grabAction, selectPath[Side::RIGHT] },
            { CurrentInputState.poseAction, posePath[Side::LEFT] },
            { CurrentInputState.poseAction, posePath[Side::RIGHT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::LEFT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::RIGHT] },
            { CurrentInputState.HapticAction, hapticPath[Side::LEFT] },
            { CurrentInputState.HapticAction, hapticPath[Side::RIGHT] }}};
        XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &suggestedBindings));
    }
    // Suggest bindings for the Oculus Touch.
    {
        XrPath oculusTouchInteractionProfilePath;
        FE_OPENXR_ERROR(
            xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/interaction_profiles/oculus/touch_controller", &oculusTouchInteractionProfilePath));
        std::vector<XrActionSuggestedBinding> bindings{{{CurrentInputState.grabAction, squeezeValuePath[Side::LEFT]},
            { CurrentInputState.grabAction, squeezeValuePath[Side::RIGHT] },
            { CurrentInputState.poseAction, posePath[Side::LEFT] },
            { CurrentInputState.poseAction, posePath[Side::RIGHT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::LEFT] },
            { CurrentInputState.HapticAction, hapticPath[Side::LEFT] },
            { CurrentInputState.HapticAction, hapticPath[Side::RIGHT] }}};
        XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &suggestedBindings));
    }
    // Suggest bindings for the Vive Controller.
    {
        XrPath viveControllerInteractionProfilePath;
        FE_OPENXR_ERROR(
            xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/interaction_profiles/htc/vive_controller", &viveControllerInteractionProfilePath));
        std::vector<XrActionSuggestedBinding> bindings{{
            //{ CurrentInputState.grabAction, triggerValuePath[Side::LEFT] },
            //{ CurrentInputState.grabAction, triggerValuePath[Side::RIGHT] },
            { CurrentInputState.triggerClickAction, triggerClickPath[Side::LEFT] },
            { CurrentInputState.triggerClickAction, triggerClickPath[Side::RIGHT] },
            { CurrentInputState.AButtonClickAction, squeezeClickPath[Side::LEFT] },
            { CurrentInputState.AButtonClickAction, squeezeClickPath[Side::RIGHT] },


            { CurrentInputState.poseAction, posePath[Side::LEFT] },
            { CurrentInputState.poseAction, posePath[Side::RIGHT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::LEFT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::RIGHT] },
            { CurrentInputState.HapticAction, hapticPath[Side::LEFT] },
            { CurrentInputState.HapticAction, hapticPath[Side::RIGHT] }}};
        XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBindings.interactionProfile = viveControllerInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &suggestedBindings));
    }

    // Suggest bindings for the Valve Index Controller.
    {
        XrPath indexControllerInteractionProfilePath;
        FE_OPENXR_ERROR(
            xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/interaction_profiles/valve/index_controller", &indexControllerInteractionProfilePath));
        //std::vector<XrActionSuggestedBinding> bindings{{
            //{ CurrentInputState.triggerClickAction, triggerClickPath[Side::LEFT] },
            //{ CurrentInputState.triggerClickAction, triggerClickPath[Side::RIGHT] },
            //{ CurrentInputState.triggerClickAction, AButtonClickPath[Side::LEFT] },
            //{ CurrentInputState.triggerClickAction, AButtonClickPath[Side::RIGHT] },
            //{ CurrentInputState.grabAction, squeezeForcePath[Side::LEFT] },
            //{ CurrentInputState.grabAction, squeezeForcePath[Side::RIGHT] },
            //{ CurrentInputState.poseAction, posePath[Side::LEFT] },
            //{ CurrentInputState.poseAction, posePath[Side::RIGHT] },
            //{ CurrentInputState.BButtonClickAction, bClickPath[Side::LEFT] },
            //{ CurrentInputState.BButtonClickAction, bClickPath[Side::RIGHT] },
            //{ CurrentInputState.HapticAction, hapticPath[Side::LEFT] },
            //{ CurrentInputState.HapticAction, hapticPath[Side::RIGHT] }}};
        XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBindings.interactionProfile = indexControllerInteractionProfilePath;
        suggestedBindings.suggestedBindings = /*bindings*/ValveIndexBindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)/*bindings*/ValveIndexBindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &suggestedBindings));
    }

    // Suggest bindings for the Microsoft Mixed Reality Motion Controller.
    {
        XrPath microsoftMixedRealityInteractionProfilePath;
        FE_OPENXR_ERROR(xrStringToPath(FEOpenXR_CORE.OpenXRInstance, "/interaction_profiles/microsoft/motion_controller",
            &microsoftMixedRealityInteractionProfilePath));
        std::vector<XrActionSuggestedBinding> bindings{{
            { CurrentInputState.grabAction, squeezeClickPath[Side::LEFT] },
            { CurrentInputState.grabAction, squeezeClickPath[Side::RIGHT] },
            { CurrentInputState.poseAction, posePath[Side::LEFT] },
            { CurrentInputState.poseAction, posePath[Side::RIGHT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::LEFT] },
            { CurrentInputState.BButtonClickAction, menuClickPath[Side::RIGHT] },
            { CurrentInputState.HapticAction, hapticPath[Side::LEFT] },
            { CurrentInputState.HapticAction, hapticPath[Side::RIGHT] }}};
        XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBindings.interactionProfile = microsoftMixedRealityInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        FE_OPENXR_ERROR(xrSuggestInteractionProfileBindings(FEOpenXR_CORE.OpenXRInstance, &suggestedBindings));
    }
    XrActionSpaceCreateInfo ActionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    ActionSpaceInfo.action = CurrentInputState.poseAction;
    ActionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
    ActionSpaceInfo.subactionPath = CurrentInputState.handSubactionPath[Side::LEFT];
    FE_OPENXR_ERROR(xrCreateActionSpace(FEOpenXR_CORE.Session, &ActionSpaceInfo, &CurrentInputState.handSpace[Side::LEFT]));
    ActionSpaceInfo.subactionPath = CurrentInputState.handSubactionPath[Side::RIGHT];
    FE_OPENXR_ERROR(xrCreateActionSpace(FEOpenXR_CORE.Session, &ActionSpaceInfo, &CurrentInputState.handSpace[Side::RIGHT]));

    XrSessionActionSetsAttachInfo AttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    AttachInfo.countActionSets = 1;
    AttachInfo.actionSets = &CurrentInputState.actionSet;
    FE_OPENXR_ERROR(xrAttachSessionActionSets(FEOpenXR_CORE.Session, &AttachInfo));
}

void FEOpenXRInput::Init()
{
    if (!FEOpenXR_CORE.bInitializedCorrectly)
        return;

    InitializeActions();
}

void FEOpenXRInput::Update()
{
    CurrentInputState.handActive = { XR_FALSE, XR_FALSE };

    // Sync actions
    const XrActiveActionSet activeActionSet{ CurrentInputState.actionSet, XR_NULL_PATH };
    XrActionsSyncInfo syncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;
    FE_OPENXR_ERROR(xrSyncActions(FEOpenXR_CORE.Session, &syncInfo));

    // Get pose and grab action state and start haptic vibrate when hand is 90% squeezed.
    for (auto hand : { Side::LEFT, Side::RIGHT })
    {
        XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        getInfo.action = CurrentInputState.grabAction;
        getInfo.subactionPath = CurrentInputState.handSubactionPath[hand];

        XrActionStateFloat grabValue{ XR_TYPE_ACTION_STATE_FLOAT };
        FE_OPENXR_ERROR(xrGetActionStateFloat(FEOpenXR_CORE.Session, &getInfo, &grabValue));
        if (grabValue.isActive == XR_TRUE)
        {
            // Scale the rendered hand by 1.0f (open) to 0.5f (fully squeezed).
            CurrentInputState.handScale[hand] = 1.0f - 0.5f * grabValue.currentState;
            if (grabValue.currentState > 0.9f) {
                XrHapticVibration vibration{ XR_TYPE_HAPTIC_VIBRATION };
                vibration.amplitude = 0.5;
                vibration.duration = XR_MIN_HAPTIC_DURATION;
                vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

                XrHapticActionInfo hapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
                hapticActionInfo.action = CurrentInputState.HapticAction;
                hapticActionInfo.subactionPath = CurrentInputState.handSubactionPath[hand];
                FE_OPENXR_ERROR(xrApplyHapticFeedback(FEOpenXR_CORE.Session, &hapticActionInfo, (XrHapticBaseHeader*)&vibration));
            }
        }

        getInfo.action = CurrentInputState.poseAction;
        XrActionStatePose poseState{ XR_TYPE_ACTION_STATE_POSE };
        FE_OPENXR_ERROR(xrGetActionStatePose(FEOpenXR_CORE.Session, &getInfo, &poseState));
        CurrentInputState.handActive[hand] = poseState.isActive;



        // Trigger/click
        XrActionStateGetInfo triggerGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        triggerGetInfo.action = CurrentInputState.triggerClickAction;
        triggerGetInfo.subactionPath = CurrentInputState.handSubactionPath[hand];

        XrActionStateBoolean triggerValue{ XR_TYPE_ACTION_STATE_BOOLEAN };
        FE_OPENXR_ERROR(xrGetActionStateBoolean(FEOpenXR_CORE.Session, &triggerGetInfo, &triggerValue));
        
        if (triggerValue.currentState == 0 && hand == 0 && bLeftTriggerPressed)
        {
            bLeftTriggerPressed = false;
            ActivateLeftTriggerReleaseUserCallBacks();
        }

        if (triggerValue.currentState == 0 && hand == 1 && bRightTriggerPressed)
        {
            bRightTriggerPressed = false;
            ActivateRightTriggerReleaseUserCallBacks();
        }
            
        if (triggerValue.changedSinceLastSync && triggerValue.currentState)
        {
            if (hand == 0)
            {
                bLeftTriggerPressed = true;
                ActivateLeftTriggerPressUserCallBacks();
            }
            else if (hand == 1)
            {
                bRightTriggerPressed = true;
                ActivateRightTriggerPressUserCallBacks();
            }
        }

        // A Button Click
        XrActionStateGetInfo ActionGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
        ActionGetInfo.action = CurrentInputState.AButtonClickAction;
        ActionGetInfo.subactionPath = CurrentInputState.handSubactionPath[hand];

        XrActionStateBoolean ActionValue{ XR_TYPE_ACTION_STATE_BOOLEAN };
        FE_OPENXR_ERROR(xrGetActionStateBoolean(FEOpenXR_CORE.Session, &ActionGetInfo, &ActionValue));

        if (ActionValue.currentState == 0 && hand == 0 && bLeftAButtonPressed)
        {
            bLeftAButtonPressed = false;
            ActivateLeftAButtonReleaseUserCallBacks();
        }

        if (ActionValue.currentState == 0 && hand == 1 && bRightAButtonPressed)
        {
            bRightAButtonPressed = false;
            ActivateRightAButtonReleaseUserCallBacks();
        }

        if (ActionValue.changedSinceLastSync && ActionValue.currentState)
        {
            if (hand == 0)
            {
                bLeftAButtonPressed = true;
                ActivateLeftAButtonPressUserCallBacks();
            }
            else if (hand == 1)
            {
                bRightAButtonPressed = true;
                ActivateRightAButtonPressUserCallBacks();
            }
        }
    }

    // There were no subaction paths specified for the quit action, because we don't care which hand did it.
    XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO, nullptr, CurrentInputState.BButtonClickAction, XR_NULL_PATH };
    XrActionStateBoolean quitValue{ XR_TYPE_ACTION_STATE_BOOLEAN };
    FE_OPENXR_ERROR(xrGetActionStateBoolean(FEOpenXR_CORE.Session, &getInfo, &quitValue));
    if ((quitValue.isActive == XR_TRUE) && (quitValue.changedSinceLastSync == XR_TRUE) && (quitValue.currentState == XR_TRUE))
    {
        FE_OPENXR_ERROR(xrRequestExitSession(FEOpenXR_CORE.Session));
    }

    UpdateControllerSpaceLocation();
}

void FEOpenXRInput::UpdateControllerSpaceLocation()
{
    LeftControllerLocation = { XR_TYPE_SPACE_LOCATION };
    RightControllerLocation = { XR_TYPE_SPACE_LOCATION };

    for (auto Hand : { Side::LEFT, Side::RIGHT })
    {
        XrSpaceLocation* CurrentLocation = Hand == 0 ? &LeftControllerLocation : &RightControllerLocation;
        //XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
        XrResult res = xrLocateSpace(FEOpenXR_INPUT.CurrentInputState.handSpace[Hand], FEOpenXR_CORE.ApplicationSpace, FrameState->predictedDisplayTime, CurrentLocation);

        if (XR_UNQUALIFIED_SUCCESS(res))
        {
            if ((CurrentLocation->locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (CurrentLocation->locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
            {
                //float scale = 0.1f * FEOpenXR_INPUT.CurrentInputState.handScale[Hand];
                //cubes.push_back(Cube{ spaceLocation.pose, {scale, scale, scale} });
            }
        }
        else
        {
            // Tracking loss is expected when the hand is not active so only log a message
            // if the hand is active.
            if (FEOpenXR_INPUT.CurrentInputState.handActive[Hand] == XR_TRUE)
            {
                LOG.Add("Unable to locate hand action space in app space!", " OpenXR");
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
        //float scale = 0.1f * FEOpenXR_INPUT.CurrentInputState.handScale[hand];
        //cubes.push_back(Cube{ spaceLocation.pose, {scale, scale, scale} });
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
        //float scale = 0.1f * FEOpenXR_INPUT.CurrentInputState.handScale[hand];
        //cubes.push_back(Cube{ spaceLocation.pose, {scale, scale, scale} });
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
    if (UserCallBack != nullptr)
        LeftTriggerPressUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerPressCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        RightTriggerPressUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::ActivateLeftTriggerPressUserCallBacks()
{
    for (size_t i = 0; i < LeftTriggerPressUserCallBacks.size(); i++)
    {
        if (LeftTriggerPressUserCallBacks[i] != nullptr)
            LeftTriggerPressUserCallBacks[i]();
    }
}

void FEOpenXRInput::ActivateRightTriggerPressUserCallBacks()
{
    for (size_t i = 0; i < RightTriggerPressUserCallBacks.size(); i++)
    {
        if (RightTriggerPressUserCallBacks[i] != nullptr)
            RightTriggerPressUserCallBacks[i]();
    }
}

void FEOpenXRInput::SetLeftTriggerReleaseCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        LeftTriggerReleaseUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightTriggerReleaseCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        RightTriggerReleaseUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::ActivateLeftTriggerReleaseUserCallBacks()
{
    for (size_t i = 0; i < LeftTriggerReleaseUserCallBacks.size(); i++)
    {
        if (LeftTriggerReleaseUserCallBacks[i] != nullptr)
            LeftTriggerReleaseUserCallBacks[i]();
    }
}

void FEOpenXRInput::ActivateRightTriggerReleaseUserCallBacks()
{
    for (size_t i = 0; i < RightTriggerReleaseUserCallBacks.size(); i++)
    {
        if (RightTriggerReleaseUserCallBacks[i] != nullptr)
            RightTriggerReleaseUserCallBacks[i]();
    }
}

void FEOpenXRInput::SetLeftAButtonPressCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        LeftAButtonPressUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonPressCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        RightAButtonPressUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::ActivateLeftAButtonPressUserCallBacks()
{
    for (size_t i = 0; i < LeftAButtonPressUserCallBacks.size(); i++)
    {
        if (LeftAButtonPressUserCallBacks[i] != nullptr)
            LeftAButtonPressUserCallBacks[i]();
    }
}

void FEOpenXRInput::ActivateRightAButtonPressUserCallBacks()
{
    for (size_t i = 0; i < RightAButtonPressUserCallBacks.size(); i++)
    {
        if (RightAButtonPressUserCallBacks[i] != nullptr)
            RightAButtonPressUserCallBacks[i]();
    }
}

void FEOpenXRInput::SetLeftAButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        LeftAButtonReleaseUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::SetRightAButtonReleaseCallBack(std::function<void()> UserCallBack)
{
    if (UserCallBack != nullptr)
        RightAButtonReleaseUserCallBacks.push_back(UserCallBack);
}

void FEOpenXRInput::ActivateLeftAButtonReleaseUserCallBacks()
{
    for (size_t i = 0; i < LeftAButtonReleaseUserCallBacks.size(); i++)
    {
        if (LeftAButtonReleaseUserCallBacks[i] != nullptr)
            LeftAButtonReleaseUserCallBacks[i]();
    }
}

void FEOpenXRInput::ActivateRightAButtonReleaseUserCallBacks()
{
    for (size_t i = 0; i < RightAButtonReleaseUserCallBacks.size(); i++)
    {
        if (RightAButtonReleaseUserCallBacks[i] != nullptr)
            RightAButtonReleaseUserCallBacks[i]();
    }
}

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