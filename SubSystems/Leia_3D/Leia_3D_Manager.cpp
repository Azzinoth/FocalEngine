#include "Leia_3D_Manager.h"
#include "../../FEngine.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "sr/management/srcontext.h"
#include "sr/sense/display/switchablehint.h"
#include "sr/utility/exception.h"
#include "sr/weaver/glweaver.h"

#define SRDISPLAY_LAZYBINDING
#include "sr/world/display/display.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetLeia3DManager()
{
	return Leia3DManager::GetInstancePointer();
}
#endif

// LeiaSR SDK can leave OpenGL errors in the error queue. Engine would then report them
// in the next unrelated FE_GL_ERROR call, so they are logged and cleared right after SDK calls.
static void ReportAndClearLeiaOpenGLErrors(const std::string& FunctionName)
{
	// Limited number of iterations, in case glGetError keeps returning errors without a valid context.
	for (int i = 0; i < 32; i++)
	{
		const GLenum Error = glGetError();
		if (Error == GL_NO_ERROR)
			return;

		LOG.Add("In function " + FunctionName + " LeiaSR SDK left OpenGL error: " + std::to_string(Error), "FE_LOG_LEIA_3D", FE_LOG_WARNING);
	}
}

namespace
{
	void SetOpenGLCapability(const GLenum Capability, const GLboolean bEnabled)
	{
		if (bEnabled)
		{
			FE_GL_ERROR(glEnable(Capability));
		}
		else
		{
			FE_GL_ERROR(glDisable(Capability));
		}
	}

	// ImGui OpenGL backend saves and restores OpenGL state only around its own rendering.
	// Weaving outside of it would leave state changed by weaver to the engine, so it is saved and restored here, similar to what backend does.
	struct OpenGLStateBackup
	{
		GLint DrawFramebuffer = 0;
		GLint ReadFramebuffer = 0;
		GLint Viewport[4] = { 0, 0, 0, 0 };
		GLint ScissorBox[4] = { 0, 0, 0, 0 };
		GLint Program = 0;
		GLint VertexArray = 0;
		GLint ArrayBuffer = 0;
		GLint ActiveTexture = 0;
		GLint FirstUnitTexture = 0;
		GLint BlendSourceRGB = 0;
		GLint BlendDestinationRGB = 0;
		GLint BlendSourceAlpha = 0;
		GLint BlendDestinationAlpha = 0;
		GLint BlendEquationRGB = 0;
		GLint BlendEquationAlpha = 0;
		GLboolean bBlend = GL_FALSE;
		GLboolean bCullFace = GL_FALSE;
		GLboolean bDepthTest = GL_FALSE;
		GLboolean bStencilTest = GL_FALSE;
		GLboolean bScissorTest = GL_FALSE;

		void Save()
		{
			FE_GL_ERROR(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &DrawFramebuffer));
			FE_GL_ERROR(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &ReadFramebuffer));
			FE_GL_ERROR(glGetIntegerv(GL_VIEWPORT, Viewport));
			FE_GL_ERROR(glGetIntegerv(GL_SCISSOR_BOX, ScissorBox));
			FE_GL_ERROR(glGetIntegerv(GL_CURRENT_PROGRAM, &Program));
			FE_GL_ERROR(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &VertexArray));
			FE_GL_ERROR(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &ArrayBuffer));
			FE_GL_ERROR(glGetIntegerv(GL_ACTIVE_TEXTURE, &ActiveTexture));
			// Like in ImGui OpenGL backend, only texture of the first unit is saved.
			FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
			FE_GL_ERROR(glGetIntegerv(GL_TEXTURE_BINDING_2D, &FirstUnitTexture));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_SRC_RGB, &BlendSourceRGB));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_DST_RGB, &BlendDestinationRGB));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_SRC_ALPHA, &BlendSourceAlpha));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_DST_ALPHA, &BlendDestinationAlpha));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_EQUATION_RGB, &BlendEquationRGB));
			FE_GL_ERROR(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &BlendEquationAlpha));
			bBlend = glIsEnabled(GL_BLEND);
			bCullFace = glIsEnabled(GL_CULL_FACE);
			bDepthTest = glIsEnabled(GL_DEPTH_TEST);
			bStencilTest = glIsEnabled(GL_STENCIL_TEST);
			bScissorTest = glIsEnabled(GL_SCISSOR_TEST);
		}

		void Restore() const
		{
			FE_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, DrawFramebuffer));
			FE_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, ReadFramebuffer));
			FE_GL_ERROR(glViewport(Viewport[0], Viewport[1], Viewport[2], Viewport[3]));
			FE_GL_ERROR(glScissor(ScissorBox[0], ScissorBox[1], ScissorBox[2], ScissorBox[3]));
			FE_GL_ERROR(glUseProgram(static_cast<GLuint>(Program)));
			FE_GL_ERROR(glBindVertexArray(static_cast<GLuint>(VertexArray)));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(ArrayBuffer)));
			FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
			FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(FirstUnitTexture)));
			FE_GL_ERROR(glActiveTexture(static_cast<GLenum>(ActiveTexture)));
			FE_GL_ERROR(glBlendEquationSeparate(static_cast<GLenum>(BlendEquationRGB), static_cast<GLenum>(BlendEquationAlpha)));
			FE_GL_ERROR(glBlendFuncSeparate(static_cast<GLenum>(BlendSourceRGB), static_cast<GLenum>(BlendDestinationRGB),
											static_cast<GLenum>(BlendSourceAlpha), static_cast<GLenum>(BlendDestinationAlpha)));
			SetOpenGLCapability(GL_BLEND, bBlend);
			SetOpenGLCapability(GL_CULL_FACE, bCullFace);
			SetOpenGLCapability(GL_DEPTH_TEST, bDepthTest);
			SetOpenGLCapability(GL_STENCIL_TEST, bStencilTest);
			SetOpenGLCapability(GL_SCISSOR_TEST, bScissorTest);
		}
	};
}

Leia3DManager::Leia3DManager()
{
}

Leia3DManager::~Leia3DManager()
{
	Shutdown();
}

bool Leia3DManager::InitializeContext(const bool bInitialLensState)
{
	if (Context != nullptr)
		return true;

	try
	{
		Context = SR::SRContext::create(bInitialLensState);
	}
	catch (const SR::ServerNotAvailableException&)
	{
		LOG.Add("In function Leia3DManager::InitializeContext could not connect to the LeiaSR service, it might not be running.", "FE_LOG_LEIA_3D", FE_LOG_ERROR);
		Context = nullptr;
		return false;
	}

	LensHint = SR::SwitchableLensHint::create(*Context);
	CreateWeaver();
	Context->initialize();
	ReportAndClearLeiaOpenGLErrors("Leia3DManager::InitializeContext");

	return true;
}

void Leia3DManager::CreateWeaver()
{
	FEWindow* MainWindow = APPLICATION.GetMainWindow();
	if (MainWindow == nullptr)
	{
		LOG.Add("In function Leia3DManager::CreateWeaver main window is nullptr.", "FE_LOG_LEIA_3D", FE_LOG_ERROR);
		return;
	}

	HWND WindowHandle = glfwGetWin32Window(MainWindow->GetGlfwWindow());
	if (SR::CreateGLWeaver(*Context, WindowHandle, &Weaver) != WeaverErrorCode::WeaverSuccess)
	{
		LOG.Add("In function Leia3DManager::CreateWeaver failed to create OpenGL weaver.", "FE_LOG_LEIA_3D", FE_LOG_ERROR);
		Weaver = nullptr;
		return;
	}

	// Stereo image is stored as sRGB values in GL_RGBA8 texture and window framebuffer is not sRGB,
	// so weaver should convert to linear after reading and back to sRGB before writing.
	Weaver->setShaderSRGBConversion(true, true);
}

void Leia3DManager::Shutdown()
{
	// Weaver uses both OpenGL context and SR context, so it should be destroyed first.
	if (Weaver != nullptr)
	{
		Weaver->destroy();
		Weaver = nullptr;
	}

	if (StereoViewsFramebuffer != 0)
	{
		FE_GL_ERROR(glDeleteFramebuffers(1, &StereoViewsFramebuffer));
		StereoViewsFramebuffer = 0;
	}

	if (EyeResultFramebuffer != 0)
	{
		FE_GL_ERROR(glDeleteFramebuffers(1, &EyeResultFramebuffer));
		EyeResultFramebuffer = 0;
	}

	if (StereoViewsTexture != 0)
	{
		FE_GL_ERROR(glDeleteTextures(1, &StereoViewsTexture));
		StereoViewsTexture = 0;
	}

	if (FinalResultFramebuffer != nullptr)
	{
		delete FinalResultFramebuffer;
		FinalResultFramebuffer = nullptr;
	}

	// LensHint is owned by Context, so it is deleted together with it.
	if (Context != nullptr)
	{
		SR::SRContext::deleteSRContext(Context);
		Context = nullptr;
		LensHint = nullptr;
	}
}

bool Leia3DManager::Set3DModeEnabled(const bool bNewValue)
{
	if (Context == nullptr)
		return false;

	bNewValue ? LensHint->enable() : LensHint->disable();

	b3DModeEnabled = bNewValue;
	return true;
}

bool Leia3DManager::Is3DModeEnabled() const
{
	return b3DModeEnabled;
}

FETexture* Leia3DManager::GetFinalResult() const
{
	if (FinalResultFramebuffer == nullptr)
		return nullptr;

	return FinalResultFramebuffer->GetColorAttachment();
}

bool Leia3DManager::Initialize(const std::string& InSceneID, const std::string& InViewPortID)
{
	FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(InSceneID);
	if (CurrentScene == nullptr)
		return false;

	FEViewport* CurrentViewport = ENGINE.GetViewport(InViewPortID);
	if (CurrentViewport == nullptr)
		return false;

	SceneID = InSceneID;
	ViewPortID = InViewPortID;
	InitializeScene();
	return true;
}

FEScene* Leia3DManager::GetCurrentScene() const
{
	if (SceneID.empty())
		return nullptr;

	FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(SceneID);
	return CurrentScene;
}

void Leia3DManager::InitializeScene()
{
	FEScene* CurrentScene = GetCurrentScene();
	if (CurrentScene == nullptr)
		return;

	if (MonitorEntity != nullptr)
	{
		MonitorEntity->GetParentScene()->DeleteEntity(MonitorEntity);
		MonitorEntity = nullptr;
	}

	MonitorEntity = CurrentScene->CreateEntity("Leia3DMonitor");

	if (LeftEye != nullptr)
	{
		LeftEye->GetParentScene()->DeleteEntity(LeftEye);
		LeftEye = nullptr;
	}

	LeftEye = CurrentScene->CreateEntity("Leia3DLeftEye");
	LeftEye->AddComponent<FECameraComponent>();
	CAMERA_SYSTEM.SetCameraViewport(LeftEye, ViewPortID);
	MonitorEntity->AttachChild(LeftEye);

	if (RightEye != nullptr)
	{
		RightEye->GetParentScene()->DeleteEntity(RightEye);
		RightEye = nullptr;
	}

	RightEye = CurrentScene->CreateEntity("Leia3DRightEye");
	RightEye->AddComponent<FECameraComponent>();
	CAMERA_SYSTEM.SetCameraViewport(RightEye, ViewPortID);
	MonitorEntity->AttachChild(RightEye);
}

void Leia3DManager::Render()
{
	FEScene* CurrentScene = GetCurrentScene();
	if (CurrentScene == nullptr)
		return;

	if (Weaver == nullptr || LeftEye == nullptr || RightEye == nullptr)
		return;

	// Camera and transform systems already updated this frame, so eye cameras are updated right before rendering.
	UpdateEyeCameras();

	FEEntity* PreviousMainCamera = CAMERA_SYSTEM.GetMainCamera(CurrentScene);

	CAMERA_SYSTEM.SetMainCamera(LeftEye);
	RENDERER.Render(CurrentScene);

	CAMERA_SYSTEM.SetMainCamera(RightEye);
	RENDERER.Render(CurrentScene);

	if (PreviousMainCamera != nullptr)
		CAMERA_SYSTEM.SetMainCamera(PreviousMainCamera);

	FETexture* LeftEyeResult = RENDERER.GetCameraResult(LeftEye);
	FETexture* RightEyeResult = RENDERER.GetCameraResult(RightEye);
	if (LeftEyeResult == nullptr || RightEyeResult == nullptr)
		return;

	if (LeftEyeResult->GetWidth() <= 0 || LeftEyeResult->GetHeight() <= 0)
		return;

	UpdateStereoViewsTexture(LeftEyeResult->GetWidth(), LeftEyeResult->GetHeight());

	// Copy each eye result into its half of StereoViewsTexture.
	// glBlitFramebuffer also converts formats, camera result could be for example GL_RGBA16F.
	GLint PreviousReadFramebuffer = 0;
	GLint PreviousDrawFramebuffer = 0;
	FE_GL_ERROR(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &PreviousReadFramebuffer));
	FE_GL_ERROR(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &PreviousDrawFramebuffer));
	// Blit is affected by scissor test.
	const GLboolean bScissorTestWasEnabled = glIsEnabled(GL_SCISSOR_TEST);
	FE_GL_ERROR(glDisable(GL_SCISSOR_TEST));

	FE_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, StereoViewsFramebuffer));
	FE_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, EyeResultFramebuffer));

	FETexture* EyeResults[2] = { LeftEyeResult, RightEyeResult };
	for (int i = 0; i < 2; i++)
	{
		FE_GL_ERROR(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EyeResults[i]->GetTextureID(), 0));
		// Linear filter, in case for one frame right eye result has different size than left one.
		FE_GL_ERROR(glBlitFramebuffer(0, 0, EyeResults[i]->GetWidth(), EyeResults[i]->GetHeight(),
									  i * StereoViewsViewWidth, 0, (i + 1) * StereoViewsViewWidth, StereoViewsViewHeight,
									  GL_COLOR_BUFFER_BIT, GL_LINEAR));
	}

	// Eye result textures are owned by renderer, so they should not stay attached here.
	FE_GL_ERROR(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));

	FE_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, PreviousReadFramebuffer));
	FE_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PreviousDrawFramebuffer));
	if (bScissorTestWasEnabled)
		FE_GL_ERROR(glEnable(GL_SCISSOR_TEST));

	RenderFinalResult();
}

void Leia3DManager::RenderFinalResult()
{
	FEViewport* Viewport = ENGINE.GetViewport(ViewPortID);
	if (Viewport == nullptr || Viewport->GetWidth() <= 0 || Viewport->GetHeight() <= 0)
		return;

	FEWindow* MainWindow = APPLICATION.GetMainWindow();
	if (MainWindow == nullptr)
		return;

	int FramebufferWidth = 0;
	int FramebufferHeight = 0;
	glfwGetFramebufferSize(MainWindow->GetGlfwWindow(), &FramebufferWidth, &FramebufferHeight);
	if (FramebufferWidth <= 0 || FramebufferHeight <= 0)
		return;

	// Rectangle that eye cameras were rendered for, in framebuffer pixels, with top-left origin like in ImGui.
	// Viewport is in window client coordinates, like in UpdateEyeCameras.
	const ImVec2 Scale = ImGui::GetIO().DisplayFramebufferScale;
	const int PixelX = static_cast<int>(Viewport->GetX() * Scale.x);
	const int PixelY = static_cast<int>(Viewport->GetY() * Scale.y);
	const int PixelWidth = static_cast<int>(Viewport->GetWidth() * Scale.x);
	const int PixelHeight = static_cast<int>(Viewport->GetHeight() * Scale.y);

	OpenGLStateBackup StateBackup;
	StateBackup.Save();

	// Creating textures and framebuffers also changes OpenGL state, so it is done after state is saved.
	UpdateFinalResult(FramebufferWidth, FramebufferHeight);
	if (FinalResultFramebuffer == nullptr)
	{
		StateBackup.Restore();
		return;
	}

	FinalResultFramebuffer->Bind();

	// LeiaSR OpenGL weaver clips its output to scissor rectangle, but reads its Y from the top of the window.
	// Scissor rectangle of the whole texture is the same in both conventions.
	FE_GL_ERROR(glDisable(GL_SCISSOR_TEST));
	FE_GL_ERROR(glScissor(0, 0, FinalResultFramebuffer->GetWidth(), FinalResultFramebuffer->GetHeight()));

	// Weaver draws nothing while window is occluded, and window could move since the previous frame, so old woven pixels should not stay.
	const GLfloat Black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	FE_GL_ERROR(glClearBufferfv(GL_COLOR, 0, Black));

	// ImGui uses top-left origin, OpenGL uses bottom-left origin.
	FE_GL_ERROR(glViewport(PixelX, FinalResultFramebuffer->GetHeight() - PixelY - PixelHeight, PixelWidth, PixelHeight));

	Weaver->weave();
	ReportAndClearLeiaOpenGLErrors("Leia3DManager::RenderFinalResult");

	FinalResultFramebuffer->UnBind();
	StateBackup.Restore();
}

float Leia3DManager::GetWorldUnitsPerMillimeter() const
{
	return WorldUnitsPerMillimeter;
}

void Leia3DManager::SetWorldUnitsPerMillimeter(const float NewValue)
{
	if (NewValue <= 0.0f)
	{
		LOG.Add("In function Leia3DManager::SetWorldUnitsPerMillimeter value should be positive.", "FE_LOG_LEIA_3D", FE_LOG_WARNING);
		return;
	}

	WorldUnitsPerMillimeter = NewValue;
}

void Leia3DManager::UpdateEyeCameras()
{
	if (Context == nullptr || Weaver == nullptr || MonitorEntity == nullptr || LeftEye == nullptr || RightEye == nullptr)
		return;

	FEViewport* Viewport = ENGINE.GetViewport(ViewPortID);
	if (Viewport == nullptr || Viewport->GetWidth() <= 0 || Viewport->GetHeight() <= 0)
		return;

	FEWindow* MainWindow = APPLICATION.GetMainWindow();
	if (MainWindow == nullptr)
		return;

	SR::IDisplayManager* DisplayManager = SR::TryGetDisplayManagerInstance(*Context);
	if (DisplayManager == nullptr)
	{
		LOG.Add("In function Leia3DManager::UpdateEyeCameras display manager is not available.", "FE_LOG_LEIA_3D", FE_LOG_WARNING);
		return;
	}

	SR::IDisplay* Display = DisplayManager->getPrimaryActiveSRDisplay();
	if (Display == nullptr || !Display->isValid())
		return;

	// Eye tracker uses display coordinates: millimeters, origin in the center of the display, X to the right, Y up, Z towards the viewer.
	// Location of the display is in desktop pixels, physical size is in centimeters.
	const SR_recti DisplayLocation = Display->getLocation();
	const double DisplayWidthInPixels = static_cast<double>(DisplayLocation.right - DisplayLocation.left);
	const double DisplayHeightInPixels = static_cast<double>(DisplayLocation.bottom - DisplayLocation.top);
	if (DisplayWidthInPixels <= 0.0 || DisplayHeightInPixels <= 0.0)
		return;

	const double MillimetersPerPixelX = Display->getPhysicalSizeWidth() * 10.0 / DisplayWidthInPixels;
	const double MillimetersPerPixelY = Display->getPhysicalSizeHeight() * 10.0 / DisplayHeightInPixels;

	// 3D area is the viewport, not the whole display. Its rectangle is in window client pixels, so it is moved to desktop pixels first.
	POINT ClientOrigin = { 0, 0 };
	ClientToScreen(glfwGetWin32Window(MainWindow->GetGlfwWindow()), &ClientOrigin);
	const double AreaCenterXInPixels = ClientOrigin.x + Viewport->GetX() + Viewport->GetWidth() / 2.0;
	const double AreaCenterYInPixels = ClientOrigin.y + Viewport->GetY() + Viewport->GetHeight() / 2.0;

	// Desktop Y goes down, display Y goes up.
	const glm::vec3 AreaCenter = glm::vec3(static_cast<float>((AreaCenterXInPixels - (DisplayLocation.left + DisplayLocation.right) / 2.0) * MillimetersPerPixelX),
										   static_cast<float>(-(AreaCenterYInPixels - (DisplayLocation.top + DisplayLocation.bottom) / 2.0) * MillimetersPerPixelY),
										   0.0f);
	const float AreaWidth = static_cast<float>(Viewport->GetWidth() * MillimetersPerPixelX);
	const float AreaHeight = static_cast<float>(Viewport->GetHeight() * MillimetersPerPixelY);

	glm::vec3 LeftEyePosition = glm::vec3(0.0f);
	glm::vec3 RightEyePosition = glm::vec3(0.0f);
	Weaver->getPredictedEyePositions(&LeftEyePosition.x, &RightEyePosition.x);

	// When nobody is tracked, eyes could be at zero, then default viewing position of the display is used.
	if (LeftEyePosition.z < 1.0f || RightEyePosition.z < 1.0f)
	{
		glm::vec3 DefaultViewingPosition = glm::vec3(0.0f);
		Display->getDefaultViewingPosition(DefaultViewingPosition.x, DefaultViewingPosition.y, DefaultViewingPosition.z);
		// Average distance between eyes is about 64 millimeters.
		const glm::vec3 HalfEyeDistance = glm::vec3(32.0f, 0.0f, 0.0f);
		LeftEyePosition = DefaultViewingPosition - HalfEyeDistance;
		RightEyePosition = DefaultViewingPosition + HalfEyeDistance;
	}

	UpdateEyeCamera(LeftEye, LeftEyePosition - AreaCenter, AreaWidth, AreaHeight);
	UpdateEyeCamera(RightEye, RightEyePosition - AreaCenter, AreaWidth, AreaHeight);
}

void Leia3DManager::UpdateEyeCamera(FEEntity* Eye, const glm::vec3 EyePosition, const float AreaWidth, const float AreaHeight)
{
	if (Eye == nullptr || EyePosition.z <= 0.0f)
		return;

	FETransformComponent& EyeTransform = Eye->GetComponent<FETransformComponent>();
	FECameraComponent& EyeCamera = Eye->GetComponent<FECameraComponent>();

	// Eye camera is a child of MonitorEntity, which is the center of the 3D area, with the area in its local XY plane, facing +Z.
	// Eye keeps default orientation, so it looks along -Z, straight at the plane of the area.
	EyeTransform.SetPosition(EyePosition * WorldUnitsPerMillimeter);

	// Transform and camera systems already updated this frame, so world and view matrices are updated here.
	const glm::mat4 EyeWorldMatrix = MonitorEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * EyeTransform.GetLocalMatrix();
	EyeTransform.ForceSetWorldMatrix(EyeWorldMatrix);
	EyeCamera.SetViewMatrix(glm::inverse(EyeWorldMatrix));

	// Off-axis projection (Kooima, "Generalized Perspective Projection"): frustum goes from the eye through the edges of the 3D area.
	// In eye space area edges are just offsets from the eye, scaled to the near plane. Only ratios are used, so millimeters are fine here.
	const float NearPlane = EyeCamera.GetNearPlane();
	const float ToNearPlane = NearPlane / EyePosition.z;
	const float Left = (-AreaWidth / 2.0f - EyePosition.x) * ToNearPlane;
	const float Right = (AreaWidth / 2.0f - EyePosition.x) * ToNearPlane;
	const float Bottom = (-AreaHeight / 2.0f - EyePosition.y) * ToNearPlane;
	const float Top = (AreaHeight / 2.0f - EyePosition.y) * ToNearPlane;
	EyeCamera.SetProjectionMatrix(glm::frustum(Left, Right, Bottom, Top, NearPlane, EyeCamera.GetFarPlane()));
}

void Leia3DManager::UpdateStereoViewsTexture(const int ViewWidth, const int ViewHeight)
{
	if (StereoViewsTexture != 0 && ViewWidth == StereoViewsViewWidth && ViewHeight == StereoViewsViewHeight)
		return;

	if (StereoViewsTexture == 0)
		FE_GL_ERROR(glGenTextures(1, &StereoViewsTexture));

	if (StereoViewsFramebuffer == 0)
		FE_GL_ERROR(glGenFramebuffers(1, &StereoViewsFramebuffer));

	if (EyeResultFramebuffer == 0)
		FE_GL_ERROR(glGenFramebuffers(1, &EyeResultFramebuffer));

	// Left eye view in the left half, right eye view in the right half.
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, StereoViewsTexture));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ViewWidth * 2, ViewHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

	GLint PreviousDrawFramebuffer = 0;
	FE_GL_ERROR(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &PreviousDrawFramebuffer));
	FE_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, StereoViewsFramebuffer));
	FE_GL_ERROR(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, StereoViewsTexture, 0));
	FE_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PreviousDrawFramebuffer));

	StereoViewsViewWidth = ViewWidth;
	StereoViewsViewHeight = ViewHeight;
	Weaver->setInputViewTexture(StereoViewsTexture, StereoViewsViewWidth, StereoViewsViewHeight, GL_RGBA8);
}

void Leia3DManager::UpdateFinalResult(const int Width, const int Height)
{
	if (FinalResultFramebuffer != nullptr && FinalResultFramebuffer->GetWidth() == Width && FinalResultFramebuffer->GetHeight() == Height)
		return;

	delete FinalResultFramebuffer;
	FinalResultFramebuffer = RESOURCE_MANAGER.CreateFramebuffer(0, Width, Height);
	if (FinalResultFramebuffer == nullptr)
		return;

	FETexture* FinalResult = RESOURCE_MANAGER.CreateTexture(GL_RGB8, GL_RGB, Width, Height, true, "Leia3DFinalResult");
	// Each "3D" pixel is meant for one exact physical pixel, so it should not be mixed with its neighbors.
	FinalResult->SetFilterType(FE_TEXTURE_MINMAG_FILTER_TYPE::NEAREST);
	FinalResultFramebuffer->SetColorAttachment(FinalResult);

	FinalResultFramebuffer->Bind();
	const GLenum FramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (FramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
		LOG.Add("In function Leia3DManager::UpdateFinalResult framebuffer is not complete, status: " + std::to_string(FramebufferStatus), "FE_LOG_LEIA_3D", FE_LOG_ERROR);

	FinalResultFramebuffer->UnBind();
}