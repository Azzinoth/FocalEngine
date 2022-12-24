#include "resizeTexturePopup.h"

resizeTexturePopup* resizeTexturePopup::Instance = nullptr;

resizeTexturePopup::resizeTexturePopup()
{
	PopupCaption = "Resize texture";
	ObjToWorkWith = nullptr;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	CancelButton->SetHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	CancelButton->SetActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

	ApplyButton = new ImGuiButton("Apply");
}

resizeTexturePopup::~resizeTexturePopup()
{
	delete CancelButton;
	delete ApplyButton;
}

void resizeTexturePopup::Show(FETexture* ObjToWorkWith)
{
	if (ObjToWorkWith == nullptr)
		return;
	bShouldOpen = true;
	this->ObjToWorkWith = ObjToWorkWith;

	NewWidth = ObjToWorkWith->GetWidth();
	NewHeight = ObjToWorkWith->GetHeight();
}

void resizeTexturePopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::SetNextWindowSize(PopupSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGui::EndPopup();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.GetWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));

		ImGui::SetCursorPos(ImVec2(20, 45));
		ImGui::Text("Width:");
		ImGui::SetCursorPos(ImVec2(85, 40));
		ImGui::SetNextItemWidth(80);
		ImGui::DragInt("##Width", &NewWidth);

		if (NewWidth < 4)
			NewWidth = 4;

		if (NewWidth > 8196)
			NewWidth = 8196;

		ImGui::SetCursorPos(ImVec2(190, 45));
		ImGui::Text("Height:");
		ImGui::SetCursorPos(ImVec2(265, 40));
		ImGui::SetNextItemWidth(80);
		ImGui::DragInt("##Height", &NewHeight);

		if (NewHeight < 4)
			NewHeight = 4;

		if (NewHeight > 8196)
			NewHeight = 8196;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
		ImGui::Text("Where to put resized texture data:");

		static int mode = 0;
		ImGui::RadioButton("Replace currect texture.", &mode, 0);
		ImGui::RadioButton("Create new texture.", &mode, 1);
		bReplaceTexture = mode == 0 ? true : false;

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

		ApplyButton->SetPosition(ImVec2(PopupSize.x / 4 - ApplyButton->GetSize().x / 2, PopupSize.y - 35));
		ApplyButton->Render();
		if (ApplyButton->IsClicked())
		{
			if (bReplaceTexture)
			{
				RESOURCE_MANAGER.ResizeTexture(ObjToWorkWith, NewWidth, NewHeight, 0/*filtration*/);
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(ObjToWorkWith);
			}
			else
			{
				unsigned char* result = RESOURCE_MANAGER.ResizeTextureRawData(ObjToWorkWith, NewWidth, NewHeight, 0/*filtration*/);
				FETexture* NewTexture = RESOURCE_MANAGER.RawDataToFETexture(result, NewWidth, NewHeight);
				VIRTUAL_FILE_SYSTEM.CreateFile(NewTexture, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTexture);
				delete[] result;
			}

			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		CancelButton->SetPosition(ImVec2(PopupSize.x / 2 + PopupSize.x / 4 - CancelButton->GetSize().x / 2, PopupSize.y - 35));
		CancelButton->Render();
		if (CancelButton->IsClicked())
		{
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}