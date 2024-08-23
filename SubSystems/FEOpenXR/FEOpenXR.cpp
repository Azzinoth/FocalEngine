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
		for (auto hand : { Side::LEFT, Side::RIGHT })
		{
			xrDestroySpace(FEOpenXR_INPUT.CurrentInputState.HandSpace[hand]);
		}
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

bool FEOpenXR::Init(std::string VRAppName)
{
	FEOpenXR_CORE.Init(VRAppName);
	FEOpenXR_INPUT.Init();
	FEOpenXR_RENDERING.Init();

	FEOpenXR_INPUT.FrameState = &FEOpenXR_RENDERING.FrameState;

	return FEOpenXR_CORE.bInitializedCorrectly;
}

void FEOpenXR::PollEvents()
{
	bool session_stopping = false;

	// --- Handle runtime Events
	// we do this before xrWaitFrame() so we can go idle or
	// break out of the main render loop as early as possible and don't have to
	// uselessly render or submit one. Calling xrWaitFrame commits you to
	// calling xrBeginFrame eventually.
	XrEventDataBuffer runtime_event;
	runtime_event.type = XR_TYPE_EVENT_DATA_BUFFER;
	runtime_event.next = nullptr;

	XrResult poll_result = xrPollEvent(FEOpenXR_CORE.OpenXRInstance, &runtime_event);
	while (poll_result == XR_SUCCESS)
	{
		switch (runtime_event.type)
		{
		case XR_TYPE_EVENT_DATA_EVENTS_LOST:
		{
			XrEventDataEventsLost* event = (XrEventDataEventsLost*)&runtime_event;

			LOG.Add("EVENT: " + std::to_string(event->lostEventCount) + " events data lost!", "FE_LOG_OPENXR");
			// do we care if the runtime loses events?
			break;
		}
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* event = (XrEventDataInstanceLossPending*)&runtime_event;
			LOG.Add("EVENT: instance loss pending at " + std::to_string(event->lossTime) + "! Destroying instance.", "FE_LOG_OPENXR");
			session_stopping = true;
			break;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* event = (XrEventDataSessionStateChanged*)&runtime_event;
			//printf("EVENT: session state changed from %d to %d\n", SessionState, event->state);

			FEOpenXR_CORE.SessionState = event->state;

			switch (FEOpenXR_CORE.SessionState)
			{
				case XR_SESSION_STATE_READY:
				{
					XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
					sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
					FE_OPENXR_ERROR(xrBeginSession(FEOpenXR_CORE.Session, &sessionBeginInfo));
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
			XrEventDataReferenceSpaceChangePending* event =
				(XrEventDataReferenceSpaceChangePending*)&runtime_event;
			(void)event;
			// TODO: do something
			break;
		}
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			//printf("EVENT: interaction profile changed!\n");
			XrEventDataInteractionProfileChanged* event =
				(XrEventDataInteractionProfileChanged*)&runtime_event;
			(void)event;

			XrInteractionProfileState state;
			state.type = XR_TYPE_INTERACTION_PROFILE_STATE;

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
			XrEventDataVisibilityMaskChangedKHR* event =
				(XrEventDataVisibilityMaskChangedKHR*)&runtime_event;
			(void)event;
			// this event is from an extension
			break;
		}
		case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT:
		{
			//printf("EVENT: perf settings!\n");
			XrEventDataPerfSettingsEXT* event = (XrEventDataPerfSettingsEXT*)&runtime_event;
			(void)event;
			// this event is from an extension
			break;
		}

		default:;//printf("Unhandled event type %d\n", runtime_event.type);
		}

		runtime_event.type = XR_TYPE_EVENT_DATA_BUFFER;
		poll_result = xrPollEvent(FEOpenXR_CORE.OpenXRInstance, &runtime_event);
	}
	if (poll_result == XR_EVENT_UNAVAILABLE)
	{
		// processed all events in the queue
	}
	else
	{
		printf("Failed to poll events!\n");
		//break;
	}

	if (session_stopping)
	{
		printf("Quitting main render loop\n");
		return;
	}
}

void FEOpenXR::Update()
{
	PollEvents();
	FEOpenXR_RENDERING.Update();
	FEOpenXR_INPUT.Update();
}

glm::vec2 FEOpenXR::EyeResolution()
{
	return FEOpenXR_RENDERING.EyeResolution();
}