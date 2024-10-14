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

//void FENativeScriptProject::SetProjectName(std::string NewValue)
//{
//	std::string TemporaryProjectName = NewValue;
//	std::string CorrectProjectName = "";
//	for (size_t i = 0; i < TemporaryProjectName.size(); i++)
//	{
//		if (TemporaryProjectName[i] == ' ')
//			CorrectProjectName += "_";
//		else
//			CorrectProjectName += TemporaryProjectName[i];
//	}
//
//	ProjectName = CorrectProjectName;
//}
//
//std::string FENativeScriptProject::GetProjectName()
//{
//	return ProjectName;
//}

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
		"SubSystems/FocalEngine/FENativeScriptConnector.h", "SubSystems/FocalEngine/FENativeScriptConnector.cpp",
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
	std::vector<std::string> FoldersToCreate = { "SubSystems/", "SubSystems/FocalEngine", "BuildManagement/" };
	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory + FoldersToCreate[i]))
		{
			LOG.Add("FENativeScriptProject::InitializeProject: Error creating " + FoldersToCreate[i] + " directory", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EditorFolder = FILE_SYSTEM.GetCurrentWorkingPath();
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/EnsureBuildCompletion.cmake", VSProjectDirectory + "BuildManagement/EnsureBuildCompletion.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/DebugBuildActions.cmake", VSProjectDirectory + "BuildManagement/DebugBuildActions.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/ReleaseBuildActions.cmake", VSProjectDirectory + "BuildManagement/ReleaseBuildActions.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/CMakeLists.txt", VSProjectDirectory + "CMakeLists.txt" });
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/FENativeScriptConnector.h", VSProjectDirectory + "SubSystems/FocalEngine/FENativeScriptConnector.h" });
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/FENativeScriptConnector.cpp", VSProjectDirectory + "SubSystems/FocalEngine/FENativeScriptConnector.cpp" });

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

	if (!UpdateEngineFiles())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Error updating engine files in user native script project", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
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

	std::vector<InstructionWhatToReplaceInFile> Instructions;
	InstructionWhatToReplaceInFile CurrentInstruction;
	CurrentInstruction.SubStringInLineToTrigger = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string SourceFilesList = "file(GLOB Main_SRC\n";
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFilesList += "\t\"" + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) + "\"\n";
	}
	SourceFilesList += ")";

	CurrentInstruction.SubStringInLineToTrigger = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.What = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.ReplaceWith = SourceFilesList;
	Instructions.push_back(CurrentInstruction);

	if (!ReplaceInFile(CMakeFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing CMakeLists.txt", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "--target PLACE_HOLDER --config Release";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string DebugBuildActionsCMake = VSProjectDirectory + "BuildManagement/DebugBuildActions.cmake";
	if (!ReplaceInFile(DebugBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/DebugBuildActions.cmake", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "--target PLACE_HOLDER --config Debug";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = GetVSProjectName();
	Instructions.push_back(CurrentInstruction);

	std::string ReleaseBuildActionsCMake = VSProjectDirectory + "BuildManagement/ReleaseBuildActions.cmake";
	if (!ReplaceInFile(ReleaseBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/ReleaseBuildActions.cmake", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "SET_MODULE_ID(\"PLACE_HOLDER\");";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = Parent->GetObjectID();
	Instructions.push_back(CurrentInstruction);

	std::string FENativeScriptConnectorHeaderFilePath = VSProjectDirectory + "SubSystems/FocalEngine/FENativeScriptConnector.h";
	if (!ReplaceInFile(FENativeScriptConnectorHeaderFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing FENativeScriptConnector.h", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FENativeScriptProject::ReplaceInFile(std::string Path, std::vector<InstructionWhatToReplaceInFile> Instructions)
{
	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: File path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::fstream File(Path, std::ios::in);
	if (!File.is_open())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: Error opening file " + Path, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<std::string> FileContent;
	std::string Line;
	while (std::getline(File, Line))
		FileContent.push_back(Line);

	File.close();

	for (size_t i = 0; i < FileContent.size(); i++)
	{
		for (size_t j = 0; j < Instructions.size(); j++)
		{
			if (FileContent[i].find(Instructions[j].SubStringInLineToTrigger) != std::string::npos)
			{
				FileContent[i].replace(FileContent[i].find(Instructions[j].What), Instructions[j].What.size(), Instructions[j].ReplaceWith);
			}
		}
	}

	File.open(Path, std::ios::out | std::ios::trunc);
	if (!File.is_open())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: Error opening file " + Path, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Write the modified content back to the file
	for (size_t i = 0; i < FileContent.size(); i++)
		File << FileContent[i] + "\n";

	File.close();
	return true;
}

bool FENativeScriptProject::UpdateEngineFiles()
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: ProjectPath is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: ProjectPath does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	FEAssetPackage* EngineHeadersPackage = CreateEngineHeadersAssetPackage();
	if (EngineHeadersPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error creating engine headers asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!UnPackEngineHeadersAssetPackage(EngineHeadersPackage, VSProjectDirectory + "SubSystems/FocalEngine/"))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error unpacking engine headers asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackage* EngineLIBPackage = CreateEngineLIBAssetPackage();
	if (EngineLIBPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error creating engine lib asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!UnPackEngineLIBAssetPackage(EngineLIBPackage, VSProjectDirectory))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error unpacking engine lib asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

FEAssetPackage* FENativeScriptProject::CreateEngineHeadersAssetPackage()
{
	FEAssetPackage* EngineHeadersAssetPackage = new FEAssetPackage();
	EngineHeadersAssetPackage->SetName("EngineHeaders");
	if (EngineHeadersAssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::CreateEngineHeadersAssetPackage: Error creating asset package", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FENativeScriptProject::CreateEngineHeadersAssetPackage: Engine folder does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);
	// After having all files in the engine folder, we need to filter out only the header files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].find(".h") != std::string::npos || AllFiles[i].find(".inl") != std::string::npos)
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(EnginePath.size());
			EntryData.Type = "Text";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "Engine header file";

			EngineHeadersAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineHeadersAssetPackage;
}

bool FENativeScriptProject::UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Asset package is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Destination path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Destination path does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Asset package is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		std::string LocalPath = std::filesystem::path(AssetPackageContent[i].Name).parent_path().string();
		// Since we are not using folders in FEAssetPackage, we need to create all folders in the file path.
		// First we need to get chain of folders.
		std::vector<std::string> FolderChain;
		try
		{
			std::filesystem::path Directory(LocalPath);
			while (!Directory.string().empty())
			{
				if (!FolderChain.empty())
				{
					if (FolderChain.back() == Directory.string())
						break;
				}
				FolderChain.push_back(Directory.string());
				Directory = Directory.parent_path();
			}

			std::reverse(FolderChain.begin(), FolderChain.end());
		}
		catch (const std::exception& Exception)
		{
			LOG.Add("Error in FENativeScriptProject::UnPackEngineHeadersAssetPackage: " + std::string(Exception.what()), "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}

		// Then we will go from the root folder to the last folder and create them if they do not exist.
		for (size_t i = 0; i < FolderChain.size(); i++)
		{
			std::string FinalPath = Path + FolderChain[i];
			if (!FILE_SYSTEM.DoesDirectoryExist(FinalPath))
			{
				if (!FILE_SYSTEM.CreateDirectory(FinalPath))
				{
					LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error creating directory " + FinalPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
					return false;
				}
			}
		}

		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

FEAssetPackage* FENativeScriptProject::CreateEngineLIBAssetPackage()
{
	FEAssetPackage* EngineLIBAssetPackage = new FEAssetPackage();
	EngineLIBAssetPackage->SetName("EngineHeaders");
	if (EngineLIBAssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::CreateEngineLIBAssetPackage: Error creating asset package", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FENativeScriptProject::CreateEngineLIBAssetPackage: Engine folder does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);

	std::vector<std::string> DebugStrings;
	// After having all files in the engine folder, we need to filter out only the lib files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].find(".lib") != std::string::npos)
		{
			// FIX ME! Currently projects would need only debug lib files. Is it correct?
			// And only FocalEngine.lib and FEBasicApplication.lib are needed.
			if (AllFiles[i].find("FocalEngine.lib") == std::string::npos && AllFiles[i].find("FEBasicApplication.lib") == std::string::npos)
				continue;

			FEAssetPackageEntryIntializeData EntryData;
			EntryData.Name = FILE_SYSTEM.GetFileName(AllFiles[i]);
			DebugStrings.push_back(EntryData.Name);
			EntryData.Type = "BINARY";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "Engine lib file";

			EngineLIBAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineLIBAssetPackage;
}

bool FENativeScriptProject::UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Asset package is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Destination path is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Destination path does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Asset package is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool FENativeScriptProject::ConfigureAndBuildCMake()
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FENativeScriptProject::RunCMake: VSProjectDirectory is empty", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 17 2022";

	// CMake configure command.
	std::string ConfigureCommand = "cmake -S \"" + VSProjectDirectory + "\" -B \"" + VSProjectDirectory + "\" -G \"" + Generator + "\"";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FENativeScriptProject::RunCMake: Error running CMake configure command", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Construct the CMake build command
	std::string BuildCommand = "cmake --build \"" + VSProjectDirectory + "\" --config Debug";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FENativeScriptProject::RunCMake: Error running CMake build command", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
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

	std::vector<InstructionWhatToReplaceInFile> Instructions;
	InstructionWhatToReplaceInFile CurrentInstruction;
	CurrentInstruction.SubStringInLineToTrigger = "SET_MODULE_ID(\"PLACE_HOLDER\")";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = "2342HA";
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "class PLACE_HOLDER : public FENativeScriptCore";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "REGISTER_SCRIPT(PLACE_HOLDER)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "REGISTER_SCRIPT_FIELD(PLACE_HOLDER, int, ExampleVariable)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	std::string ScriptHeaderFilePath = VSProjectDirectory + ScriptName + ".h";
	if (!ReplaceInFile(ScriptHeaderFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error initializing " + ScriptName + ".h", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "#include \"NativeScriptTemplate.h\"";
	CurrentInstruction.What = "NativeScriptTemplate";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::Awake()";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::OnDestroy()";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::OnUpdate(double DeltaTime)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	std::string ScriptSourceFilePath = VSProjectDirectory + ScriptName + ".cpp";
	if (!ReplaceInFile(ScriptSourceFilePath, Instructions))
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

	UpdateTrackedFileWriteTime(DebugDllFileData);
	UpdateTrackedFileWriteTime(DebugPdbFileData);
	UpdateTrackedFileWriteTime(ReleaseDllFileData);

	return bResult;
}