#pragma once

#include "../Editor/FEEditorSubWindows/deletePopups.h"

class renameFailedPopUp : public ImGuiModalPopup
{
	ImGuiButton* okButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(renameFailedPopUp)
	void render() override;
private:
	SINGLETON_PRIVATE_PART(renameFailedPopUp)
};

class renamePopUp : public ImGuiModalPopup
{
	FEObject* objToWorkWith;
	char newName[512];
public:
	SINGLETON_PUBLIC_PART(renamePopUp)

	void show(FEObject* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			ImGui::Text("New object name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (strlen(newName) > 0)
				{
					objToWorkWith->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;

					objToWorkWith->setName(newName);

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedPopUp::getInstance().show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}
		else
		{
			ImGui::PopStyleVar();
		}
	}
private:
	SINGLETON_PRIVATE_PART(renamePopUp)
};