#pragma once
#include "FENativeScriptSystem.h"
using namespace FocalEngine;

namespace FocalEngine
{
	struct TrackedFileData
	{
		std::string Path;
		uint64_t WriteTime;
	};

	class FENativeScriptProject
	{
		friend class FENativeScriptModule;
		friend class FEResourceManager;

		FENativeScriptModule* Parent = nullptr;

		std::string WorkingDirectory = "";
		std::string VSProjectDirectory = "";
		//std::string ProjectName = "";
		bool EnsureVSProjectDirectoryIsIntact();

		std::vector<std::string> SourceFileList;
		FEAssetPackage* DataToRecoverVSProject = nullptr;

		TrackedFileData DebugDllFileData;
		TrackedFileData DebugPdbFileData;
		TrackedFileData ReleaseDllFileData;

		const uint64_t DifferenceThreshold = 1'000'000;

		FENativeScriptModule* LastGeneratedScriptModule = nullptr;
		FENativeScriptModule* ModuleThatIsBeingEdited = nullptr;

		bool InitializeProject(std::vector<std::string> SourceFileFullPathList);
		bool ConfigureAndBuildCMake();

		bool InitializeCMakeFileAndScriptFiles(std::vector<std::string> SourceFileFullPathList);

		struct InstructionWhatToReplaceInFile
		{
			std::string SubStringInLineToTrigger;
			std::string What;
			std::string ReplaceWith;
		};

		bool ReplaceInFile(std::string FilePath, std::vector<InstructionWhatToReplaceInFile> Instructions);
		bool UpdateEngineFiles();

		FEAssetPackage* CreateEngineHeadersAssetPackage();
		bool UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path);

		FEAssetPackage* CreateEngineLIBAssetPackage();
		bool UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path);

		void SetFileTracking();
		bool GenerateScriptFilesFromTemplate(std::string ScriptName);

		bool IsFileChanged(const TrackedFileData& FileData);
		void UpdateTrackedFileWriteTime(TrackedFileData& FileData);

		bool UpdateParentScriptModule();
	public:
		FENativeScriptProject();
		~FENativeScriptProject();

		void SetWorkingDirectory(std::string NewValue);
		std::string GetWorkingDirectory();
		std::string GetVSProjectDirectory();

		// Space in project name is not allowed
		//void SetProjectName(std::string NewValue);
		//std::string GetProjectName();
		std::string GetVSProjectName();

		bool HasRecoverableVSProjectData();
		bool CreateNewVSProject(std::string FirstScriptName);

		bool IsVSProjectFolderValidAndIntact();
		bool RecreateVSProject();

		bool RunVSProject();

		bool Initialize(FEAssetPackage* Data);
		void UpdateDataToRecoverVSProject();

		bool Update();
	};
}