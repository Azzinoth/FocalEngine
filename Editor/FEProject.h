#pragma once

#include "../FEngine.h"
#include "../Editor/FEEditorDragAndDropManager.h"
#include "FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../Editor/FEEditorGizmoManager.h"
#include "../Editor/FEEditorPreviewManager.h"
#include "FEEditorInternalResources.h"
#include "../Editor/FEEditorVirtualFileSystem.h"

using namespace FocalEngine;

class FEProject
{
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	std::string getName();
	void setName(std::string newName);

	std::string getProjectFolder();
	void saveScene();
	void loadScene();
	void loadSceneVer0();
	
	FETexture* sceneScreenshot;
	bool modified = false;
	void createDummyScreenshot();

	void addFileToDeleteList(std::string fileName);
private:
	std::string name;
	std::string projectFolder;

	void writeTransformToJSON(Json::Value& root, FETransformComponent* transform);
	void readTransformToJSON(Json::Value& root, FETransformComponent* transform);

	std::vector<std::string> filesToDelete;
};

#define PROJECTS_FOLDER "C:/Users/Kindr/Desktop/FocalEngine/FEProjects"
#define PROJECTS_FILE_VER 0.01f

class FEProjectManager
{
	std::vector<FEProject*> list;
	int indexChosen = -1;
	FEProject* current = nullptr;
public:
	SINGLETON_PUBLIC_PART(FEProjectManager)
	SINGLETON_PRIVATE_PART(FEProjectManager)

	void initializeResources();

	FEProject* getCurrent();
	void setCurrent(FEProject* project);

	std::vector<FEProject*> getList();

	void openProject(int projectIndex);
	void loadProjectList();
	void closeCurrentProject();

	void displayProjectSelection();
};

#define PROJECT_MANAGER FEProjectManager::getInstance()