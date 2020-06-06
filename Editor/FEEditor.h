#pragma once

#include "FEProject.h"
#include <direct.h>

using namespace FocalEngine;

#define PROJECTS_FOLDER "C:/Users/kandr/Downloads/FEProjects"

static double mouseX, mouseY;
static bool isCameraInputActive = false;
static std::pair<std::string, float > entityUnderMouse;
static std::string selectedEntity = "";
static std::string clipboardEntity = "";
static bool selectedEntityWasChanged;
static std::vector<FEProject*> projectList;
static int projectChosen = -1;

static FEProject* currentProject = nullptr;

//test Delete this!
static FETexture* testTexture;

void determineEntityUnderMouse();
void mouseButtonCallback(int button, int action, int mods);
glm::dvec3 mouseRay();
void mouseMoveCallback(double xpos, double ypos);
void keyButtonCallback(int key, int scancode, int action, int mods);

void toolTip(const char* text);
void showPosition(std::string objectName, glm::vec3& position);
void showRotation(std::string objectName, glm::vec3& rotation);
void showScale(std::string objectName, glm::vec3& scale);
void showTransformConfiguration(std::string objectName, FETransformComponent* transform);

void displayMaterialPrameter(FEShaderParam* param);
void displayMaterialPrameters(FEMaterial* material);
void displayLightProperties(FELight* light);
void displayLightsProperties();
void displaySceneEntities();
void displayMaterialEditor();
void displayTextureInMaterialEditor(FETexture*& texture);

void displayContentBrowser();
void displayPostProcess();
void displayProjectSelection();

void addEntityButton();

// file system
bool checkFolder(const char* dirPath);
bool createFolder(const char* dirPath);
bool deleteFolder(const char* dirPath);
std::vector<std::string> getFolderList(const char* dirPath);
bool changeFileName(const char* filePath, const char* newName);
bool deleteFile(const char* filePath);
// file system END

void renderEditor();
void loadProjectList();
void loadEditor();


int timesTextureUsed(FETexture* texture);

class ImGuiModalPopup
{
protected:
	bool shouldOpen;
	bool opened;
	char* popupCaption;

	virtual void close()
	{
		opened = false;
		ImGui::CloseCurrentPopup();
	}
public:
	ImGuiModalPopup()
	{
		popupCaption = "";
		shouldOpen = false;
		opened = false;
	}

	virtual void show()
	{
		shouldOpen = true;
		opened = true;
	}

	virtual void render()
	{
		if (shouldOpen)
		{
			ImGui::OpenPopup(popupCaption);
			shouldOpen = false;
		}
	}

	bool isOpened()
	{
		return opened;
	}
};

class deleteTexturePopup : public ImGuiModalPopup
{
	FETexture* textureToWorkWith;
public:
	deleteTexturePopup()
	{
		popupCaption = "Delete texture";
		textureToWorkWith = nullptr;
	}

	void show(FETexture* TextureToDelete)
	{
		shouldOpen = true;
		textureToWorkWith = TextureToDelete;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (textureToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this texture is used in some materials
			// to-do: should be done through counter, not by searching each time.
			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			int result = timesTextureUsed(textureToWorkWith);
			
			ImGui::Text(("Do you want to delete \"" + textureToWorkWith->getName() + "\" texture ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " materials !").c_str());

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				deleteFile((currentProject->getProjectFolder() + textureToWorkWith->getName() + ".FEtexture").c_str());
				FEResourceManager::getInstance().deleteFETexture(textureToWorkWith);
				currentProject->saveScene();

				textureToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteTexturePopup deleteTextureWindow;

class renameFailedPopUp : public ImGuiModalPopup
{
public:
	renameFailedPopUp()
	{
		popupCaption = "Invalid texture name";
	}

	void show()
	{
		shouldOpen = true;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Entered texture name is occupied");

			ImGui::SetCursorPosX(60);
			ImGui::SetCursorPosY(50);
			if (ImGui::Button("OK", ImVec2(120, 0)))
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
	FETexture* textureToWorkWith;
public:
	renameTexturePopUp()
	{
		popupCaption = "Rename Texture";
		textureToWorkWith = nullptr;
	}

	void show(FETexture* TextureToWorkWith)
	{
		shouldOpen = true;
		textureToWorkWith = TextureToWorkWith;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (textureToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			static std::string currentName = "";
			if (currentName.size() == 0)
				currentName = textureToWorkWith->getName();

			ImGui::Text("New texture name :");
			static char newName[512] = "";

			if (strcmp(newName, "") == 0)
				strcpy_s(newName, currentName.size() + 1, currentName.c_str());

			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = textureToWorkWith->getName();
				// if new name is acceptable
				if (resourceManager.setTextureName(textureToWorkWith, newName))
				{
					// also rename texture filename correspondently
					changeFileName((currentProject->getProjectFolder() + oldName + ".FETexture").c_str(), (currentProject->getProjectFolder() + newName + ".FETexture").c_str());
					// save assets list with new texture name
					currentProject->saveScene();

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
					currentName = "";
				}
				else
				{
					textureToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				textureToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameTexturePopUp renameTextureWindow;

class loadTexturePopUp : public ImGuiModalPopup
{
	bool hasAlpha;
public:
	loadTexturePopUp()
	{
		popupCaption = "Load Texture";
	}

	void show()
	{
		shouldOpen = true;
		hasAlpha = false;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Insert texture file path :");
			static char filePath[512] = "";

			ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
			ImGui::Separator();

			ImGui::Checkbox("Texture uses alpha channel", &hasAlpha);

			if (ImGui::Button("Load", ImVec2(120, 0)))
			{
				FETexture* newTexture = FEResourceManager::getInstance().LoadPngTextureAndCompress(filePath, hasAlpha);
				FEResourceManager::getInstance().saveFETexture((currentProject->getProjectFolder() + newTexture->getName() + ".FETexture").c_str(), newTexture);
				// add asset list saving....
				currentProject->saveScene();

				ImGuiModalPopup::close();
				strcpy_s(filePath, "");
				hasAlpha = false;
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
				hasAlpha = false;
			}
			ImGui::EndPopup();
		}
	}
};
static loadTexturePopUp loadTextureWindow;

class selectTexturePopUp : public ImGuiModalPopup
{
	FETexture** textureToWorkWith;
	int textureIndexUnderMouse = -1;
	int textureIndexSelected = -1;
	bool pushedStyle = false;
	std::vector<std::string> textureList;
	std::vector<std::string> filteredTextureList;
	char filter[512];
public:
	selectTexturePopUp()
	{
		popupCaption = "Select texture";
	}

	void show(FETexture** texture)
	{
		shouldOpen = true;
		pushedStyle = false;
		textureToWorkWith = texture;
		textureList = FEResourceManager::getInstance().getTextureList();
		filteredTextureList = textureList;
		strcpy_s(filter, "");
	}

	void close() override
	{
		ImGuiModalPopup::close();
		textureIndexUnderMouse = -1;
		textureIndexSelected = -1;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredTextureList = textureList;
				}
				else
				{
					filteredTextureList.clear();
					for (size_t i = 0; i < textureList.size(); i++)
					{
						if (textureList[i].find(filter) != -1)
						{
							filteredTextureList.push_back(textureList[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectTexturePopupColumns", false);
			for (size_t i = 0; i < filteredTextureList.size(); i++)
			{
				pushedStyle = false;
				if (textureIndexSelected == i)
				{
					pushedStyle = true;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				}

				if (ImGui::ImageButton((void*)(intptr_t)FEResourceManager::getInstance().getTexture(filteredTextureList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					textureIndexSelected = i;
				}

				if (pushedStyle)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemHovered())
				{
					textureIndexUnderMouse = i;
				}

				ImGui::Text(filteredTextureList[i].c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(300);
			ImGui::SetCursorPosY(25);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			if (ImGui::Button("Select", ImVec2(140, 24)))
			{
				if (textureIndexSelected != -1)
				{
					*textureToWorkWith = FEResourceManager::getInstance().getTexture(filteredTextureList[textureIndexSelected]);
					close();
				}
			}

			ImGui::SetCursorPosX(460);
			ImGui::SetCursorPosY(25);

			if (ImGui::Button("Cancel", ImVec2(140, 24)))
			{
				close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}
};
static selectTexturePopUp selectTextureWindow;