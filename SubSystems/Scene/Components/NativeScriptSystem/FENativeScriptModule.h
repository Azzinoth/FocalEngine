#pragma once

#include "FENativeScriptCore.h"
#include "../../../FileSystem/FEAssetPackage.h"

namespace FocalEngine
{
	class FENativeScriptModule : public FEObject
	{
		friend class FEResourceManager;
		friend class FENativeScriptSystem;
	
		FENativeScriptModule();
		FENativeScriptModule(std::string DLLFilePath, std::string PDBFilePath = "", std::vector<std::string> ScriptFiles = {});
	public:
		~FENativeScriptModule();

		bool IsLoadedToMemory();
	private:
		FEAssetPackage* ScriptAssetPackage;
		std::string DLLAssetID;
		std::string PDBAssetID;

		std::string CMakeFileAssetID;
		std::vector<std::string> ScriptAssetIDs;

		bool bIsLoadedToMemory = false;
		HMODULE DLLHandle = nullptr;
		std::string DLLModuleID;
		std::unordered_map<std::string, FEScriptData> Registry;

		std::string ExtractedDLLPath;
		std::string ExtractedPDBPath;
	};
}