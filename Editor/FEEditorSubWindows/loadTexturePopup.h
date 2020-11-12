#pragma once

#include "../Editor/FEEditorSubWindows/gyzmosSettingsWindow.h"

class loadTexturePopUp : public ImGuiModalPopup
{
	std::string filePath;
public:
	loadTexturePopUp()
	{
		popupCaption = "Choose Texture Type";
	}

	void show(std::string FilePath)
	{
		shouldOpen = true;
		filePath = FilePath;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Does this texture uses alpha channel ?");
			PROJECT_MANAGER.getCurrent()->modified = true;

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				FETexture* newTexture = RESOURCE_MANAGER.LoadPNGTexture(filePath.c_str(), true);
				newTexture->setDirtyFlag(true);
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				FETexture* newTexture = RESOURCE_MANAGER.LoadPNGTexture(filePath.c_str(), false);
				newTexture->setDirtyFlag(true);
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static loadTexturePopUp loadTextureWindow;