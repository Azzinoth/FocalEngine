#include "FEOpenXRCore.h"

using namespace FocalEngine;

FEOpenXRCore* FEOpenXRCore::Instance = nullptr;

FEOpenXRCore::FEOpenXRCore() {}
FEOpenXRCore::~FEOpenXRCore() {}

void FEOpenXRCore::CreateInstance()
{
	std::vector<const char*> Extension;
	Extension.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

	XrInstanceCreateInfo CreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	strcpy_s(CreateInfo.applicationInfo.applicationName, sizeof(CreateInfo.applicationInfo.applicationName), VRAppName.c_str());
	CreateInfo.applicationInfo.applicationVersion = 1;
	strcpy_s(CreateInfo.applicationInfo.engineName, sizeof(CreateInfo.applicationInfo.engineName), "NoEngine");
	CreateInfo.applicationInfo.engineVersion = 1;
	CreateInfo.applicationInfo.apiVersion = XR_API_VERSION_1_0;

	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extension.size());
	CreateInfo.enabledExtensionNames = Extension.data();

	FE_OPENXR_ERROR(xrCreateInstance(&CreateInfo, &OpenXRInstance));
	if (OpenXRInstance == nullptr)
		bInitializedCorrectly = false;
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

	XrGraphicsRequirementsOpenGLKHR OpenglReqs;
	OpenglReqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
	OpenglReqs.next = nullptr;

	PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = NULL;
	{
		FE_OPENXR_ERROR(xrGetInstanceProcAddr(OpenXRInstance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR));
	}

	FE_OPENXR_ERROR(pfnGetOpenGLGraphicsRequirementsKHR(OpenXRInstance, SystemID, &OpenglReqs));

	XrGraphicsBindingOpenGLWin32KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
	graphicsBinding.hDC = wglGetCurrentDC();
	graphicsBinding.hGLRC = wglGetCurrentContext();

	XrSessionCreateInfo SessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
	SessionCreateInfo.next = &graphicsBinding;
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

void FEOpenXRCore::Init(std::string VRAppName)
{
	if (!VRAppName.empty())
	 this->VRAppName = VRAppName;

	bInitializedCorrectly = true;
	CreateInstance();
	InitializeSystem();
	InitializeSession();
	CreateReferenceSpace();
}