#pragma once

#include "../Editor/FEEditorSubWindows/shaderEditorWindow.h"

class gyzmosSettingsWindow : public FEImGuiWindow
{
	ImGuiImageButton* TransformationGizmoButton = nullptr;
	ImGuiImageButton* ScaleGizmoButton = nullptr;
	ImGuiImageButton* RotateGizmoButton = nullptr;
public:
	gyzmosSettingsWindow()
	{
		Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
	}

	~gyzmosSettingsWindow() override
	{
		delete TransformationGizmoButton;
		delete ScaleGizmoButton;
		delete RotateGizmoButton;
	}

	void Show() override
	{
		const std::string TempCaption = "##GizmosSettingsWindow";
		strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());
		Size = ImVec2(146.0f, 48.0f);
		Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2.0f - 150.0f / 2.0f, 10.0f);
		FEImGuiWindow::Show();

		if (TransformationGizmoButton == nullptr)
			TransformationGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.TransformationGizmoIcon);

		if (ScaleGizmoButton == nullptr)
			ScaleGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.ScaleGizmoIcon);

		if (RotateGizmoButton == nullptr)
			RotateGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.RotateGizmoIcon);

		float CurrentX = 5.0f;
		TransformationGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
		CurrentX += 32.0f + 16.0f;
		ScaleGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
		CurrentX += 32.0f + 16.0f;
		RotateGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
	}

	void Render() override
	{
		FEImGuiWindow::Render();

		if (!IsVisible())
			return;

		GIZMO_MANAGER.GizmosState == TRANSFORM_GIZMOS ? SetSelectedStyle(TransformationGizmoButton) : SetDefaultStyle(TransformationGizmoButton);
		ImGui::PushID(0);
		TransformationGizmoButton->Render();
		if (TransformationGizmoButton->IsClicked())
		{
			if (GIZMO_MANAGER.GizmosState != TRANSFORM_GIZMOS)
			{
				GIZMO_MANAGER.UpdateGizmoState(TRANSFORM_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::OnRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		ShowToolTip("Translate objects. key = shift");

		GIZMO_MANAGER.GizmosState == SCALE_GIZMOS ? SetSelectedStyle(ScaleGizmoButton) : SetDefaultStyle(ScaleGizmoButton);
		ImGui::PushID(1);
		ScaleGizmoButton->Render();
		if (ScaleGizmoButton->IsClicked())
		{
			if (GIZMO_MANAGER.GizmosState != SCALE_GIZMOS)
			{
				GIZMO_MANAGER.UpdateGizmoState(SCALE_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::OnRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		ShowToolTip("Scale objects. key = shift");

		GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS ? SetSelectedStyle(RotateGizmoButton) : SetDefaultStyle(RotateGizmoButton);
		ImGui::PushID(2);
		RotateGizmoButton->Render();
		if (RotateGizmoButton->IsClicked())
		{
			if (GIZMO_MANAGER.GizmosState != ROTATE_GIZMOS)
			{
				GIZMO_MANAGER.UpdateGizmoState(ROTATE_GIZMOS);
				ImGui::PopID();
				FEImGuiWindow::OnRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		ShowToolTip("Rotate objects. key = shift");

		FEImGuiWindow::OnRenderEnd();
	}
};
static gyzmosSettingsWindow GyzmosSettingsWindowObject;