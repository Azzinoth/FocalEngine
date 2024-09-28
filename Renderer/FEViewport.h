#pragma once
#include "FEFramebuffer.h"

namespace FocalEngine
{
	enum FEViewportType
	{
		FE_VIEWPORT_NULL = 0,
		FE_VIEWPORT_OS_WINDOW = 1,
		FE_VIEWPORT_GLFW_WINDOW = 2,
		FE_VIEWPORT_FEWINDOW = 3,
		FE_VIEWPORT_IMGUI_WINDOW = 4
	};

	class FEViewport
	{
		friend class FERenderer;
		friend class FEngine;
		friend class FECameraSystem;
		friend struct FECameraComponent;

		std::string ID = "";

		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;

		FEViewportType Type = FE_VIEWPORT_NULL;
		void* WindowHandle = nullptr;

		FEViewport();
	public:
		std::string GetID() const;
		int GetX() const;
		int GetY() const;
		int GetWidth() const;
		int GetHeight() const;
	};
}