#include "Editor\FEEditorSubWindows\renamePopups.h"

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
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
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
