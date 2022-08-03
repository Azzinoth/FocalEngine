#include "projectWasModifiedPopUp.h"

projectWasModifiedPopUp* projectWasModifiedPopUp::Instance = nullptr;

projectWasModifiedPopUp::projectWasModifiedPopUp()
{
	PopupCaption = "Save project ?";
	ObjToWorkWith = nullptr;
}

void projectWasModifiedPopUp::Show(FEProject* Project, const bool FullyCloseApplication)
{
	bShouldOpen = true;
	ObjToWorkWith = Project;
	bShouldTerminate = FullyCloseApplication;
}

void projectWasModifiedPopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGui::EndPopup();
			return;
		}

		bool ButtonPressed = false;

		ImGui::SetWindowPos(ImVec2(ENGINE.GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.GetWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text("Project was modified, should it be saved before exit ?");
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.15f - 140.0f / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
		if (ImGui::Button("Save", ImVec2(140, 0)))
		{
			PROJECT_MANAGER.GetCurrent()->SaveScene();
			ENGINE.TakeScreenshot((PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + "projectScreenShot.texture").c_str());
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
			ButtonPressed = true;
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 140.0f / 2.0f);
		if (ImGui::Button("Exit without saving", ImVec2(140, 0)))
		{
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
			ButtonPressed = true;
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.85f - 140.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(140, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();

		if (ButtonPressed)
		{
			PROJECT_MANAGER.CloseCurrentProject();
			if (bShouldTerminate)
			{
				ENGINE.Terminate();
				return;
			}
		}
	}
	else
	{
		ImGui::PopStyleVar();
	}
}