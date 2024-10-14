#pragma once

#include "FENativeScriptCore.h"
#include "../../../FileSystem/FEAssetPackage.h"

namespace FocalEngine
{
	class FENativeScriptProject;
	class FENativeScriptModule : public FEObject
	{
		friend class FEResourceManager;
		friend class FENativeScriptSystem;
		friend class FENativeScriptProject;
	
		FENativeScriptModule();
		FENativeScriptModule(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles = {});

		bool UpdateFiles(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles = {});
	public:
		~FENativeScriptModule();

		bool IsLoadedToMemory();
		FENativeScriptProject* GetProject() const;
	private:
		FEAssetPackage* ScriptAssetPackage;

		std::string DebugDLLAssetID;
		std::string DebugPDBAssetID;

		std::string ReleaseDLLAssetID;
		std::string ReleasePDBAssetID;

		std::string CMakeFileAssetID;
		std::vector<std::string> ScriptAssetIDs;

		bool bIsLoadedToMemory = false;
		HMODULE DLLHandle = nullptr;
		std::unordered_map<std::string, FEScriptData> Registry;

		std::string ExtractedDLLPath;
		std::string ExtractedPDBPath;

		FENativeScriptProject* Project = nullptr;
	};
}