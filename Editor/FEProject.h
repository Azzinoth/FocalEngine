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
	void saveScene(bool fullSave = false);
	void saveSceneTo(std::string newPath);
	void loadScene();
	void loadSceneVer0();

	FETexture* sceneScreenshot;
	void createDummyScreenshot();

	void addFileToDeleteList(std::string fileName);

	bool isModified();
	void setModified(bool newValue);

	void addUnSavedObject(FEObject* object);
private:
	std::string name;
	std::string projectFolder;
	bool modified = false;
	std::vector<FEObject*> unSavedObjects;

	void writeTransformToJSON(Json::Value& root, FETransformComponent* transform);
	void readTransformToJSON(Json::Value& root, FETransformComponent* transform);

	std::vector<std::string> filesToDelete;

	bool shouldIncludeInSceneFile(FETexture* texture);
	void setProjectFolder(std::string newValue);
};

//#define PROJECTS_FOLDER "../FocalEngineProjects"
//#define PROJECTS_FOLDER "C:/Users/Kindr/Downloads/FocalEngineProjects-master"
//#define PROJECTS_FOLDER "C:/Users/Anastasiia/Desktop/FocalEngine/FEProjects"
#define PROJECTS_FILE_VER 0.02f

class FEProjectManager
{
	std::vector<FEProject*> list;
	int indexChosen = -1;
	FEProject* current = nullptr;
	std::string customProjectFolder = "../FocalEngineProjects-master";
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

	bool containProject(std::string path);
	void setProjectsFolder(std::string folderPath);
};

#define PROJECTS_FOLDER PROJECT_MANAGER.customProjectFolder.c_str()
#define PROJECT_MANAGER FEProjectManager::getInstance()

static const char* const basicScene = R"(
	{
	"camera" : 
	{
		"aspectRatio" : 2.1216442584991455,
		"farPlane" : 5000.0,
		"fov" : 70.0,
		"movementSpeed" : 10.0,
		"nearPlane" : 0.10000000149011612,
		"pitch" : 0.0,
		"position" : 
		{
			"X" : 0.0,
			"Y" : 0.0,
			"Z" : 0.0
		},
		"roll" : 0.0,
		"yaw" : 0.0
	},
	"effects" : 
	{
		"Anti-Aliasing_FXAA_" : 
		{
			"FXAAReduceMin" : 0.0078125,
			"FXAAReduceMul" : 0.40000000596046448,
			"FXAASpanMax" : 8.0
		},
		"Bloom" : 
		{
			"BloomSize" : 5.0,
			"thresholdBrightness" : 1.0
		},
		"Chromatic Aberration" : 
		{
			"Shift strength" : 1.0
		},
		"Depth of Field" : 
		{
			"Distance dependent strength" : 100.0,
			"Far distance" : 9000.0,
			"Near distance" : 0.0,
			"Strength" : 2.0
		},
		"Distance fog" : 
		{
			"Density" : 0.0070000002160668373,
			"Gradient" : 2.5
		},
		"Gamma Correction & Exposure" : 
		{
			"Exposure" : 1.0,
			"Gamma" : 2.2000000476837158
		},
		"Sky" : 
		{
			"Enabled" : 0.0,
			"Sphere size" : 50.0
		}
	},
	"entities" : null,
	"gameModels" : null,
	"lights" : 
	{
		"732E010E566C183C7968160F" : 
		{
			"CSM" : 
			{
				"CSMXYDepth" : 1.0,
				"CSMZDepth" : 3.0,
				"activeCascades" : 4,
				"shadowCoverage" : 50.0
			},
			"ID" : "732E010E566C183C7968160F",
			"castShadows" : true,
			"color" : 
			{
				"B" : 1.0,
				"G" : 1.0,
				"R" : 1.0
			},
			"direction" : 
			{
				"X" : 0.0,
				"Y" : 0.0,
				"Z" : -1.0
			},
			"enabled" : true,
			"intensity" : 1.0,
			"name" : "sun",
			"shadowBias" : 0.0010000000474974513,
			"shadowBiasVariableIntensity" : 1.0,
			"shadowBlurFactor" : 1.0,
			"staticShadowBias" : false,
			"transformation" : 
			{
				"position" : 
				{
					"X" : 0.0,
					"Y" : 0.0,
					"Z" : 0.0
				},
				"rotation" : 
				{
					"X" : 0.0,
					"Y" : 0.0,
					"Z" : 0.0
				},
				"scale" : 
				{
					"X" : 1.0,
					"Y" : 1.0,
					"Z" : 1.0,
					"uniformScaling" : true
				}
			},
			"type" : 9
		}
	},
	"materials" : null,
	"meshes" : null,
	"terrains" : null,
	"textures" : null,
	"version" : 0.019999999552965164
}
)";