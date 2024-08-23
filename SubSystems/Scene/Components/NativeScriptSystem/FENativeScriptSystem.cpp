#include "FENativeScriptSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetNativeScriptSystem()
{
	return FENativeScriptSystem::GetInstancePointer();
}
#endif

FENativeScriptSystem::FENativeScriptSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FENativeScriptComponent>(NativeScriptComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FENativeScriptComponent>(NativeScriptComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FENativeScriptComponent>(DuplicateNativeScriptComponent);
}

void FENativeScriptSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FENativeScriptComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FENativeScriptComponent>(OnMyComponentDestroy);
}

void FENativeScriptSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
	//NativeScriptComponent.ParentEntity = Entity;
	//auto& nsc = entity->GetComponent<FENativeScriptComponent>();
	/*if (NativeScriptComponent.CreateScript)
		NativeScriptComponent.CreateScript(NativeScriptComponent);

	if (NativeScriptComponent.Instance)
	{
		NativeScriptComponent.Instance->Entity = Entity;
		NativeScriptComponent.Instance->OnCreate();
	}*/
}

//void FENativeScriptSystem::Init(FEEntity* Entity)
//{
//	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
//		return;
//
//	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
//	//NativeScriptComponent.ParentEntity = Entity;
//
//	if (NativeScriptComponent.CreateScript)
//		NativeScriptComponent.CreateScript(NativeScriptComponent);
//
//	if (NativeScriptComponent.CoreInstance)
//	{
//		NativeScriptComponent.CoreInstance->ParentEntity = Entity;
//		NativeScriptComponent.CoreInstance->OnCreate();
//	}
//}

void FENativeScriptSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
}

FENativeScriptSystem::~FENativeScriptSystem() {};

void FENativeScriptSystem::Update(double DeltaTime)
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (FEScene* Scene : ActiveScenes)
	{
		// FIX ME! When scene is paused, we should not update the scripts.
		std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.IsInitialized())
				NativeScriptComponent.OnUpdate(DeltaTime);
		}
	}
}

void FENativeScriptSystem::DuplicateNativeScriptComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr)
		return;

	TargetEntity->GetComponent<FENativeScriptComponent>() = SourceEntity->GetComponent<FENativeScriptComponent>();
	//TargetEntity->GetComponent<FENativeScriptComponent>().ParentEntity = TargetEntity;
}

Json::Value FENativeScriptSystem::NativeScriptComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	/*glm::vec3 Position = TransformComponent.GetPosition();
	Root["Position"]["X"] = Position.x;
	Root["Position"]["Y"] = Position.y;
	Root["Position"]["Z"] = Position.z;

	glm::quat Rotation = TransformComponent.GetQuaternion();
	Root["Rotation"]["X"] = Rotation.x;
	Root["Rotation"]["Y"] = Rotation.y;
	Root["Rotation"]["Z"] = Rotation.z;
	Root["Rotation"]["W"] = Rotation.w;

	Root["Scale"]["UniformScaling"] = TransformComponent.IsUniformScalingSet();
	glm::vec3 Scale = TransformComponent.GetScale();
	Root["Scale"]["X"] = Scale.x;
	Root["Scale"]["Y"] = Scale.y;
	Root["Scale"]["Z"] = Scale.z;*/

	return Root;
}

void FENativeScriptSystem::NativeScriptComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	/*glm::vec3 Position;
	Position.x = Root["Position"]["X"].asFloat();
	Position.y = Root["Position"]["Y"].asFloat();
	Position.z = Root["Position"]["Z"].asFloat();
	TransformComponent.SetPosition(Position);

	glm::quat Rotation;
	Rotation.x = Root["Rotation"]["X"].asFloat();
	Rotation.y = Root["Rotation"]["Y"].asFloat();
	Rotation.z = Root["Rotation"]["Z"].asFloat();
	Rotation.w = Root["Rotation"]["W"].asFloat();
	TransformComponent.SetQuaternion(Rotation);

	bool bUniformScaling = Root["Scale"]["UniformScaling"].asBool();
	TransformComponent.SetUniformScaling(bUniformScaling);

	glm::vec3 Scale;
	Scale.x = Root["Scale"]["X"].asFloat();
	Scale.y = Root["Scale"]["Y"].asFloat();
	Scale.z = Root["Scale"]["Z"].asFloat();
	TransformComponent.SetScale(Scale);*/
}

bool FENativeScriptSystem::LoadCompiledNativeScriptModule(std::string DLLPath)
{
	if (DLLPath.empty())
	{
		LOG.Add("DLL path is empty in FENativeScriptSystem::LoadCompiledNativeScriptModule.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	auto TimeStamp = TIME.GetTimeStamp();

	if (!FILE_SYSTEM.CheckDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "\\NativeScriptsTemp"))
		FILE_SYSTEM.CreateDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "\\NativeScriptsTemp");

	std::string CurrentTempDirectory = FILE_SYSTEM.GetCurrentWorkingPath() + "\\NativeScriptsTemp\\" + std::to_string(TimeStamp);
	FILE_SYSTEM.CreateDirectory(CurrentTempDirectory);
	//FILE_SYSTEM.CreateDirectory();

	std::string DLLName = FILE_SYSTEM.GetFileName(DLLPath);
	DLLName = DLLName.substr(0, DLLName.find_last_of("."));
	std::string FileParentDirectory = FILE_SYSTEM.GetCurrentWorkingPath();
	std::string NewDLLPath = CurrentTempDirectory + "\\" + DLLName + ".dll";
	FILE_SYSTEM.CopyFile(DLLPath, NewDLLPath);

	std::string PDBName = FILE_SYSTEM.GetFileName(DLLPath);
	// We need to remove the extension from the PDB name.
	PDBName = PDBName.substr(0, PDBName.find_last_of("."));
	std::string OldPDBPath = FILE_SYSTEM.GetDirectoryPath(DLLPath) + "\\" + PDBName + ".pdb";
	std::string NewPDBPath = CurrentTempDirectory + "\\" + PDBName + ".pdb";
	FILE_SYSTEM.CopyFile(OldPDBPath, NewPDBPath);

	HMODULE DLLHandle = LoadLibraryA(NewDLLPath.c_str());
	if (!DLLHandle)
	{
		LOG.Add("FENativeScriptSystem::LoadCompiledNativeScriptModule failed to load DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	
	// Ensuring that the DLL has the required functions.
	typedef char* (*Get_ModuleID_Function)(void);
	Get_ModuleID_Function GetModuleID = (Get_ModuleID_Function)GetProcAddress(DLLHandle, "GetModuleID");
	if (!GetModuleID)
	{
		LOG.Add("FENativeScriptSystem::LoadCompiledNativeScriptModule failed to get GetModuleID function from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::string NewModuleID = GetModuleID();
	if (NewModuleID.empty() || NewModuleID.size() != 24)
	{
		LOG.Add("FENativeScriptSystem::LoadCompiledNativeScriptModule failed to get proper ModuleID from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	typedef void* (*Get_Script_Registry_Function)(void);
	Get_Script_Registry_Function GetScriptRegistry = (Get_Script_Registry_Function)GetProcAddress(DLLHandle, "GetScriptRegistry");
	if (!GetScriptRegistry)
	{
		LOG.Add("FENativeScriptSystem::LoadCompiledNativeScriptModule failed to get GetScriptRegistry function from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	void* RawDLLRegistry = GetScriptRegistry();
	if (!RawDLLRegistry)
	{
		LOG.Add("FENativeScriptSystem::LoadCompiledNativeScriptModule failed to get ScriptRegistry from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::unordered_map<std::string, FEScriptData>* DLLRegistry = (std::unordered_map<std::string, FEScriptData>*)RawDLLRegistry;

	// Check if we already have a module with the same ID.
	if (Modules.find(NewModuleID) != Modules.end())
	{
		// Currently we will just replace the old module with the new one.
		// But before that we should unload the old module.
		FreeLibrary(Modules[NewModuleID]->DLLHandle);
		std::string TemporaryDLLPath = Modules[NewModuleID]->DLLPath;
		std::string TemporaryPDBPath = Modules[NewModuleID]->PDBPath;
		Modules.erase(NewModuleID);

		std::string FolderPath = FILE_SYSTEM.GetDirectoryPath(TemporaryDLLPath);
		FILE_SYSTEM.DeleteFile(TemporaryDLLPath);
		FILE_SYSTEM.DeleteFile(TemporaryPDBPath);
		FILE_SYSTEM.DeleteDirectory(FolderPath);
	}
	
	FENativeScriptModuleData* NewModule = new FENativeScriptModuleData();
	NewModule->Name = DLLPath;
	NewModule->DLLPath = NewDLLPath;
	NewModule->PDBPath = NewPDBPath;
	NewModule->ID = NewModuleID;

	NewModule->DLLHandle = DLLHandle;
	NewModule->Registry = (*DLLRegistry);

	Modules[NewModuleID] = NewModule;
	UpdateScriptComponentsDueToModuleUpdate(NewModuleID);

	return true;
}

void FENativeScriptSystem::InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, std::string ModuleID, FEScriptData& ScriptData)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeComponentInternal failed to add script component to null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (ModuleID.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeComponentInternal failed to add script component to entity with empty ModuleID.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}
	
	NativeScriptComponent.CoreInstance = ScriptData.ConstructorFunction();
	NativeScriptComponent.ModuleID = ModuleID;
	NativeScriptComponent.CoreInstance->ParentEntity = Entity;
	NativeScriptComponent.ScriptData = &ScriptData;
}

void FENativeScriptSystem::UpdateScriptComponentsDueToModuleUpdate(std::string ModuleID)
{
	if (Modules.find(ModuleID) == Modules.end())
	{
		LOG.Add("FENativeScriptSystem::UpdateScriptComponentsDueToModuleUpdate failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	FENativeScriptModuleData* ModuleData = Modules[ModuleID];
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (FEScene* Scene : ActiveScenes)
	{
		std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.ModuleID == ModuleID)
			{
				std::string OldScriptName = NativeScriptComponent.GetScriptData()->Name;
				// We should remove the script component from the entity.
				Entities[i]->RemoveComponent<FENativeScriptComponent>();
				// If the script is not in the new registry, we are done here.
				if (ModuleData->Registry.find(OldScriptName) == ModuleData->Registry.end())
					continue;

				Entities[i]->AddComponent<FENativeScriptComponent>();
				InitializeComponentInternal(Entities[i], Entities[i]->GetComponent<FENativeScriptComponent>(), ModuleID, ModuleData->Registry[OldScriptName]);
			}
		}
	}
}

FENativeScriptModuleData* FENativeScriptSystem::GetModuleData(std::string ModuleID)
{
	if (Modules.find(ModuleID) == Modules.end())
	{
		LOG.Add("FENativeScriptSystem::GetModuleData failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	return Modules[ModuleID];
}

std::vector<std::string> FENativeScriptSystem::GetModuleScriptNameList(std::string ModuleID)
{
	FENativeScriptModuleData* ModuleData = GetModuleData(ModuleID);
	if (ModuleData == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetModuleScriptNameList failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::vector<std::string>();
	}

	std::vector<std::string> Result;

	auto Iterator = ModuleData->Registry.begin();
	while (Iterator != ModuleData->Registry.end())
	{
		std::string ScriptName = Iterator->first;
		Result.push_back(ScriptName);

		Iterator++;
	}

	return Result;
}

std::vector<std::string> FENativeScriptSystem::GetModuleIDList()
{
	std::vector<std::string> Result;

	auto Iterator = Modules.begin();
	while (Iterator != Modules.end())
	{
		std::string ModuleID = Iterator->first;
		Result.push_back(ModuleID);

		Iterator++;
	}

	return Result;
}

void FENativeScriptSystem::InitializeScriptComponent(FEEntity* Entity, std::string ModuleID, std::string ScriptName)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (ModuleID.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to entity with empty ModuleID.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (ScriptName.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to entity with empty ScriptName.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (Modules.find(ModuleID) == Modules.end())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	FENativeScriptModuleData* ModuleData = Modules[ModuleID];
	if (ModuleData->Registry.find(ScriptName) == ModuleData->Registry.end())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find script with name: " + ScriptName + " in module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (!Entity->HasComponent<FENativeScriptComponent>())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed because entity does not have FENativeScriptComponent.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	InitializeComponentInternal(Entity, Entity->GetComponent<FENativeScriptComponent>(), ModuleID, ModuleData->Registry[ScriptName]);
}

std::unordered_map<std::string, FEScriptVariableInfo> FENativeScriptSystem::GetVariablesRegistry(FEEntity* Entity)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to get field registry from null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	if (!Entity->HasComponent<FENativeScriptComponent>())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to get field registry from entity without FENativeScriptComponent.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
	if (Modules.find(NativeScriptComponent.ModuleID) == Modules.end())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find module with ID: " + NativeScriptComponent.ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	FENativeScriptModuleData* ModuleData = Modules[NativeScriptComponent.ModuleID];
	if (ModuleData->Registry.find(NativeScriptComponent.GetScriptData()->Name) == ModuleData->Registry.end())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find script with name: " + NativeScriptComponent.GetModuleID() + " in module with ID: " + NativeScriptComponent.ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	return ModuleData->Registry[NativeScriptComponent.GetModuleID()].VariablesRegistry;
}

std::unordered_map<std::string, FEScriptVariableInfo> FENativeScriptSystem::GetVariablesRegistry(std::string ModuleID, std::string ScriptName)
{
	if (Modules.find(ModuleID) == Modules.end())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	FENativeScriptModuleData* ModuleData = Modules[ModuleID];
	if (ModuleData->Registry.find(ScriptName) == ModuleData->Registry.end())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find script with name: " + ScriptName + " in module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	return ModuleData->Registry[ScriptName].VariablesRegistry;
}