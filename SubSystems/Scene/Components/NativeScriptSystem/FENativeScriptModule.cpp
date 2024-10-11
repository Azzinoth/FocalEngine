#include "FENativeScriptModule.h"
using namespace FocalEngine;

FENativeScriptModule::FENativeScriptModule() : FEObject(FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE, "Unnamed native script module") {};
FENativeScriptModule::FENativeScriptModule(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles) : FEObject(FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE, "Unnamed native script module")
{
	ScriptAssetPackage = new FEAssetPackage();
	DebugDLLAssetID = ScriptAssetPackage->ImportAssetFromFile(DebugDLLFilePath);
	DebugPDBAssetID = ScriptAssetPackage->ImportAssetFromFile(DebugPDBFilePath);
	ReleaseDLLAssetID = ScriptAssetPackage->ImportAssetFromFile(ReleaseDLLFilePath);

	ScriptAssetIDs.resize(ScriptFiles.size());
	for (size_t i = 0; i < ScriptFiles.size(); i++)
	{
		ScriptAssetIDs[i] = ScriptAssetPackage->ImportAssetFromFile(ScriptFiles[i]);
	}
}

FENativeScriptModule::~FENativeScriptModule() {}

bool FENativeScriptModule::IsLoadedToMemory()
{
	return bIsLoadedToMemory;
}

std::string FENativeScriptModule::GetDLLModuleID()
{
	return DLLModuleID;
}