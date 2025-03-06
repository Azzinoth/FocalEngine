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
		std::string GetVSProjectName();

		bool HasRecoverableVSProjectData();
		bool CreateNewVSProject(std::string FirstScriptName);

		bool IsVSProjectFolderValidAndIntact();
		bool RecreateVSProject();

		bool RunVSProject();

		bool Initialize(FEAssetPackage* Data);
		void UpdateDataToRecoverVSProject();

		bool Update();

		std::vector<std::string> GetSourceFileList();
		bool ExtractSourceFilesTo(std::string DestinationDirectory);
	};
}