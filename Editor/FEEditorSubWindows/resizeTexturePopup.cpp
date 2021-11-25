#include "resizeTexturePopup.h"

resizeTexturePopup* resizeTexturePopup::_instance = nullptr;

resizeTexturePopup::resizeTexturePopup()
{
	popupCaption = "Resize texture";
	objToWorkWith = nullptr;

	cancelButton = new ImGuiButton("Cancel");
	cancelButton->setDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	cancelButton->setHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	cancelButton->setActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

	applyButton = new ImGuiButton("Apply");
}

resizeTexturePopup::~resizeTexturePopup()
{
	if (cancelButton != nullptr)
		delete cancelButton;

	if (applyButton != nullptr)
		delete applyButton;
}

void resizeTexturePopup::show(FETexture* ObjToWorkWith)
{
	if (ObjToWorkWith == nullptr)
		return;
	shouldOpen = true;
	objToWorkWith = ObjToWorkWith;

	newWidth = ObjToWorkWith->getWidth();
	newHeight = ObjToWorkWith->getHeight();
}

void resizeTexturePopup::render()
{
	ImGuiModalPopup::render();

	ImGui::SetNextWindowSize(popupSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGui::EndPopup();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));

		ImGui::SetCursorPos(ImVec2(20, 45));
		ImGui::Text("Width:");
		ImGui::SetCursorPos(ImVec2(85, 40));
		ImGui::SetNextItemWidth(80);
		ImGui::DragInt("##Width", &newWidth);

		if (newWidth < 4)
			newWidth = 4;

		if (newWidth > 8196)
			newWidth = 8196;

		ImGui::SetCursorPos(ImVec2(190, 45));
		ImGui::Text("Height:");
		ImGui::SetCursorPos(ImVec2(265, 40));
		ImGui::SetNextItemWidth(80);
		ImGui::DragInt("##Height", &newHeight);

		if (newHeight < 4)
			newHeight = 4;

		if (newHeight > 8196)
			newHeight = 8196;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
		ImGui::Text("Where to put resized texture data:");

		static int mode = 0;
		ImGui::RadioButton("Replace currect texture.", &mode, 0);
		ImGui::RadioButton("Create new texture.", &mode, 1);
		replaceTexture = mode == 0 ? true : false;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
		ImGui::Text("Note: Only nearest-neighbor algorithm \navailable right now.");
		/*ImGui::SetCursorPos(ImVec2(20, 185));
		ImGui::Text("Filtration:");
		ImGui::SetCursorPos(ImVec2(140, 180));
		ImGui::SetNextItemWidth(60);
		ImGui::DragInt("##Filtration", &filtration);

		if (filtration < 0)
			filtration = 0;

		if (filtration > 16)
			filtration = 16;*/

		applyButton->setPosition(ImVec2(popupSize.x / 4 - applyButton->getSize().x / 2, popupSize.y - 35));
		applyButton->render();
		if (applyButton->getWasClicked())
		{
			if (replaceTexture)
			{
				RESOURCE_MANAGER.resizeTexture(objToWorkWith, newWidth, newHeight, 0/*filtration*/);
				PROJECT_MANAGER.getCurrent()->addUnSavedObject(objToWorkWith);
			}
			else
			{
				unsigned char* result = RESOURCE_MANAGER.resizeTextureRawData(objToWorkWith, newWidth, newHeight, 0/*filtration*/);
				FETexture* newTexture = RESOURCE_MANAGER.rawDataToFETexture(result, newWidth, newHeight);
				VIRTUAL_FILE_SYSTEM.createFile(newTexture, VIRTUAL_FILE_SYSTEM.getCurrentPath());
				PROJECT_MANAGER.getCurrent()->addUnSavedObject(newTexture);
				delete[] result;
			}

			objToWorkWith = nullptr;
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		cancelButton->setPosition(ImVec2(popupSize.x / 2 + popupSize.x / 4 - cancelButton->getSize().x / 2, popupSize.y - 35));
		cancelButton->render();
		if (cancelButton->getWasClicked())
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