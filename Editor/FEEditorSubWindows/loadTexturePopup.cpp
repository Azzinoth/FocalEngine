#include "loadTexturePopup.h"

loadTexturePopUp* loadTexturePopUp::_instance = nullptr;

loadTexturePopUp::loadTexturePopUp()
{
	popupCaption = "Choose Texture Type";
}

void loadTexturePopUp::show(std::string FilePath)
{
	shouldOpen = true;
	filePath = FilePath;
}

void loadTexturePopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text("Does this texture uses alpha channel ?");
		PROJECT_MANAGER.getCurrent()->modified = true;

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
		if (ImGui::Button("Yes", ImVec2(120, 0)))
		{
			FETexture* newTexture = RESOURCE_MANAGER.LoadPNGTexture(filePath.c_str(), true);
			newTexture->setDirtyFlag(true);
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("No", ImVec2(120, 0)))
		{
			FETexture* newTexture = RESOURCE_MANAGER.LoadPNGTexture(filePath.c_str(), false);
			newTexture->setDirtyFlag(true);
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