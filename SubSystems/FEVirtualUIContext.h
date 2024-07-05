#include "FEVirtualUI.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../SubSystems/Scene/FENewEntity.h"

namespace FocalEngine
{
	// Not designed to work with canvases a lot more complex than a plane.
	class FEVirtualUIContext : public FEObject
	{
		friend class FEScene;
		friend class FERenderer;

		FEVirtualUIContext(int Width = 1280, int Height = 720, FEMesh* SampleMesh = nullptr, std::string Name = "UnNamed");
		~FEVirtualUIContext();

		FEVirtualUI* VirtualUI = nullptr;
		FEFramebuffer* Framebuffer = nullptr;

		// Canvas
		FEMesh* CanvasMesh = nullptr;
		FEMaterial* CanvasMaterial = nullptr;
		FEGameModel* CanvasGameModel = nullptr;
	public:
		FENewEntity* CanvasEntity = nullptr;
	private:

		// Mesh interaction
		std::vector<std::vector<glm::vec3>> OriginalMeshTrianglePositions;
		std::vector<std::vector<glm::vec3>> TransformedMeshTrianglePositions;
		std::vector<std::vector<glm::vec2>> MeshTriangleUVs;
		void UpdateCanvasTrianglePositions();

		glm::dvec3 InteractionRayOrigin = glm::dvec3(0.0);
		glm::dvec3 InteractionRayDirection = glm::dvec3(0.0);

		// Input
		bool bActiveInput = true;
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
	public:
		// Prevent copying and assignment
		FEVirtualUIContext(const FEVirtualUIContext&) = delete;
		FEVirtualUIContext& operator=(const FEVirtualUIContext&) = delete;

		void GetSize(int* Width, int* Height) const;
		void SetSize(int NewWidth, int NewHeight);
		int GetWidth() const;
		int GetHeight() const;

		glm::vec3 GetPosition() const;
		void SetPosition(glm::vec3 NewPosition);

		glm::vec3 GetRotation() const;
		void SetRotation(glm::vec3 NewRotation);

		glm::vec3 GetScale() const;
		void SetScale(glm::vec3 NewScale);

		bool IsVisible() const;
		void SetVisibility(bool NewValue);

		bool IsInputActive() const;
		void SetInputActive(bool NewValue);

		std::function<void()> GetRenderFunction();
		void SetRenderFunction(std::function<void()> UserRenderFunction);
		void ClearRenderFunction();

		FETexture* GetCurrentFrameTexture() const;

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
	};
}