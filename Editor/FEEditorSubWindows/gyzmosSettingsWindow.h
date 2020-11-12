#pragma once

#include "../Editor/FEEditorSubWindows/shaderEditorWindow.h"
using namespace FEGizmoManager;

class gyzmosSettingsWindow : public FEImGuiWindow
{
	ImGuiImageButton* transformationGizmoButton = nullptr;
	ImGuiImageButton* scaleGizmoButton = nullptr;
	ImGuiImageButton* rotateGizmoButton = nullptr;
public:
	gyzmosSettingsWindow()
	{
		flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
	}

	~gyzmosSettingsWindow() override
	{
		if (transformationGizmoButton != nullptr)
			delete transformationGizmoButton;

		if (scaleGizmoButton != nullptr)
			delete scaleGizmoButton;

		if (rotateGizmoButton != nullptr)
			delete rotateGizmoButton;
	}

	void show() override
	{
		std::string tempCaption = "##GizmosSettingsWindow";
		strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
		size = ImVec2(146.0f, 48.0f);
		position = ImVec2(FEngine::getInstance().getWindowWidth() / 2.0f - 150.0f / 2.0f, 10.0f);
		FEImGuiWindow::show();

		if (transformationGizmoButton == nullptr)
			transformationGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.transformationGizmoIcon);

		if (scaleGizmoButton == nullptr)
			scaleGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.scaleGizmoIcon);

		if (rotateGizmoButton == nullptr)
			rotateGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.rotateGizmoIcon);

		float currentX = 5.0f;
		transformationGizmoButton->setPosition(ImVec2(currentX, 5.0f));
		currentX += 32.0f + 16.0f;
		scaleGizmoButton->setPosition(ImVec2(currentX, 5.0f));
		currentX += 32.0f + 16.0f;
		rotateGizmoButton->setPosition(ImVec2(currentX, 5.0f));
	}

	void render() override
	{
		FEImGuiWindow::render();

		if (!isVisible())
			return;

		GIZMO_MANAGER.gizmosState == TRANSFORM_GIZMOS ? setSelectedStyle(transformationGizmoButton) : setDefaultStyle(transformationGizmoButton);
		ImGui::PushID(0);
		transformationGizmoButton->render();
		if (transformationGizmoButton->getWasClicked())
		{
			if (GIZMO_MANAGER.gizmosState != TRANSFORM_GIZMOS)
			{
				GIZMO_MANAGER.updateGizmoState(TRANSFORM_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		showToolTip("Translate objects. key = shift");

		GIZMO_MANAGER.gizmosState == SCALE_GIZMOS ? setSelectedStyle(scaleGizmoButton) : setDefaultStyle(scaleGizmoButton);
		ImGui::PushID(1);
		scaleGizmoButton->render();
		if (scaleGizmoButton->getWasClicked())
		{
			if (GIZMO_MANAGER.gizmosState != SCALE_GIZMOS)
			{
				GIZMO_MANAGER.updateGizmoState(SCALE_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		showToolTip("Scale objects. key = shift");

		GIZMO_MANAGER.gizmosState == ROTATE_GIZMOS ? setSelectedStyle(rotateGizmoButton) : setDefaultStyle(rotateGizmoButton);
		ImGui::PushID(2);
		rotateGizmoButton->render();
		if (rotateGizmoButton->getWasClicked())
		{
			if (GIZMO_MANAGER.gizmosState != ROTATE_GIZMOS)
			{
				GIZMO_MANAGER.updateGizmoState(ROTATE_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		showToolTip("Rotate objects. key = shift");

		FEImGuiWindow::onRenderEnd();
	}
};
static gyzmosSettingsWindow gyzmosSettingsWindowObject;