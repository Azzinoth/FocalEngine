#include "FENativeScriptModule.h"
#include "FENativeScriptProject.h"
using namespace FocalEngine;

FENativeScriptModule::FENativeScriptModule() : FEObject(FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE, "Unnamed native script module")
{
	ScriptAssetPackage = new FEAssetPackage();
	Project = new FENativeScriptProject();
	Project->Parent = this;
};

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

	Project = new FENativeScriptProject();
	Project->Parent = this;
}

bool FENativeScriptModule::UpdateFiles(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles)
{
	// First we will check if all files are valid
	if (!FILE_SYSTEM.DoesFileExist(DebugDLLFilePath))
	{
		LOG.Add("FENativeScriptModule::Update: DebugDLLFilePath does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(DebugPDBFilePath))
	{
		LOG.Add("FENativeScriptModule::Update: DebugPDBFilePath does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(ReleaseDLLFilePath))
	{
		LOG.Add("FENativeScriptModule::Update: ReleaseDLLFilePath does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < ScriptFiles.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(ScriptFiles[i]))
		{
			LOG.Add("FENativeScriptModule::Update: ScriptFiles[" + std::to_string(i) + "] does not exist", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Than we can start updating the files
	if (ScriptAssetPackage->IsAssetIDPresent(DebugDLLAssetID))
	{
		if (!ScriptAssetPackage->UpdateAssetFromFile(DebugDLLAssetID, DebugDLLFilePath))
		{
			LOG.Add("FENativeScriptModule::Update: Failed to update DebugDLLAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}
	else
	{
		DebugDLLAssetID = ScriptAssetPackage->ImportAssetFromFile(DebugDLLFilePath);
		if (DebugDLLAssetID.empty())
		{
			LOG.Add("FENativeScriptModule::Update: Failed to import DebugDLLAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	if (ScriptAssetPackage->IsAssetIDPresent(DebugPDBAssetID))
	{
		if (!ScriptAssetPackage->UpdateAssetFromFile(DebugPDBAssetID, DebugPDBFilePath))
		{
			LOG.Add("FENativeScriptModule::Update: Failed to update DebugPDBAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}
	else
	{
		DebugPDBAssetID = ScriptAssetPackage->ImportAssetFromFile(DebugPDBFilePath);
		if (DebugPDBAssetID.empty())
		{
			LOG.Add("FENativeScriptModule::Update: Failed to import DebugPDBAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	if (ScriptAssetPackage->IsAssetIDPresent(ReleaseDLLAssetID))
	{
		if (!ScriptAssetPackage->UpdateAssetFromFile(ReleaseDLLAssetID, ReleaseDLLFilePath))
		{
			LOG.Add("FENativeScriptModule::Update: Failed to update ReleaseDLLAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}
	else
	{
		ReleaseDLLAssetID = ScriptAssetPackage->ImportAssetFromFile(ReleaseDLLFilePath);
		if (ReleaseDLLAssetID.empty())
		{
			LOG.Add("FENativeScriptModule::Update: Failed to import ReleaseDLLAssetID", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Script files could be added or removed
	// So we will remove all of them and add them again
	for (size_t i = 0; i < ScriptAssetIDs.size(); i++)
	{
		ScriptAssetPackage->RemoveAsset(ScriptAssetIDs[i]);
	}
	ScriptAssetIDs.resize(ScriptFiles.size());
	for (size_t i = 0; i < ScriptFiles.size(); i++)
	{
		ScriptAssetIDs[i] = ScriptAssetPackage->ImportAssetFromFile(ScriptFiles[i]);
	}

	return true;
}

FENativeScriptModule::~FENativeScriptModule() {}

bool FENativeScriptModule::IsLoadedToMemory()
{
	return bIsLoadedToMemory;
}

FENativeScriptProject* FENativeScriptModule::GetProject() const
{
	return Project;
}