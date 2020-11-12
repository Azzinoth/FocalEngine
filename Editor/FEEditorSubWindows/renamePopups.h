#pragma once

#include "../Editor/FEEditorSubWindows/deletePopups.h"

class renameFailedPopUp : public ImGuiModalPopup
{
	ImGuiButton* okButton = nullptr;
public:
	renameFailedPopUp()
	{
		popupCaption = "Invalid name";
		okButton = new ImGuiButton("OK");
		okButton->setDefaultColor(ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		okButton->setHoveredColor(ImVec4(0.95f, 0.90f, 0.0f, 1.0f));
		okButton->setPosition(ImVec2(0, 0));
	}

	~renameFailedPopUp()
	{
		if (okButton != nullptr)
			delete okButton;
	}

	void show()
	{
		shouldOpen = true;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Entered name is occupied!");

			okButton->setPosition(ImVec2(ImGui::GetWindowContentRegionWidth() / 2.0f - 120.0f / 2.0f, 60));
			okButton->render();
			if (okButton->getWasClicked())
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static renameFailedPopUp renameFailedWindow;

class renameTexturePopUp : public ImGuiModalPopup
{
	FETexture* objToWorkWith;
	char newName[512];
public:
	renameTexturePopUp()
	{
		popupCaption = "Rename Texture";
		objToWorkWith = nullptr;
	}

	void show(FETexture* TextureToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = TextureToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			ImGui::Text("New texture name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (RESOURCE_MANAGER.setTextureName(objToWorkWith, newName))
				{
					objToWorkWith->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameTexturePopUp renameTextureWindow;

class renameMeshPopUp : public ImGuiModalPopup
{
	FEMesh* objToWorkWith;
	char newName[512];
public:
	renameMeshPopUp()
	{
		popupCaption = "Rename Mesh";
		objToWorkWith = nullptr;
	}

	void show(FEMesh* MeshToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = MeshToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			ImGui::Text("New mesh name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (RESOURCE_MANAGER.setMeshName(objToWorkWith, newName))
				{
					objToWorkWith->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;

					FETexture* tempTexture = PREVIEW_MANAGER.meshPreviewTextures[oldName];
					PREVIEW_MANAGER.meshPreviewTextures.erase(oldName);
					PREVIEW_MANAGER.meshPreviewTextures[newName] = tempTexture;

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameMeshPopUp renameMeshWindow;

class renameMaterialPopUp : public ImGuiModalPopup
{
	FEMaterial* objToWorkWith;
	char newName[512];
public:
	renameMaterialPopUp()
	{
		popupCaption = "Rename material";
		objToWorkWith = nullptr;
	}

	void show(FEMaterial* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			ImGui::Text("New material name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120 / 2.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (RESOURCE_MANAGER.setMaterialName(objToWorkWith, newName))
				{
					PROJECT_MANAGER.getCurrent()->modified = true;
					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120 / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameMaterialPopUp renameMaterialWindow;

class renameGameModelPopUp : public ImGuiModalPopup
{
	FEGameModel* objToWorkWith;
	char newName[512];
public:
	renameGameModelPopUp()
	{
		popupCaption = "Rename game model";
		objToWorkWith = nullptr;
	}

	void show(FEGameModel* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			ImGui::Text("New game model name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (RESOURCE_MANAGER.setGameModelName(objToWorkWith, newName))
				{
					PROJECT_MANAGER.getCurrent()->modified = true;
					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameGameModelPopUp renameGameModelWindow;

class renameEntityPopUp : public ImGuiModalPopup
{
	FEEntity* objToWorkWith;
	char newName[512];
public:
	renameEntityPopUp()
	{
		popupCaption = "Rename entity";
		objToWorkWith = nullptr;
	}

	void show(FEEntity* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
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

			FEScene& scene = FEScene::getInstance();
			ImGui::Text("New entity name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (scene.setEntityName(objToWorkWith, newName))
				{
					PROJECT_MANAGER.getCurrent()->modified = true;
					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameEntityPopUp renameEntityWindow;