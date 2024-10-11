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
		FENativeScriptModule(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles = {});
	public:
		~FENativeScriptModule();

		bool IsLoadedToMemory();
		std::string GetDLLModuleID();
	private:
		FEAssetPackage* ScriptAssetPackage;

		std::string DebugDLLAssetID;
		std::string DebugPDBAssetID;

		std::string ReleaseDLLAssetID;
		std::string ReleasePDBAssetID;

		// FIX ME! That should not be here
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