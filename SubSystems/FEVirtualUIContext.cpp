#include "FEVirtualUIContext.h"
using namespace FocalEngine;

FEVirtualUIContext::FEVirtualUIContext(int Width, int Height, FEMesh* SampleMesh, std::string Name) : FEObject(FE_VIRTUAL_UI_CONTEXT, Name)
{
	if (Width == 0 || Height == 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIContext::FEVirtualUIContext", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	CanvasMesh = SampleMesh;
	if (CanvasMesh == nullptr)
		CanvasMesh = RESOURCE_MANAGER.GetMeshByName("FEPlane")[0];

	OriginalMeshTrianglePositions = CanvasMesh->GetTrianglePositions();
	TransformedMeshTrianglePositions = OriginalMeshTrianglePositions;
	MeshTriangleUVs = CanvasMesh->GetTriangleUVs();

	CanvasMaterial = RESOURCE_MANAGER.CreateMaterial();
	RESOURCE_MANAGER.MakeMaterialStandard(CanvasMaterial);
	CanvasMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	CanvasGameModel = RESOURCE_MANAGER.CreateGameModel(CanvasMesh, CanvasMaterial);
	RESOURCE_MANAGER.MakeGameModelStandard(CanvasGameModel);

	Framebuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, Width, Height, false);
	VirtualUI = APPLICATION.AddVirtualUI(Framebuffer->FBO, Width, Height);
	VirtualUI->SetName(Name);

	CanvasMaterial->SetAlbedoMap(Framebuffer->GetColorAttachment());
}

FEVirtualUIContext::~FEVirtualUIContext()
{
	delete Framebuffer;
	APPLICATION.RemoveVirtualUI(VirtualUI);

	UnregisterCallbacksForWindow();
}

void FEVirtualUIContext::GetSize(int* Width, int* Height) const
{
	*Width = Framebuffer->GetWidth();
	*Height = Framebuffer->GetHeight();
}

void FEVirtualUIContext::SetSize(int NewWidth, int NewHeight)
{
	if (NewWidth <= 0 || NewHeight <= 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIContext::SetSize", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	InvokeResize(NewWidth, NewHeight);
}

int FEVirtualUIContext::GetWidth() const
{
	return Framebuffer->GetWidth();
}

int FEVirtualUIContext::GetHeight() const
{
	return Framebuffer->GetHeight();
}

std::function<void()> FEVirtualUIContext::GetRenderFunction()
{
	return VirtualUI->GetOnRenderFunction();
}

void FEVirtualUIContext::SetRenderFunction(std::function<void()> UserRenderFunction)
{
	VirtualUI->SetOnRenderFunction(UserRenderFunction);
}

void FEVirtualUIContext::ClearRenderFunction()
{
	VirtualUI->ClearOnRenderFunction();
}

void FEVirtualUIContext::UpdateCanvasTrianglePositions()
{
	for (size_t i = 0; i < OriginalMeshTrianglePositions.size(); i++)
	{
		TransformedMeshTrianglePositions[i][0] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetTransformMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][0], 1.0f));
		TransformedMeshTrianglePositions[i][1] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetTransformMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][1], 1.0f));
		TransformedMeshTrianglePositions[i][2] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetTransformMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][2], 1.0f));
	}
}

void FEVirtualUIContext::UpdateInteractionRay(glm::dvec3 RayOrigin, glm::dvec3 RayDirection)
{
	InteractionRayOrigin = RayOrigin;
	InteractionRayDirection = RayDirection;
}

FEVirtualUI* FEVirtualUIContext::GetVirtualUI() const
{
	return VirtualUI;
}

bool FEVirtualUIContext::IsInputActive() const
{
	return bActiveInput;
}

void FEVirtualUIContext::SetInputActive(bool NewValue)
{
	bActiveInput = NewValue;
}

FETexture* FEVirtualUIContext::GetCurrentFrameTexture() const
{
	return Framebuffer->GetColorAttachment();
}

void FEVirtualUIContext::AddOnMouseButtonCallback(std::function<void(int, int, int)> UserOnMouseButtonCallback)
{
	VirtualUI->AddOnMouseButtonCallback(UserOnMouseButtonCallback);
}

void FEVirtualUIContext::InvokeMouseButton(int Button, int Action, int Mods)
{
	if (bActiveInput == false)
		return;

	VirtualUI->InvokeMouseButton(Button, Action, Mods);
}

void FEVirtualUIContext::AddOnMouseMoveCallback(std::function<void(double, double)> UserOnMouseMoveCallback)
{
	VirtualUI->AddOnMouseMoveCallback(UserOnMouseMoveCallback);
}

void FEVirtualUIContext::InvokeMouseMove(double Xpos, double Ypos)
{
	if (bActiveInput == false)
		return;

	VirtualUI->InvokeMouseMove(Xpos, Ypos);
}

void FEVirtualUIContext::AddOnResizeCallback(std::function<void(int, int)> UserOnResizeCallback)
{
	VirtualUI->AddOnResizeCallback(UserOnResizeCallback);
}

void FEVirtualUIContext::InvokeResize(int Width, int Height)
{
	if (Width <= 0 || Height <= 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIContext::InvokeResize", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	CanvasMaterial->RemoveTexture(Framebuffer->GetColorAttachment());
	delete Framebuffer;
	Framebuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, Width, Height, false);
	CanvasMaterial->SetAlbedoMap(Framebuffer->GetColorAttachment());

	VirtualUI->InvokeResize(Framebuffer->FBO, Width, Height);
}

void FEVirtualUIContext::AddOnCharCallback(std::function<void(unsigned int)> UserOnCharCallback)
{
	VirtualUI->AddOnCharCallback(UserOnCharCallback);
}

void FEVirtualUIContext::InvokeCharInput(unsigned int codepoint)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeCharInput(codepoint);
}

void FEVirtualUIContext::AddOnKeyCallback(std::function<void(int, int, int, int)> UserOnKeyCallback)
{
	VirtualUI->AddOnKeyCallback(UserOnKeyCallback);
}

void FEVirtualUIContext::InvokeKeyInput(const int Key, const int Scancode, const int Action, const int Mods)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeKeyInput(Key, Scancode, Action, Mods);
}

void FEVirtualUIContext::AddOnScrollCallback(std::function<void(double, double)> UserOnScrollCallback)
{
	VirtualUI->AddOnScrollCallback(UserOnScrollCallback);
}

void FEVirtualUIContext::InvokeScrollInput(const double Xoffset, const double Yoffset)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeScrollInput(Xoffset, Yoffset);
}

void FEVirtualUIContext::AddOnDropCallback(std::function<void(int, const char**)> UserOnDropCallback)
{
	VirtualUI->AddOnDropCallback(UserOnDropCallback);
}

void FEVirtualUIContext::InvokeDropInput(const int Count, const char** Paths)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeDropInput(Count, Paths);
}

void FEVirtualUIContext::MouseButtonListener(int Button, int Action, int Mods)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bMouseButtonPassThrough)
		return;

	if (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_PRESS)
	{
		InvokeMouseButton(ImGuiMouseButton_Left, GLFW_PRESS);
	}
	else if (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_RELEASE)
	{
		InvokeMouseButton(ImGuiMouseButton_Left, GLFW_RELEASE);
	}
	else if (Button == GLFW_MOUSE_BUTTON_RIGHT && Action == GLFW_PRESS)
	{
		InvokeMouseButton(ImGuiMouseButton_Right, GLFW_PRESS);
	}
	else if (Button == GLFW_MOUSE_BUTTON_RIGHT && Action == GLFW_RELEASE)
	{
		InvokeMouseButton(ImGuiMouseButton_Right, GLFW_RELEASE);
	}
	else if (Button == GLFW_MOUSE_BUTTON_MIDDLE && Action == GLFW_PRESS)
	{
		InvokeMouseButton(ImGuiMouseButton_Middle, GLFW_PRESS);
	}
	else if (Button == GLFW_MOUSE_BUTTON_MIDDLE && Action == GLFW_RELEASE)
	{
		InvokeMouseButton(ImGuiMouseButton_Middle, GLFW_RELEASE);
	}
}

bool FEVirtualUIContext::IsMouseButtonPassThroughActive() const
{
	return bMouseButtonPassThrough;
}

bool FEVirtualUIContext::SetMouseButtonPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetMouseButtonPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bMouseButtonPassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::AddOnMouseEnterCallback(std::function<void(int)> UserOnMouseEnterCallback)
{
	VirtualUI->AddOnMouseEnterCallback(UserOnMouseEnterCallback);
}

void FEVirtualUIContext::InvokeMouseEnterCallback(int Entered)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeMouseEnterCallback(Entered);
}

bool FEVirtualUIContext::InteractionRayToCanvasSpace(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, glm::vec2* IntersectionPointInUVCanvasSpace, glm::vec3* IntersectionPointIn3DSpace)
{
	UpdateCanvasTrianglePositions();

	for (size_t i = 0; i < TransformedMeshTrianglePositions.size(); i++)
	{
		std::vector<glm::dvec3> CurrentTriangle;
		CurrentTriangle.push_back(glm::dvec3(TransformedMeshTrianglePositions[i][0].x, TransformedMeshTrianglePositions[i][0].y, TransformedMeshTrianglePositions[i][0].z));
		CurrentTriangle.push_back(glm::dvec3(TransformedMeshTrianglePositions[i][1].x, TransformedMeshTrianglePositions[i][1].y, TransformedMeshTrianglePositions[i][1].z));
		CurrentTriangle.push_back(glm::dvec3(TransformedMeshTrianglePositions[i][2].x, TransformedMeshTrianglePositions[i][2].y, TransformedMeshTrianglePositions[i][2].z));
		double Distance = 0.0;
		glm::dvec3 HitPoint = glm::dvec3(0.0);
		double U = 0.0;
		double V = 0.0;

		if (GEOMETRY.IsRayIntersectingTriangle(RayOrigin, RayDirection, CurrentTriangle, Distance, &HitPoint, &U, &V))
		{
			if (IntersectionPointIn3DSpace != nullptr)
				*IntersectionPointIn3DSpace = HitPoint;

			// Load texture coordinates of the triangle vertices.
			glm::dvec2 uv0 = MeshTriangleUVs[i][0];
			glm::dvec2 uv1 = MeshTriangleUVs[i][1];
			glm::dvec2 uv2 = MeshTriangleUVs[i][2];

			// Calculate texture coordinates of the hit point using interpolation.
			glm::dvec2 HitUV = (1.0 - U - V) * uv0 + U * uv1 + V * uv2;
			*IntersectionPointInUVCanvasSpace = HitUV;

			InvokeMouseEnterCallback(1);
			bRayColidingWithCanvas = true;
			return true;
		}
	}

	InvokeMouseEnterCallback(0);
	bRayColidingWithCanvas = false;
	return false;
}

void FEVirtualUIContext::MouseMoveListener(double Xpos, double Ypos)
{
	// Not useing bRayColidingWithCanvas here because we want to call UpdateRayIntersection() to determine bRayColidingWithCanvas value.
	if (!bActiveInput || !bMouseMovePassThrough)
		return;

	UpdateRayIntersection();
}

void FEVirtualUIContext::UpdateRayIntersection()
{
	glm::vec2 HitUV = glm::vec2(0.0f);
	if (InteractionRayToCanvasSpace(InteractionRayOrigin, InteractionRayDirection, &HitUV))
		InvokeMouseMove(HitUV[0] * GetWidth(), (1.0 - HitUV[1]) * GetHeight());
}

bool FEVirtualUIContext::IsMouseMovePassThroughActive() const
{
	return bMouseMovePassThrough;
}

bool FEVirtualUIContext::SetMouseMovePassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetMouseMovePassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bMouseMovePassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::CharListener(unsigned int Codepoint)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bCharPassThrough)
		return;

	InvokeCharInput(Codepoint);
}

bool FEVirtualUIContext::IsCharPassThroughActive() const
{
	return bCharPassThrough;
}

bool FEVirtualUIContext::SetCharPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetCharPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bCharPassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::KeyListener(int Key, int Scancode, int Action, int Mods)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bKeyPassThrough)
		return;

	InvokeKeyInput(Key, Scancode, Action, Mods);
}

bool FEVirtualUIContext::IsKeyPassThroughActive() const
{
	return bKeyPassThrough;
}

bool FEVirtualUIContext::SetKeyPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetKeyPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bKeyPassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::DropListener(int Count, const char** Paths)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bDropPassThrough)
		return;

	InvokeDropInput(Count, Paths);
}

bool FEVirtualUIContext::IsDropPassThroughActive() const
{
	return bDropPassThrough;
}

bool FEVirtualUIContext::SetDropPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetDropPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bDropPassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::ScrollListener(double Xoffset, double Yoffset)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bScrollPassThrough)
		return;

	InvokeScrollInput(Xoffset, Yoffset);
}

bool FEVirtualUIContext::IsScrollPassThroughActive() const
{
	return bScrollPassThrough;
}

bool FEVirtualUIContext::SetScrollPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIContext::SetScrollPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bScrollPassThrough = NewValue;
	return true;
}

void FEVirtualUIContext::RegisterCallbacksForWindow()
{
	if (WindowToListen == nullptr)
		return;

	auto MouseButtonCallbackToRegister = std::bind(&FEVirtualUIContext::MouseButtonListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	WindowToListen->AddOnMouseButtonCallback(MouseButtonCallbackToRegister);

	auto MouseMoveCallbackToRegister = std::bind(&FEVirtualUIContext::MouseMoveListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->AddOnMouseMoveCallback(MouseMoveCallbackToRegister);

	auto CharCallbackToRegister = std::bind(&FEVirtualUIContext::CharListener, this, std::placeholders::_1);
	WindowToListen->AddOnCharCallback(CharCallbackToRegister);

	auto KeyCallbackToRegister = std::bind(&FEVirtualUIContext::KeyListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	WindowToListen->AddOnKeyCallback(KeyCallbackToRegister);

	auto DropCallbackToRegister = std::bind(&FEVirtualUIContext::DropListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->AddOnDropCallback(DropCallbackToRegister);

	auto ScrollCallbackToRegister = std::bind(&FEVirtualUIContext::ScrollListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->AddOnScrollCallback(ScrollCallbackToRegister);
}

void FEVirtualUIContext::UnregisterCallbacksForWindow()
{
	if (WindowToListen == nullptr)
		return;

	auto MouseButtonCallbackToUnregister = std::bind(&FEVirtualUIContext::MouseButtonListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	WindowToListen->RemoveOnMouseButtonCallback(MouseButtonCallbackToUnregister);

	auto MouseMoveCallbackToUnregister = std::bind(&FEVirtualUIContext::MouseMoveListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->RemoveOnMouseMoveCallback(MouseMoveCallbackToUnregister);

	auto CharCallbackToUnregister = std::bind(&FEVirtualUIContext::CharListener, this, std::placeholders::_1);
	WindowToListen->RemoveOnCharCallback(CharCallbackToUnregister);

	auto KeyCallbackToUnregister = std::bind(&FEVirtualUIContext::KeyListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	WindowToListen->RemoveOnKeyCallback(KeyCallbackToUnregister);

	auto DropCallbackToUnregister = std::bind(&FEVirtualUIContext::DropListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->RemoveOnDropCallback(DropCallbackToUnregister);

	auto ScrollCallbackToUnregister = std::bind(&FEVirtualUIContext::ScrollListener, this, std::placeholders::_1, std::placeholders::_2);
	WindowToListen->RemoveOnScrollCallback(ScrollCallbackToUnregister);
}

void FEVirtualUIContext::SetWindowToListen(FEWindow* Window)
{
	if (WindowToListen != nullptr && WindowToListen != Window)
		UnregisterCallbacksForWindow();

	WindowToListen = Window;
	RegisterCallbacksForWindow();
}
FEWindow* FEVirtualUIContext::GetWindowToListen() const
{
	return WindowToListen;
}

glm::vec3 FEVirtualUIContext::GetPosition() const
{
	return CanvasEntity->GetComponent<FETransformComponent>().GetPosition();
}

void FEVirtualUIContext::SetPosition(glm::vec3 NewPosition)
{
	CanvasEntity->GetComponent<FETransformComponent>().SetPosition(NewPosition);
}

glm::vec3 FEVirtualUIContext::GetRotation() const
{
	return CanvasEntity->GetComponent<FETransformComponent>().GetRotation();
}

void FEVirtualUIContext::SetRotation(glm::vec3 NewRotation)
{
	CanvasEntity->GetComponent<FETransformComponent>().SetRotation(NewRotation);
}

glm::vec3 FEVirtualUIContext::GetScale() const
{
	return CanvasEntity->GetComponent<FETransformComponent>().GetScale();
}

void FEVirtualUIContext::SetScale(glm::vec3 NewScale)
{
	CanvasEntity->GetComponent<FETransformComponent>().SetScale(NewScale);
}

bool FEVirtualUIContext::IsVisible() const
{
	return CanvasEntity->GetComponent<FEGameModelComponent>().IsVisible();
}

void FEVirtualUIContext::SetVisibility(bool NewValue)
{
	CanvasEntity->GetComponent<FEGameModelComponent>().SetVisibility(NewValue);
}

void FEVirtualUIContext::ExecuteFunctionToAddFont(std::function<void()> Func, std::function<void()> CallbackOnFontReady)
{
	VirtualUI->ExecuteFunctionToAddFont(Func, CallbackOnFontReady);
}