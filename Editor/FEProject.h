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

	std::string GetName();
	void SetName(std::string NewValue);

	std::string GetProjectFolder();
	void SaveScene(bool bFullSave = false);
	void SaveSceneTo(std::string NewPath);
	void LoadScene();
	void LoadSceneVer0();

	FETexture* SceneScreenshot;
	void CreateDummyScreenshot();

	void AddFileToDeleteList(std::string FileName);

	bool IsModified();
	void SetModified(bool NewValue);

	void AddUnSavedObject(FEObject* Object);
private:
	std::string Name;
	std::string ProjectFolder;
	bool bModified = false;
	std::vector<FEObject*> UnSavedObjects;

	void WriteTransformToJson(Json::Value& Root, const FETransformComponent* Transform);
	void ReadTransformToJson(Json::Value& Root, FETransformComponent* Transform);

	std::vector<std::string> FilesToDelete;

	bool ShouldIncludeInSceneFile(const FETexture* Texture);
	void SetProjectFolder(std::string NewValue);
};

//#define PROJECTS_FOLDER "../FocalEngineProjects"
//#define PROJECTS_FOLDER "C:/Users/Kindr/Downloads/FocalEngineProjects-master"
//#define PROJECTS_FOLDER "C:/Users/Anastasiia/Desktop/FocalEngine/FEProjects"
#define PROJECTS_FILE_VER 0.02f

class FEProjectManager
{
	std::vector<FEProject*> List;
	int IndexChosen = -1;
	FEProject* Current = nullptr;
	std::string CustomProjectFolder = "../FocalEngineProjects-master";
public:
	SINGLETON_PUBLIC_PART(FEProjectManager)
	SINGLETON_PRIVATE_PART(FEProjectManager)

	void InitializeResources();

	FEProject* GetCurrent();
	void SetCurrent(FEProject* Project);

	std::vector<FEProject*> GetList();

	void OpenProject(int ProjectIndex);
	void LoadProjectList();
	void CloseCurrentProject();

	void DisplayProjectSelection();

	bool ContainProject(std::string Path);
	void SetProjectsFolder(std::string FolderPath);
};

#define PROJECTS_FOLDER PROJECT_MANAGER.CustomProjectFolder.c_str()
#define PROJECT_MANAGER FEProjectManager::getInstance()

static const char* const BASIC_SCENE = R"(
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