#pragma once
#include "../Renderer/FEGameModel.h"

namespace FocalEngine
{
#define VIRTUAL_UI_RENDER_FUNCTION std::function<void(FEVirtualUI*)>
	struct FEVirtualUIComponent
	{
	private:
		friend class FEVirtualUISystem;
	public:
		FEVirtualUIComponent(int Width = 1024, int Height = 1024, FEMesh* CanvasMesh = nullptr);
		~FEVirtualUIComponent();

		// Prevent copying and assignment
		FEVirtualUIComponent(const FEVirtualUIComponent&) = delete;
		FEVirtualUIComponent& operator=(const FEVirtualUIComponent&) = delete;

		glm::vec2 GetCanvasResolution() const;
		void SetCanvasResolution(glm::vec2 NewResolution);
		int GetWidth() const;
		int GetHeight() const;

		bool IsVisible() const;
		void SetVisibility(bool NewValue);

		bool IsInputActive() const;
		void SetInputActive(bool NewValue);

		VIRTUAL_UI_RENDER_FUNCTION GetRenderFunction();
		void SetRenderFunction(VIRTUAL_UI_RENDER_FUNCTION UserRenderFunction);
		void ClearRenderFunction();

		FETexture* GetCurrentFrameTexture() const;

		FEMesh* GetCanvasMesh() const;
		void SetCanvasMesh(FEMesh* NewCanvasMesh);
		FEAABB GetAABB() const;

		// Event Handling
		void AddOnResizeCallback(std::function<void(int, int)> UserOnResizeCallback);
		void AddOnMouseEnterCallback(std::function<void(int)> UserOnMouseEnterCallback);
		void AddOnMouseButtonCallback(std::function<void(int, int, int)> UserOnMouseButtonCallback);
		void AddOnMouseMoveCallback(std::function<void(double, double)> UserOnMouseMoveCallback);
		void AddOnCharCallback(std::function<void(unsigned int)> UserOnCharCallback);
		void AddOnKeyCallback(std::function<void(int, int, int, int)> UserOnKeyCallback);
		void AddOnDropCallback(std::function<void(int, const char**)> UserOnDropCallback);
		void AddOnScrollCallback(std::function<void(double, double)> UserOnScrollCallback);

		// Functions that will be invoked by upper layers of abstraction to simulate events
		void InvokeResize(int Width, int Height);
		void InvokeMouseButton(int Button, int Action, int Mods = 0);
		void InvokeMouseMove(double Xpos, double Ypos);
		void InvokeCharInput(unsigned int codepoint);
		void InvokeKeyInput(int Key, int Scancode, int Action, int Mods);
		void InvokeDropInput(int Count, const char** Paths);
		void InvokeScrollInput(double Xoffset, double Yoffset);

		FEVirtualUI* GetVirtualUI() const;

		void SetWindowToListen(FEWindow* Window);
		FEWindow* GetWindowToListen() const;

		bool IsMouseButtonPassThroughActive() const;
		// To activate pass through, there should be a window to listen
		bool SetMouseButtonPassThrough(bool NewValue);

		bool IsMouseMovePassThroughActive() const;
		// To activate passthrough, there should be a window to listen
		bool SetMouseMovePassThrough(bool NewValue);

		bool IsCharPassThroughActive() const;
		// To activate passthrough, there should be a window to listen
		bool SetCharPassThrough(bool NewValue);

		bool IsKeyPassThroughActive() const;
		// To activate passthrough, there should be a window to listen
		bool SetKeyPassThrough(bool NewValue);

		bool IsDropPassThroughActive() const;
		// To activate passthrough, there should be a window to listen
		bool SetDropPassThrough(bool NewValue);

		bool IsScrollPassThroughActive() const;
		// To activate passthrough, there should be a window to listen
		bool SetScrollPassThrough(bool NewValue);

		// It should be used instead of mouse passthrough if input device is a VR controller
		void UpdateInteractionRay(glm::dvec3 RayOrigin, glm::dvec3 RayDirection);
		void UpdateRayIntersection();

		// Use that instead of directly adding fonts to ImGui
		// to ensure that font addition is done in the correct way.
		void ExecuteFunctionToAddFont(std::function<void()> Func, std::function<void()> CallbackOnFontReady);
	private:
		FEEntity* ParentEntity = nullptr;

		FEVirtualUI* VirtualUI = nullptr;
		FEFramebuffer* Framebuffer = nullptr;
		FEMesh* CanvasMesh = nullptr;

		// Mesh interaction
		std::vector<std::vector<glm::vec3>> OriginalMeshTrianglePositions;
		std::vector<std::vector<glm::vec3>> TransformedMeshTrianglePositions;
		std::vector<std::vector<glm::vec2>> MeshTriangleUVs;
		void UpdateCanvasTrianglePositions();

		glm::dvec3 InteractionRayOrigin = glm::dvec3(0.0);
		glm::dvec3 InteractionRayDirection = glm::dvec3(0.0);

		// Input
		bool bActiveInput = false;
		bool bRayColidingWithCanvas = false;
		FEWindow* WindowToListen = nullptr;
		void RegisterCallbacksForWindow();
		void UnregisterCallbacksForWindow();

		void MouseButtonListener(int Button, int Action, int Mods);
		bool bMouseButtonPassThrough = false;
		void MouseMoveListener(double Xpos, double Ypos);
		bool bMouseMovePassThrough = false;
		void CharListener(unsigned int Codepoint);
		bool bCharPassThrough = false;
		void KeyListener(int Key, int Scancode, int Action, int Mods);
		bool bKeyPassThrough = false;
		void DropListener(int Count, const char** Paths);
		bool bDropPassThrough = false;
		void ScrollListener(double Xoffset, double Yoffset);
		bool bScrollPassThrough = false;

		bool InteractionRayToCanvasSpace(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, glm::vec2* IntersectionPointInUVCanvasSpace, glm::vec3* IntersectionPointIn3DSpace = nullptr);
		void InvokeMouseEnterCallback(int Entered);
	};
}