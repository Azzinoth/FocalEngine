#pragma once

#include "../Editor/FEEditorSubWindows/loadTexturePopup.h"

static bool shouldTerminate = false;
class projectWasModifiedPopUp : public ImGuiModalPopup
{
	FEProject* objToWorkWith;
public:
	projectWasModifiedPopUp()
	{
		popupCaption = "Save project ?";
		objToWorkWith = nullptr;
	}

	void show(FEProject* project)
	{
		shouldOpen = true;
		objToWorkWith = project;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::PopStyleVar();
				ImGui::EndPopup();
				return;
			}

			bool buttonPressed = false;

			ImGui::Text("Project was modified, should it be saved before exit ?");
			ImVec2 sentenceSize = ImGui::CalcTextSize("Project was modified, should it be saved before exit ?");
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.15f - 140.0f / 2.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			if (ImGui::Button("Save", ImVec2(140, 0)))
			{
				PROJECT_MANAGER.getCurrent()->saveScene();
				ENGINE.takeScreenshot((PROJECT_MANAGER.getCurrent()->getProjectFolder() + "projectScreenShot.texture").c_str());
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
				buttonPressed = true;
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 140.0f / 2.0f);
			if (ImGui::Button("Exit without saving", ImVec2(140, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
				buttonPressed = true;
			}

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.85f - 140.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(140, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();

			if (buttonPressed)
			{
				PROJECT_MANAGER.closeCurrentProject();
				if (shouldTerminate)
				{
					ENGINE.terminate();
					return;
				}
			}
		}
		else
		{
			ImGui::PopStyleVar();
		}
	}
};
static projectWasModifiedPopUp projectWasModifiedPopUpWindow;