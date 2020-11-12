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

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			bool buttonPressed = false;

			ImGui::Text("Project was modified, should it be saved before exit ?");
			ImVec2 sentenceSize = ImGui::CalcTextSize("Project was modified, should it be saved before exit ?");
			ImGui::SetCursorPosX(sentenceSize.x / 4.0f - 140.0f / 2.0f);
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
			ImGui::SetCursorPosX(sentenceSize.x / 2.0f + sentenceSize.x / 4.0f - 140.0f / 2.0f);
			if (ImGui::Button("Exit without saving", ImVec2(140, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
				buttonPressed = true;
			}
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
	}
};
static projectWasModifiedPopUp projectWasModifiedPopUpWindow;