#include "FEOpenXR.h"
using namespace FocalEngine;

#include "../FEngine.h"

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetOpenXR()
{
	return FEOpenXR::GetInstancePointer();
}
#endif

FEOpenXR::FEOpenXR() {}
FEOpenXR::~FEOpenXR()
{
	if (FEOpenXR_INPUT.CurrentInputState.ActionSet != XR_NULL_HANDLE)
	{
		for (auto Hand : { Side::LEFT, Side::RIGHT })
			xrDestroySpace(FEOpenXR_INPUT.CurrentInputState.HandSpace[Hand]);
		
		xrDestroyActionSet(FEOpenXR_INPUT.CurrentInputState.ActionSet);
	}

	for (int i = 0; i < FEOpenXR_RENDERING.SwapChains.size(); i++)
	{
		xrDestroySwapchain(FEOpenXR_RENDERING.SwapChains[i]);
	}

	if (FEOpenXR_CORE.ApplicationSpace != XR_NULL_HANDLE)
		xrDestroySpace(FEOpenXR_CORE.ApplicationSpace);

	if (FEOpenXR_CORE.Session != XR_NULL_HANDLE)
		xrDestroySession(FEOpenXR_CORE.Session);

	if (FEOpenXR_CORE.OpenXRInstance != XR_NULL_HANDLE)
		xrDestroyInstance(FEOpenXR_CORE.OpenXRInstance);
}

bool FEOpenXR::Init()
{
	FEOpenXR_CORE.Init();
	FEOpenXR_INPUT.Init();
	FEOpenXR_INPUT.AddControllerStateChangeCallback(OnControllerConnectionChanges);
	FEOpenXR_RENDERING.Init();

	FEOpenXR_INPUT.FrameState = &FEOpenXR_RENDERING.FrameState;

	return FEOpenXR_CORE.bInitializedCorrectly;
}

void FEOpenXR::PollEvents()
{
	bool bSessionStopping = false;

	// --- Handle runtime Events
	// we do this before xrWaitFrame() so we can go idle or
	// break out of the main render loop as early as possible and don't have to
	// uselessly render or submit one. Calling xrWaitFrame commits you to
	// calling xrBeginFrame eventually.
	XrEventDataBuffer RuntimeEvent;
	RuntimeEvent.type = XR_TYPE_EVENT_DATA_BUFFER;
	RuntimeEvent.next = nullptr;

	XrResult PollResult = xrPollEvent(FEOpenXR_CORE.OpenXRInstance, &RuntimeEvent);
	while (PollResult == XR_SUCCESS)
	{
		switch (RuntimeEvent.type)
		{
		case XR_TYPE_EVENT_DATA_EVENTS_LOST:
		{
			XrEventDataEventsLost* Event = (XrEventDataEventsLost*)&RuntimeEvent;

			LOG.Add("EVENT: " + std::to_string(Event->lostEventCount) + " events data lost!", "FE_LOG_OPENXR");
			// do we care if the runtime loses events?
			break;
		}
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* Event = (XrEventDataInstanceLossPending*)&RuntimeEvent;
			LOG.Add("EVENT: instance loss pending at " + std::to_string(Event->lossTime) + "! Destroying instance.", "FE_LOG_OPENXR");
			bSessionStopping = true;
			break;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* Event = (XrEventDataSessionStateChanged*)&RuntimeEvent;
			//printf("EVENT: session state changed from %d to %d\n", SessionState, event->state);

			FEOpenXR_CORE.SessionState = Event->state;

			switch (FEOpenXR_CORE.SessionState)
			{
				case XR_SESSION_STATE_READY:
				{
					XrSessionBeginInfo SessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
					SessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
					FE_OPENXR_ERROR(xrBeginSession(FEOpenXR_CORE.Session, &SessionBeginInfo));
					//m_sessionRunning = true;
					break;
				}
				case XR_SESSION_STATE_STOPPING:
				{
					//CHECK(m_session != XR_NULL_HANDLE);
					//m_sessionRunning = false;
					//CHECK_XRCMD(xrEndSession(m_session))
						break;
				}
				case XR_SESSION_STATE_EXITING:
				{
					//*exitRenderLoop = true;
					// Do not attempt to restart because user closed this session.
					//*requestRestart = false;
					break;
				}
				case XR_SESSION_STATE_LOSS_PENDING:
				{
					//*exitRenderLoop = true;
					// Poll for a new instance.
					//*requestRestart = true;
					break;
				}
				default:
					break;
			}
		}
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
		{
			//printf("EVENT: reference space change pending!\n");
			XrEventDataReferenceSpaceChangePending* Event = (XrEventDataReferenceSpaceChangePending*)&RuntimeEvent;
			(void)Event;
			// TODO: do something
			break;
		}
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			//printf("EVENT: interaction profile changed!\n");
			XrEventDataInteractionProfileChanged* Event = (XrEventDataInteractionProfileChanged*)&RuntimeEvent;
			(void)Event;

			XrInteractionProfileState State;
			State.type = XR_TYPE_INTERACTION_PROFILE_STATE;

			/*for (int i = 0; i < 2; i++)
			{
				XrResult res = xrGetCurrentInteractionProfile(session, hand_paths[i], &state);
				if (!xr_result(instance, res, "Failed to get interaction profile for %d", i))
					continue;

				XrPath prof = state.interactionProfile;

				uint32_t strl;
				char profile_str[XR_MAX_PATH_LENGTH];
				res = xrPathToString(instance, prof, XR_MAX_PATH_LENGTH, &strl, profile_str);
				if (!xr_result(instance, res, "Failed to get interaction profile path str for %s",
					h_p_str(i)))
					continue;

				printf("Event: Interaction profile changed for %s: %s\n", h_p_str(i), profile_str);
			}*/

			// TODO: do something
			break;
		}

		case XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR:
		{
			//printf("EVENT: visibility mask changed!!\n");
			XrEventDataVisibilityMaskChangedKHR* Event = (XrEventDataVisibilityMaskChangedKHR*)&RuntimeEvent;
			(void)Event;
			// this event is from an extension
			break;
		}
		case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT:
		{
			//printf("EVENT: perf settings!\n");
			XrEventDataPerfSettingsEXT* Event = (XrEventDataPerfSettingsEXT*)&RuntimeEvent;
			(void)Event;
			// this event is from an extension
			break;
		}

		default:;//printf("Unhandled event type %d\n", runtime_event.type);
		}

		RuntimeEvent.type = XR_TYPE_EVENT_DATA_BUFFER;
		PollResult = xrPollEvent(FEOpenXR_CORE.OpenXRInstance, &RuntimeEvent);
	}

	if (PollResult == XR_EVENT_UNAVAILABLE)
	{
		// processed all events in the queue
	}
	else
	{
		//printf("Failed to poll events!\n");
		//break;
	}

	if (bSessionStopping)
	{
		//printf("Quitting main render loop\n");
		return;
	}
}

void FEOpenXR::Update()
{
	PollEvents();
	FEOpenXR_RENDERING.Update();
	FEOpenXR_INPUT.Update();

	// Force HDR output in VR
	if (VRHeadsetEntity != nullptr)
	{
		FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(VRHeadsetEntity);
		if (CameraData != nullptr)
			CameraData->bTemporaryForceHDROutput = true; 
	}

	SceneNodesUpdate();
}

glm::vec2 FEOpenXR::EyeResolution()
{
	return FEOpenXR_RENDERING.EyeResolution();
}

void FEOpenXR::SceneNodesUpdate()
{
	// FIXME: Temporary solution, only supports one scene.
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	if (ActiveScenes.empty())
		return;

	FEScene* CurrentScene = ActiveScenes[0];
	if (CurrentScene == nullptr)
		return;

	OpenXR_MANAGER.TryToAddVRRigToScene(CurrentScene);

	if (CurrentScene->GetEntity(VRRigEntity->GetObjectID()) == nullptr || CurrentScene->GetEntity(VRHeadsetEntity->GetObjectID()) == nullptr)
		return;

	if (FEOpenXR_INPUT.IsLeftControllerConnectedAndTracked() && LeftController != nullptr)
	{
		FETransformComponent& LeftControllerTransform = LeftController->GetComponent<FETransformComponent>();
		LeftControllerTransform.SetPosition(FEOpenXR_INPUT.GetLeftControllerPosition());
		LeftControllerTransform.SetQuaternion(FEOpenXR_INPUT.GetLeftControllerOrientation());
	}

	if (FEOpenXR_INPUT.IsRightControllerConnectedAndTracked() && RightController != nullptr)
	{
		FETransformComponent& RightControllerTransform = RightController->GetComponent<FETransformComponent>();
		RightControllerTransform.SetPosition(FEOpenXR_INPUT.GetRightControllerPosition());
		RightControllerTransform.SetQuaternion(FEOpenXR_INPUT.GetRightControllerOrientation());
	}
}

FEEntity* FEOpenXR::GetVRRigEntity() const
{
	return VRRigEntity;
}

FEEntity* FEOpenXR::GetVRHeadsetEntity() const
{
	return VRHeadsetEntity;
}

bool FEOpenXR::SetCustomVRControllerModel(FEGameModel* CustomGameModel, bool bLeftController)
{
	FEEntity* ControllerEntity = bLeftController ? LeftController : RightController;
	std::string ControllerName = bLeftController ? "Left" : "Right";

	if (CustomGameModel == nullptr)
	{
		if (ControllerEntity == nullptr)
		{
			LOG.Add(ControllerName + " controller is null in function SetCustomVRControllerModel()", "FE_LOG_OPENXR", FE_LOG_ERROR);
			return false;
		}

		if (!ControllerEntity->HasComponent<FEGameModelComponent>())
		{
			LOG.Add(ControllerName + " controller does not have FEGameModelComponent in function SetCustomVRControllerModel()", "FE_LOG_OPENXR", FE_LOG_ERROR);
			return false;
		}

		ControllerEntity->GetComponent<FEGameModelComponent>().SetGameModel(RESOURCE_MANAGER.GetGameModel("504029555848336725615C49"));
		FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
		ControllerTransform.SetScale(StandardControllerScale);

		return true;
	}

	if (CustomGameModel != nullptr && ControllerEntity != nullptr)
	{
		ControllerEntity->GetComponent<FEGameModelComponent>().SetGameModel(CustomGameModel);

		FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
		ControllerTransform.SetScale(glm::vec3(1.0f));
	}
	
	return true;
}

FEEntity* FEOpenXR::GetLeftControllerEntity() const
{
	return LeftController;
}

FEEntity* FEOpenXR::GetRightControllerEntity() const
{
	return RightController;
}

bool FEOpenXR::TryToAddVRRigToScene(FEScene* Scene)
{
	if (Scene == nullptr)
		return false;

	// FIXME: VRRigEntity also should be deleted if VR is not enabled?
	if (OpenXR_MANAGER.VRRigEntity == nullptr)
	{
		if (OpenXR_MANAGER.VRRigEntity == nullptr)
		{
			OpenXR_MANAGER.VRRigEntity = Scene->CreateEntity("VRRig");
			if (OpenXR_MANAGER.VRRigEntity == nullptr)
				return false;
		}

		if (OpenXR_MANAGER.VRHeadsetEntity == nullptr)
		{
			OpenXR_MANAGER.VRHeadsetEntity = Scene->CreateEntity("VRHeadset");
			OpenXR_MANAGER.VRHeadsetEntity->AddComponent<FECameraComponent>();
			FECameraComponent& VRHeadsetCamera = OpenXR_MANAGER.VRHeadsetEntity->GetComponent<FECameraComponent>();
			CAMERA_SYSTEM.SetCameraRenderingPipeline(VRHeadsetEntity, ENGINE.GetVRRenderingPipeline());
			// FIXME: Temporary solution, SSAO is very slow in VR. And produce artifacts in right eye. strange.
			VRHeadsetCamera.SetSSAOEnabled(false);

			FENaiveSceneGraphNode* VRRigNode = Scene->SceneGraph.GetNodeByEntityID(OpenXR_MANAGER.VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* VRHeadsetNode = Scene->SceneGraph.GetNodeByEntityID(OpenXR_MANAGER.VRHeadsetEntity->GetObjectID());
			Scene->SceneGraph.MoveNode(VRHeadsetNode->GetObjectID(), VRRigNode->GetObjectID());

			if (OpenXR_MANAGER.VRHeadsetEntity == nullptr)
				return false;

			return true;
		}
		else
		{
			return true;
		}

		return false;
	}

	return false;
}

void FEOpenXR::OnControllerConnectionChanges(bool bLeftController, FE_VR_CONTROLLER_STATE_CHANGE Change)
{
	// FIXME: Temporary solution, only supports one scene.
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	if (ActiveScenes.empty())
		return;

	FEScene* CurrentScene = ActiveScenes[0];
	if (CurrentScene == nullptr)
		return;

	OpenXR_MANAGER.TryToAddVRRigToScene(CurrentScene);

	if (CurrentScene->GetEntity(OpenXR_MANAGER.VRRigEntity->GetObjectID()) == nullptr || CurrentScene->GetEntity(OpenXR_MANAGER.VRHeadsetEntity->GetObjectID()) == nullptr)
		return;

	FEEntity*& ControllerEntity = bLeftController ? OpenXR_MANAGER.LeftController : OpenXR_MANAGER.RightController;

	if (Change == FE_VR_CONTROLLER_STATE_CHANGE::CONNECTED)
	{
		if (ControllerEntity == nullptr)
		{
			std::string ControllerName = bLeftController ? "LeftController" : "RightController";
			ControllerEntity = CurrentScene->CreateEntity(ControllerName);
			ControllerEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModel("504029555848336725615C49"));
			FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
			ControllerTransform.SetScale(OpenXR_MANAGER.StandardControllerScale);

			FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(OpenXR_MANAGER.VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* ControllerNode = CurrentScene->SceneGraph.GetNodeByEntityID(ControllerEntity->GetObjectID());
			CurrentScene->SceneGraph.MoveNode(ControllerNode->GetObjectID(), VRRigNode->GetObjectID());
		}

		FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
		ControllerTransform.SetPosition(bLeftController ? FEOpenXR_INPUT.GetLeftControllerPosition() : FEOpenXR_INPUT.GetRightControllerPosition());
		ControllerTransform.SetQuaternion(bLeftController ? FEOpenXR_INPUT.GetLeftControllerOrientation() : FEOpenXR_INPUT.GetRightControllerOrientation());
	}
	else if (Change == FE_VR_CONTROLLER_STATE_CHANGE::DISCONNECTED)
	{
		if (ControllerEntity != nullptr)
		{
			// Before we delete controller from a scene, we should check if it has childs
			// and move them to the VRRigEntity, so they are not lost.
			FENaiveSceneGraphNode* ControllerNode = CurrentScene->SceneGraph.GetNodeByEntityID(ControllerEntity->GetObjectID());
			if (ControllerNode != nullptr && ControllerNode->GetRecursiveChildCount() > 0)
			{
				FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(OpenXR_MANAGER.VRRigEntity->GetObjectID());
				for (auto Child : ControllerNode->GetChildren())
					CurrentScene->SceneGraph.MoveNode(Child->GetObjectID(), VRRigNode->GetObjectID());
			}

			CurrentScene->DeleteEntity(ControllerEntity);
			ControllerEntity = nullptr;
		}
	}
	else if (Change == FE_VR_CONTROLLER_STATE_CHANGE::RECONNECTED)
	{
		if (ControllerEntity == nullptr)
		{
			std::string ControllerName = bLeftController ? "LeftController" : "RightController";
			ControllerEntity = CurrentScene->CreateEntity(ControllerName);
			ControllerEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModel("504029555848336725615C49"));
			FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
			ControllerTransform.SetScale(OpenXR_MANAGER.StandardControllerScale);

			FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(OpenXR_MANAGER.VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* ControllerNode = CurrentScene->SceneGraph.GetNodeByEntityID(ControllerEntity->GetObjectID());
			CurrentScene->SceneGraph.MoveNode(ControllerNode->GetObjectID(), VRRigNode->GetObjectID());
		}

		FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
		ControllerTransform.SetPosition(bLeftController ? FEOpenXR_INPUT.GetLeftControllerPosition() : FEOpenXR_INPUT.GetRightControllerPosition());
		ControllerTransform.SetQuaternion(bLeftController ? FEOpenXR_INPUT.GetLeftControllerOrientation() : FEOpenXR_INPUT.GetRightControllerOrientation());
	}
}