#include "FEVirtualUIComponent.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../FEEntity.h"
using namespace FocalEngine;

FEVirtualUIComponent::FEVirtualUIComponent(int Width, int Height, FEMesh* CanvasMesh)
{
	if (Width == 0 || Height == 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIComponent::FEVirtualUIComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (CanvasMesh == nullptr)
		CanvasMesh = RESOURCE_MANAGER.GetMeshByName("FEPlane")[0];

	SetCanvasMesh(CanvasMesh);

	Framebuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, Width, Height, false);
	VirtualUI = APPLICATION.AddVirtualUI(Framebuffer->FBO, Width, Height);
}

FEVirtualUIComponent::~FEVirtualUIComponent()
{
	delete Framebuffer;
	APPLICATION.RemoveVirtualUI(VirtualUI);

	UnregisterCallbacksForWindow();
}

FEMesh* FEVirtualUIComponent::GetCanvasMesh() const
{
	return CanvasMesh;
}

void FEVirtualUIComponent::SetCanvasMesh(FEMesh* NewCanvasMesh)
{
	if (NewCanvasMesh == nullptr)
	{
		LOG.Add("Invalid mesh in FEVirtualUIComponent::SetCanvasMesh", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	CanvasMesh = NewCanvasMesh;
	OriginalMeshTrianglePositions = CanvasMesh->GetTrianglePositions();
	TransformedMeshTrianglePositions = OriginalMeshTrianglePositions;
	MeshTriangleUVs = CanvasMesh->GetTriangleUVs();
}

glm::vec2 FEVirtualUIComponent::GetCanvasResolution() const
{
	glm::vec2 Result;
	Result.x = static_cast<float>(Framebuffer->GetWidth());
	Result.y = static_cast<float>(Framebuffer->GetHeight());

	return Result;
}

void FEVirtualUIComponent::SetCanvasResolution(glm::vec2 NewResolution)
{
	if (NewResolution.x <= 0 || NewResolution.y <= 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIComponent::SetCanvasResolution", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	InvokeResize(static_cast<int>(NewResolution.x), static_cast<int>(NewResolution.y));
}

int FEVirtualUIComponent::GetWidth() const
{
	return Framebuffer->GetWidth();
}

int FEVirtualUIComponent::GetHeight() const
{
	return Framebuffer->GetHeight();
}

VIRTUAL_UI_RENDER_FUNCTION FEVirtualUIComponent::GetRenderFunction()
{
	return VirtualUI->GetOnRenderFunction();
}

void FEVirtualUIComponent::SetRenderFunction(VIRTUAL_UI_RENDER_FUNCTION UserRenderFunction)
{
	VirtualUI->SetOnRenderFunction(UserRenderFunction);
}

void FEVirtualUIComponent::ClearRenderFunction()
{
	VirtualUI->ClearOnRenderFunction();
}

void FEVirtualUIComponent::UpdateCanvasTrianglePositions()
{
	if (ParentEntity == nullptr)
		return;
	
	for (size_t i = 0; i < OriginalMeshTrianglePositions.size(); i++)
	{
		TransformedMeshTrianglePositions[i][0] = glm::vec3(ParentEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][0], 1.0f));
		TransformedMeshTrianglePositions[i][1] = glm::vec3(ParentEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][1], 1.0f));
		TransformedMeshTrianglePositions[i][2] = glm::vec3(ParentEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalMeshTrianglePositions[i][2], 1.0f));
	}
}

void FEVirtualUIComponent::UpdateInteractionRay(glm::dvec3 RayOrigin, glm::dvec3 RayDirection)
{
	InteractionRayOrigin = RayOrigin;
	InteractionRayDirection = RayDirection;
}

FEVirtualUI* FEVirtualUIComponent::GetVirtualUI() const
{
	return VirtualUI;
}

bool FEVirtualUIComponent::IsInputActive() const
{
	return bActiveInput;
}

void FEVirtualUIComponent::SetInputActive(bool NewValue)
{
	bActiveInput = NewValue;
}

FETexture* FEVirtualUIComponent::GetCurrentFrameTexture() const
{
	return Framebuffer->GetColorAttachment();
}

void FEVirtualUIComponent::AddOnMouseButtonCallback(std::function<void(int, int, int)> UserOnMouseButtonCallback)
{
	VirtualUI->AddOnMouseButtonCallback(UserOnMouseButtonCallback);
}

void FEVirtualUIComponent::InvokeMouseButton(int Button, int Action, int Mods)
{
	if (bActiveInput == false)
		return;

	VirtualUI->InvokeMouseButton(Button, Action, Mods);
}

void FEVirtualUIComponent::AddOnMouseMoveCallback(std::function<void(double, double)> UserOnMouseMoveCallback)
{
	VirtualUI->AddOnMouseMoveCallback(UserOnMouseMoveCallback);
}

void FEVirtualUIComponent::InvokeMouseMove(double Xpos, double Ypos)
{
	if (bActiveInput == false)
		return;

	VirtualUI->InvokeMouseMove(Xpos, Ypos);
}

void FEVirtualUIComponent::AddOnResizeCallback(std::function<void(int, int)> UserOnResizeCallback)
{
	VirtualUI->AddOnResizeCallback(UserOnResizeCallback);
}

void FEVirtualUIComponent::InvokeResize(int Width, int Height)
{
	if (Width <= 0 || Height <= 0)
	{
		LOG.Add("Invalid width or height in FEVirtualUIComponent::InvokeResize", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	delete Framebuffer;
	Framebuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, Width, Height, false);

	VirtualUI->InvokeResize(Framebuffer->FBO, Width, Height);
}

void FEVirtualUIComponent::AddOnCharCallback(std::function<void(unsigned int)> UserOnCharCallback)
{
	VirtualUI->AddOnCharCallback(UserOnCharCallback);
}

void FEVirtualUIComponent::InvokeCharInput(unsigned int Codepoint)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeCharInput(Codepoint);
}

void FEVirtualUIComponent::AddOnKeyCallback(std::function<void(int, int, int, int)> UserOnKeyCallback)
{
	VirtualUI->AddOnKeyCallback(UserOnKeyCallback);
}

void FEVirtualUIComponent::InvokeKeyInput(const int Key, const int Scancode, const int Action, const int Mods)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeKeyInput(Key, Scancode, Action, Mods);
}

void FEVirtualUIComponent::AddOnScrollCallback(std::function<void(double, double)> UserOnScrollCallback)
{
	VirtualUI->AddOnScrollCallback(UserOnScrollCallback);
}

void FEVirtualUIComponent::InvokeScrollInput(const double Xoffset, const double Yoffset)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeScrollInput(Xoffset, Yoffset);
}

void FEVirtualUIComponent::AddOnDropCallback(std::function<void(int, const char**)> UserOnDropCallback)
{
	VirtualUI->AddOnDropCallback(UserOnDropCallback);
}

void FEVirtualUIComponent::InvokeDropInput(const int Count, const char** Paths)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeDropInput(Count, Paths);
}

void FEVirtualUIComponent::MouseButtonListener(int Button, int Action, int Mods)
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

bool FEVirtualUIComponent::IsMouseButtonPassThroughActive() const
{
	return bMouseButtonPassThrough;
}

bool FEVirtualUIComponent::SetMouseButtonPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetMouseButtonPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bMouseButtonPassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::AddOnMouseEnterCallback(std::function<void(int)> UserOnMouseEnterCallback)
{
	VirtualUI->AddOnMouseEnterCallback(UserOnMouseEnterCallback);
}

void FEVirtualUIComponent::InvokeMouseEnterCallback(int Entered)
{
	if (!bActiveInput)
		return;

	VirtualUI->InvokeMouseEnterCallback(Entered);
}

bool FEVirtualUIComponent::InteractionRayToCanvasSpace(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, glm::vec2* IntersectionPointInUVCanvasSpace, glm::vec3* IntersectionPointIn3DSpace)
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

FEAABB FEVirtualUIComponent::GetAABB() const
{
	FEAABB Result;

	if (CanvasMesh == nullptr)
		return Result;

	Result = CanvasMesh->GetAABB();

	if (ParentEntity == nullptr)
		return Result;
	
	Result = Result.Transform(ParentEntity->GetComponent<FETransformComponent>().GetWorldMatrix());

	return Result;
}

void FEVirtualUIComponent::MouseMoveListener(double Xpos, double Ypos)
{
	// Not useing bRayColidingWithCanvas here because we want to call UpdateRayIntersection() to determine bRayColidingWithCanvas value.
	if (!bActiveInput || !bMouseMovePassThrough)
		return;

	UpdateRayIntersection();
}

void FEVirtualUIComponent::UpdateRayIntersection()
{
	glm::vec2 HitUV = glm::vec2(0.0f);
	if (InteractionRayToCanvasSpace(InteractionRayOrigin, InteractionRayDirection, &HitUV))
		InvokeMouseMove(HitUV[0] * GetWidth(), (1.0 - HitUV[1]) * GetHeight());
}

bool FEVirtualUIComponent::IsMouseMovePassThroughActive() const
{
	return bMouseMovePassThrough;
}

bool FEVirtualUIComponent::SetMouseMovePassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetMouseMovePassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bMouseMovePassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::CharListener(unsigned int Codepoint)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bCharPassThrough)
		return;

	InvokeCharInput(Codepoint);
}

bool FEVirtualUIComponent::IsCharPassThroughActive() const
{
	return bCharPassThrough;
}

bool FEVirtualUIComponent::SetCharPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetCharPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bCharPassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::KeyListener(int Key, int Scancode, int Action, int Mods)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bKeyPassThrough)
		return;

	InvokeKeyInput(Key, Scancode, Action, Mods);
}

bool FEVirtualUIComponent::IsKeyPassThroughActive() const
{
	return bKeyPassThrough;
}

bool FEVirtualUIComponent::SetKeyPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetKeyPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bKeyPassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::DropListener(int Count, const char** Paths)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bDropPassThrough)
		return;

	InvokeDropInput(Count, Paths);
}

bool FEVirtualUIComponent::IsDropPassThroughActive() const
{
	return bDropPassThrough;
}

bool FEVirtualUIComponent::SetDropPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetDropPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bDropPassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::ScrollListener(double Xoffset, double Yoffset)
{
	if (!bActiveInput || !bRayColidingWithCanvas || !bScrollPassThrough)
		return;

	InvokeScrollInput(Xoffset, Yoffset);
}

bool FEVirtualUIComponent::IsScrollPassThroughActive() const
{
	return bScrollPassThrough;
}

bool FEVirtualUIComponent::SetScrollPassThrough(bool NewValue)
{
	if (WindowToListen == nullptr)
	{
		LOG.Add("No window to listen in FEVirtualUIComponent::SetScrollPassThrough", "FE_LOG_INPUT", FE_LOG_ERROR);
		return false;
	}

	bScrollPassThrough = NewValue;
	return true;
}

void FEVirtualUIComponent::RegisterCallbacksForWindow()
{
	if (WindowToListen == nullptr)
		return;

	auto MouseButtonCallbackToRegister = std::bind(&FEVirtualUIComponent::MouseButtonListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	MouseButtonListenerID = WindowToListen->AddOnMouseButtonCallback(MouseButtonCallbackToRegister);

	auto MouseMoveCallbackToRegister = std::bind(&FEVirtualUIComponent::MouseMoveListener, this, std::placeholders::_1, std::placeholders::_2);
	MouseMoveListenerID = WindowToListen->AddOnMouseMoveCallback(MouseMoveCallbackToRegister);

	auto CharCallbackToRegister = std::bind(&FEVirtualUIComponent::CharListener, this, std::placeholders::_1);
	CharListenerID = WindowToListen->AddOnCharCallback(CharCallbackToRegister);

	auto KeyCallbackToRegister = std::bind(&FEVirtualUIComponent::KeyListener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	KeyListenerID = WindowToListen->AddOnKeyCallback(KeyCallbackToRegister);

	auto DropCallbackToRegister = std::bind(&FEVirtualUIComponent::DropListener, this, std::placeholders::_1, std::placeholders::_2);
	DropListenerID = WindowToListen->AddOnDropCallback(DropCallbackToRegister);

	auto ScrollCallbackToRegister = std::bind(&FEVirtualUIComponent::ScrollListener, this, std::placeholders::_1, std::placeholders::_2);
	ScrollListenerID = WindowToListen->AddOnScrollCallback(ScrollCallbackToRegister);
}

void FEVirtualUIComponent::UnregisterCallbacksForWindow()
{
	if (WindowToListen == nullptr)
		return;

	WindowToListen->RemoveCallback(MouseButtonListenerID);
	WindowToListen->RemoveCallback(MouseMoveListenerID);
	WindowToListen->RemoveCallback(CharListenerID);
	WindowToListen->RemoveCallback(KeyListenerID);
	WindowToListen->RemoveCallback(DropListenerID);
	WindowToListen->RemoveCallback(ScrollListenerID);
}

void FEVirtualUIComponent::SetWindowToListen(FEWindow* Window)
{
	if (WindowToListen != nullptr && WindowToListen != Window)
		UnregisterCallbacksForWindow();

	WindowToListen = Window;
	RegisterCallbacksForWindow();
}
FEWindow* FEVirtualUIComponent::GetWindowToListen() const
{
	return WindowToListen;
}

bool FEVirtualUIComponent::IsVisible() const
{
	//return CanvasEntity->GetComponent<FEGameModelComponent>().IsVisible();
	return true;
}

void FEVirtualUIComponent::SetVisibility(bool NewValue)
{
	//CanvasEntity->GetComponent<FEGameModelComponent>().SetVisibility(NewValue);
}

void FEVirtualUIComponent::ExecuteFunctionToAddFont(std::function<void()> Func, std::function<void()> CallbackOnFontReady)
{
	VirtualUI->ExecuteFunctionToAddFont(Func, CallbackOnFontReady);
}