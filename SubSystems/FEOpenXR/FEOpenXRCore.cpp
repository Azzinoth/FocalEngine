#include "FEOpenXRCore.h"
#include "../../FEngine.h"

using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetOpenXRCore()
{
	return FEOpenXRCore::GetInstancePointer();
}
#endif

FEOpenXRCore::FEOpenXRCore() {}
FEOpenXRCore::~FEOpenXRCore() {}

void FEOpenXRCore::CreateInstance()
{
	std::vector<const char*> ExtensionToRequest;
	ExtensionToRequest.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

	std::vector<FEOpenXRExtensionInfo> AvailableExtensions = GetAvailableExtensionsInfo();
	for (size_t i = 0; i < AvailableExtensions.size(); i++)
	{
		if (AvailableExtensions[i].Name == XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME)
		{
			bGazeSupported = true;
			ExtensionToRequest.push_back(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME);
			break;
		}	
	}

	XrInstanceCreateInfo CreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	strcpy_s(CreateInfo.applicationInfo.applicationName, sizeof(CreateInfo.applicationInfo.applicationName), ENGINE.GetVRApplicationVisibleName().c_str());
	CreateInfo.applicationInfo.applicationVersion = 1;
	strcpy_s(CreateInfo.applicationInfo.engineName, sizeof(CreateInfo.applicationInfo.engineName), "Focal Engine");
	CreateInfo.applicationInfo.engineVersion = 1;
	CreateInfo.applicationInfo.apiVersion = XR_API_VERSION_1_0;

	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(ExtensionToRequest.size());
	CreateInfo.enabledExtensionNames = ExtensionToRequest.data();

	FE_OPENXR_ERROR(xrCreateInstance(&CreateInfo, &OpenXRInstance));
	if (OpenXRInstance == nullptr)
	{
		bInitializedCorrectly = false;
		return;
	}

	ReadRuntimeInfo();
}

void FEOpenXRCore::InitializeSystem()
{
	if (!bInitializedCorrectly)
		return;

	XrSystemGetInfo SystemGetInfo = { XR_TYPE_SYSTEM_GET_INFO };
	SystemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	FE_OPENXR_ERROR(xrGetSystem(OpenXRInstance, &SystemGetInfo, &SystemID));
	if (SystemID == 0)
		bInitializedCorrectly = false;
}

void FEOpenXRCore::InitializeSession()
{
	if (!bInitializedCorrectly)
		return;

	XrGraphicsRequirementsOpenGLKHR OpenGLRequirements;
	OpenGLRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
	OpenGLRequirements.next = nullptr;

	PFN_xrGetOpenGLGraphicsRequirementsKHR OpenGLRequirementsFunction = NULL;
	FE_OPENXR_ERROR(xrGetInstanceProcAddr(OpenXRInstance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&OpenGLRequirementsFunction));
	FE_OPENXR_ERROR(OpenGLRequirementsFunction(OpenXRInstance, SystemID, &OpenGLRequirements));

	XrGraphicsBindingOpenGLWin32KHR GraphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
	GraphicsBinding.hDC = wglGetCurrentDC();
	GraphicsBinding.hGLRC = wglGetCurrentContext();

	XrSessionCreateInfo SessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
	SessionCreateInfo.next = &GraphicsBinding;
	SessionCreateInfo.systemId = SystemID;

	FE_OPENXR_ERROR(xrCreateSession(OpenXRInstance, &SessionCreateInfo, &Session));
	if (Session == nullptr)
		bInitializedCorrectly = false;
}

void FEOpenXRCore::CreateReferenceSpace()
{
	if (!bInitializedCorrectly)
		return;

	XrPosef IdentityPose;
	IdentityPose.orientation.x = 0;
	IdentityPose.orientation.y = 0;
	IdentityPose.orientation.z = 0;
	IdentityPose.orientation.w = 1.0;

	IdentityPose.position.x = 0;
	IdentityPose.position.y = 0;
	IdentityPose.position.z = 0;

	XrReferenceSpaceCreateInfo PlaySpaceCreateInfo;
	PlaySpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	PlaySpaceCreateInfo.next = nullptr;
	PlaySpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	PlaySpaceCreateInfo.poseInReferenceSpace = IdentityPose;

	FE_OPENXR_ERROR(xrCreateReferenceSpace(Session, &PlaySpaceCreateInfo, &ApplicationSpace));
	if (ApplicationSpace == nullptr)
		bInitializedCorrectly = false;
}

void FEOpenXRCore::Init()
{
	bInitializedCorrectly = true;
	CreateInstance();
	InitializeSystem();
	InitializeSession();
	CreateReferenceSpace();
}

void FEOpenXRCore::Shutdown()
{
	bInitializedCorrectly = false;
	bSessionIsRunning = false;

	if (ApplicationSpace != nullptr)
	{
		FE_OPENXR_ERROR(xrDestroySpace(ApplicationSpace));
		ApplicationSpace = nullptr;
	}

	if (Session != nullptr)
	{
		FE_OPENXR_ERROR(xrDestroySession(Session));
		Session = nullptr;
	}

	if (OpenXRInstance != nullptr)
	{
		FE_OPENXR_ERROR(xrDestroyInstance(OpenXRInstance));
		OpenXRInstance = nullptr;
	}

	SystemID = 0;
	SessionState = XR_SESSION_STATE_UNKNOWN;
}

FEOpenXRRuntimeInfo FEOpenXRCore::GetRuntimeInfo()
{
	return ActiveRuntimeInfo;
}

bool FEOpenXRCore::ReadRuntimeInfo()
{
	if (OpenXRInstance == nullptr)
		return false;

	XrInstanceProperties InstanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
	XrResult Result = FE_OPENXR_ERROR(xrGetInstanceProperties(OpenXRInstance, &InstanceProperties));
	if (Result != XR_SUCCESS)
		return false;

	ActiveRuntimeInfo.Name = InstanceProperties.runtimeName;
	ActiveRuntimeInfo.Version = std::to_string(XR_VERSION_MAJOR(InstanceProperties.runtimeVersion)) + "." +
								std::to_string(XR_VERSION_MINOR(InstanceProperties.runtimeVersion)) + "." +
								std::to_string(XR_VERSION_PATCH(InstanceProperties.runtimeVersion));

	std::string LowerCaseName = ActiveRuntimeInfo.Name;
	std::transform(LowerCaseName.begin(), LowerCaseName.end(), LowerCaseName.begin(), ::tolower);

	if (LowerCaseName.find("somnium") != std::string::npos)
	{
		ActiveRuntimeInfo.Type = FE_VR_OPENXR_RUNTIME::SOMNIUM;
	}
	else if (LowerCaseName.find("steamVR") != std::string::npos)
	{
		ActiveRuntimeInfo.Type = FE_VR_OPENXR_RUNTIME::STEAM_VR;
	}
	else if (LowerCaseName.find("oculus") != std::string::npos || LowerCaseName.find("meta") != std::string::npos)
	{
		ActiveRuntimeInfo.Type = FE_VR_OPENXR_RUNTIME::META;
	}
	else if (LowerCaseName.find("varjo") != std::string::npos)
	{
		ActiveRuntimeInfo.Type = FE_VR_OPENXR_RUNTIME::VARJO;
	}

	return true;
}

std::vector<FEOpenXRExtensionInfo> FEOpenXRCore::GetAvailableExtensionsInfo()
{
	std::vector<FEOpenXRExtensionInfo> Extensions;

	uint32_t ExtensionCount = 0;
	XrResult Result = xrEnumerateInstanceExtensionProperties(nullptr, 0, &ExtensionCount, nullptr);

	if (Result != XR_SUCCESS)
	{
		LOG.Add("Failed to enumerate OpenXR extension count: " + XrResultToString(Result) + " in function GetAvailableExtensionsInfo()", "FE_LOG_OPENXR", FE_LOG_WARNING);
		return Extensions;
	}

	if (ExtensionCount == 0)
	{
		LOG.Add("No OpenXR extensions available in function GetAvailableExtensionsInfo()", "FE_LOG_OPENXR", FE_LOG_WARNING);
		return Extensions;
	}

	std::vector<XrExtensionProperties> AvailableExtensions(ExtensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
	Result = xrEnumerateInstanceExtensionProperties(nullptr, ExtensionCount, &ExtensionCount, AvailableExtensions.data());

	if (Result != XR_SUCCESS)
	{
		LOG.Add("Failed to enumerate OpenXR extensions: " + XrResultToString(Result) + " in function GetAvailableExtensionsInfo()", "FE_LOG_OPENXR", FE_LOG_WARNING);
		return Extensions;
	}

	Extensions.reserve(ExtensionCount);
	for (const auto& Extension : AvailableExtensions)
	{
		FEOpenXRExtensionInfo ExtensionInfo;
		ExtensionInfo.Name = Extension.extensionName;
		ExtensionInfo.Version = Extension.extensionVersion;
		Extensions.push_back(ExtensionInfo);
	}

	return Extensions;
}