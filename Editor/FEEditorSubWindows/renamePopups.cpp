#include "renamePopups.h"

renameFailedPopUp* renameFailedPopUp::_instance = nullptr;

renameFailedPopUp::renameFailedPopUp()
{
	popupCaption = "Invalid name";
	okButton = new ImGuiButton("OK");
	okButton->setDefaultColor(ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	okButton->setHoveredColor(ImVec4(0.95f, 0.90f, 0.0f, 1.0f));
	okButton->setPosition(ImVec2(0, 0));
}

renameFailedPopUp::~renameFailedPopUp()
{
	if (okButton != nullptr)
		delete okButton;
}

void renameFailedPopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text("Entered name is incorrect !");

		okButton->setPosition(ImVec2(ImGui::GetWindowWidth() / 2.0f - 120.0f / 2.0f, ImGui::GetCursorPosY() + 10.0f));
		okButton->render();
		if (okButton->getWasClicked())
		{
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

renamePopUp* renamePopUp::_instance = nullptr;

renamePopUp::renamePopUp()
{
	popupCaption = "Rename object";
	objToWorkWith = nullptr;
}

void renamePopUp::show(FEObject* ObjToWorkWith)
{
	shouldOpen = true;
	objToWorkWith = ObjToWorkWith;
	strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
}

void renamePopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGui::EndPopup();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
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
