#include "FENativeScriptProject.h"
#include <shellapi.h>
using namespace FocalEngine;

FENativeScriptProject::FENativeScriptProject()
{

}

void FENativeScriptProject::SetWorkingDirectory(std::string NewValue)
{
	if (NewValue.empty())
	{
		LOG.Add("FENativeScriptProject::SetWorkingDirectory: NewValue is empty!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::SetWorkingDirectory: Parent is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(NewValue))
	{
		LOG.Add("FENativeScriptProject::SetWorkingDirectory: NewValue does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	WorkingDirectory = NewValue;
	VSProjectDirectory = WorkingDirectory + "NativeScriptProjects/" + Parent->GetObjectID() + "/";
}

std::string FENativeScriptProject::GetWorkingDirectory()
{
	return WorkingDirectory;
}

std::string FENativeScriptProject::GetVSProjectDirectory()
{
	return VSProjectDirectory;
}

std::string FENativeScriptProject::GetVSProjectName()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::GetVSProjectName: Parent is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	std::string AppropriateProjectName = Parent->GetName();
	for (size_t i = 0; i < AppropriateProjectName.size(); i++)
	{
		if (AppropriateProjectName[i] == ' ')
			AppropriateProjectName[i] = '_';
	}

	if (AppropriateProjectName.empty())
		AppropriateProjectName = "UntitledProject";

	return AppropriateProjectName;
}

FENativeScriptProject::~FENativeScriptProject()
{
	if (DataToRecoverVSProject != nullptr)
		delete DataToRecoverVSProject;
}

bool FENativeScriptProject::Initialize(FEAssetPackage* Data)
{
	if (Data == nullptr)
	{
		LOG.Add("FENativeScriptProject::Initialize: Data is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	DataToRecoverVSProject = Data;

	std::vector<FEAssetPackageAssetInfo> SourceFilesList = DataToRecoverVSProject->GetEntryList();
	for (size_t i = 0; i < SourceFilesList.size(); i++)
	{
		SourceFileList.push_back(SourceFilesList[i].Name);
	}

	SetFileTracking();
	return true;
}

void FENativeScriptProject::UpdateDataToRecoverVSProject()
{
	if (DataToRecoverVSProject == nullptr)
	{
		if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
		{
			LOG.Add("FENativeScriptProject::Save: VSProjectDirectory does not exist and DataToRecoverVSProject is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
			return;
		}

		DataToRecoverVSProject = new FEAssetPackage();
		for (size_t i = 0; i < SourceFileList.size(); i++)
		{
			std::string FullFilePath = VSProjectDirectory + SourceFileList[i];
			if (!FILE_SYSTEM.DoesFileExist(FullFilePath))
			{
				LOG.Add("FENativeScriptProject::Save: File does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			FEAssetPackageEntryIntializeData EntryData;
			EntryData.Name = SourceFileList[i];
			EntryData.Type = "Text";
			DataToRecoverVSProject->ImportAssetFromFile(FullFilePath, EntryData);
		}
	}
	else
	{
		if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
		{
			LOG.Add("FENativeScriptProject::Save: VSProjectDirectory does not exist and saving old DataToRecoverVSProject!", "FE_LOG_LOADING", FE_LOG_WARNING);
		}
		else
		{
			std::vector<FEAssetPackageAssetInfo> OldSourceFilesList = DataToRecoverVSProject->GetEntryList();
			for (size_t i = 0; i < SourceFileList.size(); i++)
			{
				std::string FullFilePath = VSProjectDirectory + SourceFileList[i];
				if (!FILE_SYSTEM.DoesFileExist(FullFilePath))
				{
					LOG.Add("FENativeScriptProject::Save: File does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
					continue;
				}

				for (size_t j = 0; j < OldSourceFilesList.size(); j++)
				{
					if (OldSourceFilesList[j].Name == SourceFileList[i])
					{
						if (!DataToRecoverVSProject->UpdateAssetFromFile(OldSourceFilesList[j].ID, FullFilePath))
						{
							LOG.Add("FENativeScriptProject::Save: Failed to update asset!", "FE_LOG_LOADING", FE_LOG_ERROR);
						}
						break;
					}
				}
			}
		}
	}
}

bool FENativeScriptProject::UpdateParentScriptModule()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::UpdateParentScriptModule: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<std::string> FullPathSourceFileList;
	for (size_t i = 0; i < SourceFileList.size(); i++)
	{
		FullPathSourceFileList.push_back(VSProjectDirectory + SourceFileList[i]);
	}

	if (!Parent->UpdateFiles(DebugDllFileData.Path, DebugPdbFileData.Path, ReleaseDllFileData.Path, FullPathSourceFileList))
	{
		LOG.Add("FENativeScriptProject::UpdateParentScriptModule: Error updating parent native script module", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!NATIVE_SCRIPT_SYSTEM.ReloadDLL(Parent))
	{
		LOG.Add("FENativeScriptProject::UpdateParentScriptModule: Error reloading parent native script module", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FENativeScriptProject::HasRecoverableVSProjectData()
{
	if (DataToRecoverVSProject == nullptr)
		return false;

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = DataToRecoverVSProject->GetEntryList();
	if (AssetPackageContent.empty())
		return false;

	return true;
}

bool FENativeScriptProject::IsVSProjectFolderValidAndIntact()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Path does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<std::string> InitialFilesToCheck = {
		"CMakeLists.txt",
		"FocalEngine.lib", "FEBasicApplication.lib",
		"BuildManagement/EnsureBuildCompletion.cmake",
		"BuildManagement/DebugBuildActions.cmake", "BuildManagement/ReleaseBuildActions.cmake"
	};

	for (size_t i = 0; i < InitialFilesToCheck.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(VSProjectDirectory + InitialFilesToCheck[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + InitialFilesToCheck[i] + " does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	std::string AppropriateProjectName = GetVSProjectName();
	std::vector<std::string> VSProjectFilesToCheck = {
		AppropriateProjectName + ".sln", AppropriateProjectName + ".vcxproj", AppropriateProjectName + ".vcxproj.filters"
	};

	for (size_t i = 0; i < VSProjectFilesToCheck.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(VSProjectDirectory + VSProjectFilesToCheck[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + VSProjectFilesToCheck[i] + " does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	if (SourceFileList.empty())
		return false;

	for (size_t i = 0; i < SourceFileList.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(VSProjectDirectory + SourceFileList[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + SourceFileList[i] + " does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	return true;
}

bool FENativeScriptProject::EnsureVSProjectDirectoryIsIntact()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::EnsureVSProjectDirectoryIsIntact: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::EnsureVSProjectDirectoryIsIntact: Path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(WorkingDirectory + "NativeScriptProjects/"))
	{
		if (!FILE_SYSTEM.CreateDirectory(WorkingDirectory + "NativeScriptProjects/"))
		{
			LOG.Add("FENativeScriptProject::EnsureVSProjectDirectoryIsIntact: Error creating \"NativeScriptProjects/\" directory", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	if (VSProjectDirectory != WorkingDirectory + "NativeScriptProjects/" + Parent->GetObjectID() + "/")
		VSProjectDirectory = WorkingDirectory + "NativeScriptProjects/" + Parent->GetObjectID() + "/";

	if (FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		if (!FILE_SYSTEM.DeleteDirectory(VSProjectDirectory))
		{
			LOG.Add("FENativeScriptProject::EnsureVSProjectDirectoryIsIntact: Error deleting old directory", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}
	
	if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::EnsureVSProjectDirectoryIsIntact: Error creating directory", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FENativeScriptProject::RecreateVSProject()
{
	if (DataToRecoverVSProject == nullptr)
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: DataToRecoverVSProject is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = DataToRecoverVSProject->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Asset package is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!EnsureVSProjectDirectoryIsIntact())
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Error ensuring VS project directory is intact", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<std::string> SourceFileFullPathList;
	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		DataToRecoverVSProject->ExportAssetToFile(AssetPackageContent[i].ID, VSProjectDirectory + AssetPackageContent[i].Name);
		SourceFileFullPathList.push_back(VSProjectDirectory + AssetPackageContent[i].Name);
	}

	if (!InitializeProject(SourceFileFullPathList))
		return false;

	if (!ConfigureAndBuildCMake())
		return false;

	return true;
}

bool FENativeScriptProject::InitializeProject(std::vector<std::string> SourceFileFullPathList)
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Project path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (SourceFileFullPathList.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: source file list are empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!SourceFileFullPathList.empty())
	{
		for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
		{
			if (!FILE_SYSTEM.DoesFileExist(SourceFileFullPathList[i]))
			{
				LOG.Add("FENativeScriptProject::InitializeProject: Source file " + SourceFileFullPathList[i] + " does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
				return false;
			}
		}
	}

	// Create all needed folders.
	std::vector<std::string> FoldersToCreate = { "SubSystems/", "SubSystems/FocalEngine", "BuildManagement/", "SubSystems/FocalEngine/Resources/", "SubSystems/FocalEngine/Resources/UserScriptsData/" };
	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory + FoldersToCreate[i]))
		{
			LOG.Add("FENativeScriptProject::InitializeProject: Error creating " + FoldersToCreate[i] + " directory", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/NativeScriptProjectData/BuildManagement/EnsureBuildCompletion.cmake", VSProjectDirectory + "BuildManagement/EnsureBuildCompletion.cmake" });
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/NativeScriptProjectData/BuildManagement/DebugBuildActions.cmake", VSProjectDirectory + "BuildManagement/DebugBuildActions.cmake" });
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/NativeScriptProjectData/BuildManagement/ReleaseBuildActions.cmake", VSProjectDirectory + "BuildManagement/ReleaseBuildActions.cmake" });
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/NativeScriptProjectData/CMakeLists_Template.txt", VSProjectDirectory + "CMakeLists.txt" });
	// We need to copy only one .cpp file.
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/FENativeScriptConnector.cpp", VSProjectDirectory + "SubSystems/FocalEngine/Resources/UserScriptsData/FENativeScriptConnector.cpp" });

	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		FilesToCopy.push_back({ SourceFileFullPathList[i], VSProjectDirectory + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) });
		if (FilesToCopy.back().first == FilesToCopy.back().second)
			FilesToCopy.pop_back();
	}

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFile(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FENativeScriptProject::InitializeProject: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	SourceFileList.clear();
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFileList.push_back(FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]));
	}

	if (!InitializeCMakeFileAndScriptFiles(SourceFileFullPathList))
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Error initializing CMakeLists.txt", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!RESOURCE_MANAGER.CopyEngineFiles(true, false, true, VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Error copying engine files in user native script project", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	SetFileTracking();
	return true;
}

bool FENativeScriptProject::InitializeCMakeFileAndScriptFiles(std::vector<std::string> SourceFileFullPathList)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string CMakeFilePath = VSProjectDirectory + "CMakeLists.txt";

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string SourceFilesList = "file(GLOB Main_SRC\n";
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFilesList += "\t\"" + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) + "\"\n";
	}
	SourceFilesList += ")";

	CurrentInstruction.ContextPattern = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.TargetText = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.ReplacementText = SourceFilesList;
	Instructions.push_back(CurrentInstruction);

	if (!FILE_SYSTEM.PerformTextReplacements(CMakeFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing CMakeLists.txt", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.ContextPattern = "--target PLACE_HOLDER --config Release";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string DebugBuildActionsCMake = VSProjectDirectory + "BuildManagement/DebugBuildActions.cmake";
	if (!FILE_SYSTEM.PerformTextReplacements(DebugBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/DebugBuildActions.cmake", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.ContextPattern = "--target PLACE_HOLDER --config Debug";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string ReleaseBuildActionsCMake = VSProjectDirectory + "BuildManagement/ReleaseBuildActions.cmake";
	if (!FILE_SYSTEM.PerformTextReplacements(ReleaseBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/ReleaseBuildActions.cmake", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FENativeScriptProject::ConfigureAndBuildCMake()
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::ConfigureAndBuildCMake: VSProjectDirectory is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 17 2022";

	// CMake configure command.
	std::string ConfigureCommand = "cmake -S \"" + VSProjectDirectory + "\" -B \"" + VSProjectDirectory + "\" -G \"" + Generator + "\"";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FENativeScriptProject::ConfigureAndBuildCMake: Error running CMake configure command", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Construct the CMake build command
	std::string BuildCommand = "cmake --build \"" + VSProjectDirectory + "\" --config Debug";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FENativeScriptProject::ConfigureAndBuildCMake: Error running CMake build command", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FENativeScriptProject::RunVSProject()
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::RunProjectVSSolution: VSProjectDirectory is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string SolutionPath = VSProjectDirectory + GetVSProjectName() + ".sln";

	// Use ShellExecute to open the solution file
	HINSTANCE Result = ShellExecuteA(
		NULL,
		"open",
		SolutionPath.c_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL
	);

	// Check if ShellExecute was successful
	if ((INT_PTR)Result <= 32)
	{
		LOG.Add("FENativeScriptProject::RunProjectVSSolution: Error running solution", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void FENativeScriptProject::SetFileTracking()
{
	DebugDllFileData.Path = VSProjectDirectory + "Debug/" + GetVSProjectName() + ".dll";
	DebugDllFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(DebugDllFileData.Path);

	DebugPdbFileData.Path = VSProjectDirectory + "Debug/" + GetVSProjectName() + ".pdb";
	DebugPdbFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(DebugPdbFileData.Path);

	ReleaseDllFileData.Path = VSProjectDirectory + "Release/" + GetVSProjectName() + ".dll";
	ReleaseDllFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(ReleaseDllFileData.Path);
}

bool FENativeScriptProject::GenerateScriptFilesFromTemplate(std::string ScriptName)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: VSProjectDirectory does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ScriptName.empty())
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Script name is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::string TemplateHeaderFilePath = EnginePath + "/Resources/UserScriptsData/NativeScriptTemplate.h";
	std::string TemplateCPPFilePath = EnginePath + "/Resources/UserScriptsData/NativeScriptTemplate.cpp";

	if (!FILE_SYSTEM.CopyFile(TemplateHeaderFilePath, VSProjectDirectory + ScriptName + ".h"))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error copying file " + TemplateHeaderFilePath + " to " + VSProjectDirectory + ScriptName + ".h", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFile(TemplateCPPFilePath, VSProjectDirectory + ScriptName + ".cpp"))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error copying file " + TemplateCPPFilePath + " to " + VSProjectDirectory + ScriptName + ".cpp", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "class PLACE_HOLDER : public FENativeScriptCore";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "REGISTER_SCRIPT(PLACE_HOLDER)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "REGISTER_SCRIPT_FIELD(PLACE_HOLDER, int, ExampleVariable)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "SET_MODULE_ID(\"PLACE_HOLDER\");";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = Parent->GetObjectID();
	Instructions.push_back(CurrentInstruction);

	std::string ScriptHeaderFilePath = VSProjectDirectory + ScriptName + ".h";
	if (!FILE_SYSTEM.PerformTextReplacements(ScriptHeaderFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error initializing " + ScriptName + ".h", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.ContextPattern = "#include \"NativeScriptTemplate.h\"";
	CurrentInstruction.TargetText = "NativeScriptTemplate";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::Awake()";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::OnDestroy()";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::OnUpdate(double DeltaTime)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ScriptName;
	Instructions.push_back(CurrentInstruction);

	std::string ScriptSourceFilePath = VSProjectDirectory + ScriptName + ".cpp";
	if (!FILE_SYSTEM.PerformTextReplacements(ScriptSourceFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error initializing " + ScriptName + ".cpp", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FENativeScriptProject::CreateNewVSProject(std::string FirstScriptName)
{
	if (FirstScriptName.empty())
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: First script name is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: Parent is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	// Replace all spaces with underscores.
	for (size_t i = 0; i < FirstScriptName.size(); i++)
	{
		if (FirstScriptName[i] == ' ')
			FirstScriptName[i] = '_';
	}

	if (!EnsureVSProjectDirectoryIsIntact())
	{
		LOG.Add("FENativeScriptProject::CreateNewVSProject: Error ensuring VS project directory is intact", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!GenerateScriptFilesFromTemplate(FirstScriptName))
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: Error generating script files from template", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!InitializeProject({ VSProjectDirectory + FirstScriptName + ".h", VSProjectDirectory + FirstScriptName + ".cpp" }))
		return false;

	if (!ConfigureAndBuildCMake())
		return false;

	return RunVSProject();
}

bool FENativeScriptProject::IsFileChanged(const TrackedFileData& FileData)
{
	uint64_t NewWriteTime = FILE_SYSTEM.GetFileLastWriteTime(FileData.Path);
	if (NewWriteTime != FileData.WriteTime && NewWriteTime != 0)
	{
		uint64_t Differece = NewWriteTime >= FileData.WriteTime ? NewWriteTime - FileData.WriteTime : FileData.WriteTime - NewWriteTime;
		if (Differece > DifferenceThreshold)
			return true;
	}

	return false;
}

void FENativeScriptProject::UpdateTrackedFileWriteTime(TrackedFileData& FileData)
{
	uint64_t NewTimeStamp = FILE_SYSTEM.GetFileLastWriteTime(FileData.Path);

	if (NewTimeStamp > FileData.WriteTime)
		FileData.WriteTime = NewTimeStamp;
}

bool FENativeScriptProject::Update()
{
	if (!IsVSProjectFolderValidAndIntact())
		return false;

	bool CheckForReload = false;
	if (IsFileChanged(DebugDllFileData) && IsFileChanged(DebugPdbFileData) || IsFileChanged(ReleaseDllFileData))
		CheckForReload = true;

	if (!CheckForReload)
		return false;

	// Waiting for external build system to finish.
	FILE_SYSTEM.WaitForFileAccess(VSProjectDirectory + "BuildManagement/Force_Build_Finished.txt", 1000);

	if (FILE_SYSTEM.DoesFileExist(VSProjectDirectory + "BuildManagement/Force_Build_Finished.txt"))
	{
		FILE_SYSTEM.DeleteFile(VSProjectDirectory + "BuildManagement/Force_Build_Finished.txt");
	}
	else
	{
		LOG.Add("FENativeScriptProject::Update: Force_Build_Finished.txt does not exist, script reload aborted.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			
		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	// Waiting for all files to be accessible.
	if (!FILE_SYSTEM.WaitForFileAccess(DebugDllFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + DebugDllFileData.Path + "does not exist, script reload aborted.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			
		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	if (!FILE_SYSTEM.WaitForFileAccess(DebugPdbFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + DebugPdbFileData.Path + "does not exist, script reload aborted.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);

		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	if (!FILE_SYSTEM.WaitForFileAccess(ReleaseDllFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + ReleaseDllFileData.Path + "does not exist, script reload aborted.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);

		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	bool bResult = UpdateParentScriptModule();

	// In some cases we would want to save such module right away.
	// It is needed to ensure that the module is saved before the next frame.
	// TO-DO: It is a hack and should be replaced with a proper solution.
	RESOURCE_MANAGER.SaveFENativeScriptModule(Parent, VSProjectDirectory + Parent->GetObjectID() + ".nativescriptmodule");

	UpdateTrackedFileWriteTime(DebugDllFileData);
	UpdateTrackedFileWriteTime(DebugPdbFileData);
	UpdateTrackedFileWriteTime(ReleaseDllFileData);

	return bResult;
}

std::vector<std::string> FENativeScriptProject::GetSourceFileList()
{
	return SourceFileList;
}

bool FENativeScriptProject::ExtractSourceFilesTo(std::string DestinationDirectory)
{
	if (DestinationDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::ExtractSourceFilesTo: Destination directory is empty", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(DestinationDirectory))
	{
		LOG.Add("FENativeScriptProject::ExtractSourceFilesTo: Destination directory does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = DataToRecoverVSProject->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::ExtractSourceFilesTo: Asset package is empty", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<std::string> SourceFileFullPathList;
	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		if (!DataToRecoverVSProject->ExportAssetToFile(AssetPackageContent[i].ID, DestinationDirectory + AssetPackageContent[i].Name))
		{
			LOG.Add("FENativeScriptProject::ExtractSourceFilesTo: Error exporting asset " + AssetPackageContent[i].Name, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}