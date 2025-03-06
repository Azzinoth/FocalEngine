#pragma once
#include "FEFramebuffer.h"

namespace FocalEngine
{
	enum FEViewportType
	{
		FE_VIEWPORT_VIRTUAL = 0,
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

		FEViewport();

		std::string ID = "";

		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;

		void SetWidth(const int NewWidth);
		void SetHeight(const int NewHeight);

		FEViewportType Type = FE_VIEWPORT_VIRTUAL;
		void* WindowHandle = nullptr;
	public:
		std::string GetID() const;

		FEViewportType GetType() const;

		int GetX() const;
		int GetY() const;

		int GetWidth() const;
		int GetHeight() const;
	};
}