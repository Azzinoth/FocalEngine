#include "FEOpenXR.h"

using namespace FocalEngine;

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

	SceneNodesUpdate();
}

glm::vec2 FEOpenXR::EyeResolution()
{
	return FEOpenXR_RENDERING.EyeResolution();
}

void FEOpenXR::SceneNodesUpdate()
{
	// FIXME: Temporary solution, only supports one scene.
	FEScene* CurrentScene = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active)[0];
	if (CurrentScene == nullptr)
		return;

	if (VRRigEntity == nullptr)
	{
		if (VRRigEntity == nullptr)
			VRRigEntity = CurrentScene->CreateEntity("VRRig");

		if (VRHeadsetEntity == nullptr)
		{
			VRHeadsetEntity = CurrentScene->CreateEntity("VRHeadset");
			VRHeadsetEntity->AddComponent<FECameraComponent>();
			FECameraComponent& VRHeadsetCamera = VRHeadsetEntity->GetComponent<FECameraComponent>();
			// FIXME: Temporary solution, SSAO is very slow in VR. And produce artifacts in right eye. strange.
			VRHeadsetCamera.SetSSAOEnabled(false);

			FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* VRHeadsetNode = CurrentScene->SceneGraph.GetNodeByEntityID(VRHeadsetEntity->GetObjectID());
			CurrentScene->SceneGraph.MoveNode(VRHeadsetNode->GetObjectID(), VRRigNode->GetObjectID());
		}
	}

	if (CurrentScene->GetEntity(VRRigEntity->GetObjectID()) == nullptr || CurrentScene->GetEntity(VRHeadsetEntity->GetObjectID()) == nullptr)
		return;

	if (FEOpenXR_INPUT.IsLeftControllerConnectedAndTracked())
	{
		if (LeftController == nullptr)
		{
			LeftController = CurrentScene->CreateEntity("LeftController");
			LeftController->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModel("504029555848336725615C49"));
			FETransformComponent& LeftControllerTransform = LeftController->GetComponent<FETransformComponent>();
			LeftControllerTransform.SetScale(glm::vec3(10.0f));

			FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* LeftControllerNode = CurrentScene->SceneGraph.GetNodeByEntityID(LeftController->GetObjectID());
			CurrentScene->SceneGraph.MoveNode(LeftControllerNode->GetObjectID(), VRRigNode->GetObjectID());
		}

		FETransformComponent& LeftControllerTransform = LeftController->GetComponent<FETransformComponent>();
		LeftControllerTransform.SetPosition(FEOpenXR_INPUT.GetLeftControllerPosition());
		LeftControllerTransform.SetQuaternion(FEOpenXR_INPUT.GetLeftControllerOrientation());
	}
	else
	{
		if (LeftController != nullptr)
		{
			CurrentScene->DeleteEntity(LeftController);
			LeftController = nullptr;
		}
	}

	if (FEOpenXR_INPUT.IsRightControllerConnectedAndTracked())
	{
		if (RightController == nullptr)
		{
			RightController = CurrentScene->CreateEntity("RightController");
			RightController->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModel("504029555848336725615C49"));
			FETransformComponent& RightControllerTransform = RightController->GetComponent<FETransformComponent>();
			RightControllerTransform.SetScale(glm::vec3(10.0f));

			FENaiveSceneGraphNode* VRRigNode = CurrentScene->SceneGraph.GetNodeByEntityID(VRRigEntity->GetObjectID());
			FENaiveSceneGraphNode* RightControllerNode = CurrentScene->SceneGraph.GetNodeByEntityID(RightController->GetObjectID());
			CurrentScene->SceneGraph.MoveNode(RightControllerNode->GetObjectID(), VRRigNode->GetObjectID());
		}

		FETransformComponent& RightControllerTransform = RightController->GetComponent<FETransformComponent>();
		RightControllerTransform.SetPosition(FEOpenXR_INPUT.GetRightControllerPosition());
		RightControllerTransform.SetQuaternion(FEOpenXR_INPUT.GetRightControllerOrientation());
	}
	else
	{
		if (RightController != nullptr)
		{
			CurrentScene->DeleteEntity(RightController);
			RightController = nullptr;
		}
	}
}