#include "FENativeScriptModule.h"
using namespace FocalEngine;

FENativeScriptModule::FENativeScriptModule() : FEObject(FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE, "Unnamed native script module") {};
FENativeScriptModule::FENativeScriptModule(std::string DLLFilePath, std::string PDBFilePath, std::vector<std::string> ScriptFiles) : FEObject(FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE, "Unnamed native script module")
{
	ScriptAssetPackage = new FEAssetPackage();
	DLLAssetID = ScriptAssetPackage->ImportAssetFromFile(DLLFilePath);
	if (!PDBFilePath.empty())
		PDBAssetID = ScriptAssetPackage->ImportAssetFromFile(PDBFilePath);

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