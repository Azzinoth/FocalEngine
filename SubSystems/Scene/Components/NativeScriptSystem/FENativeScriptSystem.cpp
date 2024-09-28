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

	// We should clean folder with extracted native scripts.
	std::string ExtractedFolderPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/ExtractedNativeScripts/";
	if (FILE_SYSTEM.DoesDirectoryExist(ExtractedFolderPath))
		FILE_SYSTEM.DeleteDirectory(ExtractedFolderPath);

	// Activate all standard script modules.
	// After engine is initialized, we should activate the modules.
	std::vector<std::string> ModulesIDsToActivate = RESOURCE_MANAGER.GetNativeScriptModuleList();
	for (size_t i = 0; i < ModulesIDsToActivate.size(); i++)
	{
		ActivateNativeScriptModule(ModulesIDsToActivate[i]);
	}

	FENativeScriptModule* ModuleWithCameraScripts = RESOURCE_MANAGER.GetNativeScriptModule("2B7956623302254F620A675F");
	if (ModuleWithCameraScripts == nullptr)
	{
		LOG.Add("can't find module with camera scripts in FEResourceManager::LoadStandardPrefabs", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	// Free camera
	FEPrefab* FreeCameraPrefab = RESOURCE_MANAGER.CreatePrefab("Free camera prefab", "4575527C773848040760656F");
	FreeCameraPrefab->SetTag(ENGINE_RESOURCE_TAG);

	FEScene* PrefabScene = FreeCameraPrefab->GetScene();

	FEEntity* CameraEntity = PrefabScene->CreateEntity("Free camera");
	CameraEntity->AddComponent<FECameraComponent>();
	CameraEntity->AddComponent<FENativeScriptComponent>();
	InitializeScriptComponent(CameraEntity, ModuleWithCameraScripts->GetObjectID(), "FreeCameraController");

	// Model view camera
	FEPrefab* ModelViewCameraPrefab = RESOURCE_MANAGER.CreatePrefab("Model view camera prefab", "14745A482D1B2C328C268027");
	ModelViewCameraPrefab->SetTag(ENGINE_RESOURCE_TAG);

	PrefabScene = ModelViewCameraPrefab->GetScene();

	CameraEntity = PrefabScene->CreateEntity("Model view camera");
	CameraEntity->AddComponent<FECameraComponent>();
	CameraEntity->AddComponent<FENativeScriptComponent>();
	InitializeScriptComponent(CameraEntity, ModuleWithCameraScripts->GetObjectID(), "ModelViewCameraController");
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
}

void FENativeScriptSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
}

FENativeScriptSystem::~FENativeScriptSystem() {};

void FENativeScriptSystem::Update(double DeltaTime)
{
	std::vector<FEScene*> ActiveGameModeScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::GameMode);
	for (FEScene* Scene : ActiveGameModeScenes)
	{
		std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.IsInitialized())
				NativeScriptComponent.OnUpdate(DeltaTime);
		}
	}

	//if (ActiveGameModeScenes.empty())
	//{
		std::vector<FEScene*> ActiveEditorScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::EditorMode);
		for (FEScene* Scene : ActiveEditorScenes)
		{
			std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

			for (size_t i = 0; i < Entities.size(); i++)
			{
				FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
				if (NativeScriptComponent.IsInitialized() && NativeScriptComponent.ScriptData->bRunInEditor)
					NativeScriptComponent.OnUpdate(DeltaTime);
			}
		}
	//}
}

void FENativeScriptSystem::CopyVariableValuesInternal(FENativeScriptComponent* SourceComponent, FENativeScriptComponent* TargetComponent)
{
	if (SourceComponent == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CopyVariableValuesInternal failed to copy variable values from null component.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (SourceComponent->CoreInstance == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CopyVariableValuesInternal failed to copy variable values from component with null core instance.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (TargetComponent == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CopyVariableValuesInternal failed to copy variable values to null component.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (TargetComponent->CoreInstance == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CopyVariableValuesInternal failed to copy variable values to component with null core instance.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	std::unordered_map<std::string, FEScriptVariableInfo> SourceVariables = GetVariablesRegistry(SourceComponent->CoreInstance->ParentEntity);
	std::unordered_map<std::string, FEScriptVariableInfo> TargetVariables = GetVariablesRegistry(TargetComponent->CoreInstance->ParentEntity);

	auto VariableIterator = SourceVariables.begin();
	while (VariableIterator != SourceVariables.end())
	{
		std::string VariableName = VariableIterator->first;
		FEScriptVariableInfo VariableInfo = VariableIterator->second;

		if (TargetVariables.find(VariableName) == TargetVariables.end())
			continue;

		FEScriptVariableInfo TargetVariableInfo = TargetVariables[VariableName];
		if (VariableInfo.Type != TargetVariableInfo.Type)
			continue;

		std::any VariableValue = VariableInfo.Getter(SourceComponent->GetCoreInstance());
		TargetVariableInfo.Setter(TargetComponent->GetCoreInstance(), VariableValue);

		VariableIterator++;
	}
}

void FENativeScriptSystem::DuplicateNativeScriptComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DuplicateNativeScriptComponent failed to duplicate script component from null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (TargetEntity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DuplicateNativeScriptComponent failed to duplicate script component to null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (!SourceEntity->HasComponent<FENativeScriptComponent>())
	{
		LOG.Add("FENativeScriptSystem::DuplicateNativeScriptComponent failed to duplicate script component from entity without FENativeScriptComponent.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}
		
	if (TargetEntity->HasComponent<FENativeScriptComponent>())
	{
		LOG.Add("FENativeScriptSystem::DuplicateNativeScriptComponent failed to duplicate script component to entity with FENativeScriptComponent.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}
	
	FENativeScriptComponent& SourceComponent = SourceEntity->GetComponent<FENativeScriptComponent>();
	TargetEntity->AddComponent<FENativeScriptComponent>();
	NATIVE_SCRIPT_SYSTEM.InitializeComponentInternal(TargetEntity, TargetEntity->GetComponent<FENativeScriptComponent>(), SourceComponent.GetModuleID(), *SourceComponent.ScriptData);
	NATIVE_SCRIPT_SYSTEM.CopyVariableValuesInternal(&SourceComponent, &TargetEntity->GetComponent<FENativeScriptComponent>());
}

void FENativeScriptSystem::SaveVariableToJSON(Json::Value& Root, FEScriptVariableInfo& VariableInfo, FENativeScriptCore* Core)
{
	if (Core == nullptr)
	{
		LOG.Add("FENativeScriptSystem::SaveVariableToJSON Core is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return;
	}

	std::string VariableName = VariableInfo.Name;
	Root[VariableName]["Name"] = VariableName;
	Root[VariableName]["Type"] = VariableInfo.Type;

	if (VariableInfo.Type.find("vector<") != std::string::npos || VariableInfo.Type.find("std::vector<") != std::string::npos)
	{
		if (VariableInfo.Type.find("float>") != std::string::npos) SaveArrayVariableTTypeToJSON<float>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("int>") != std::string::npos) SaveArrayVariableTTypeToJSON<int>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("bool>") != std::string::npos) SaveArrayVariableTTypeToJSON<bool>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("std::string>") != std::string::npos) SaveArrayVariableTTypeToJSON<std::string>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("glm::vec2>") != std::string::npos) SaveArrayVariableTTypeToJSON<glm::vec2>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("glm::vec3>") != std::string::npos) SaveArrayVariableTTypeToJSON<glm::vec3>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type.find("glm::vec4>") != std::string::npos) SaveArrayVariableTTypeToJSON<glm::vec4>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
	}
	else
	{
		if (VariableInfo.Type == "int") SaveVariableTTypeToJSON<int>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "float") SaveVariableTTypeToJSON<float>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "double") SaveVariableTTypeToJSON<double>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "bool") SaveVariableTTypeToJSON<bool>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "glm::vec2") SaveVariableTTypeToJSON<glm::vec2>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "glm::vec3") SaveVariableTTypeToJSON<glm::vec3>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "glm::vec4") SaveVariableTTypeToJSON<glm::vec4>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "glm::quat") SaveVariableTTypeToJSON<glm::quat>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "string") SaveVariableTTypeToJSON<std::string>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
		else if (VariableInfo.Type == "FEPrefab*") SaveVariableTTypeToJSON<FEPrefab*>(Root[VariableName]["Value"], VariableInfo.Getter(Core));
	}
}

Json::Value FENativeScriptSystem::NativeScriptComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	Root["ModuleID"] = NativeScriptComponent.ModuleID;
	Root["DLLID"] = NATIVE_SCRIPT_SYSTEM.GetAssociatedDLLID(NativeScriptComponent.ModuleID);
	Root["Name"] = NativeScriptComponent.GetScriptData()->Name;

	Json::Value VariablesRoot;
	std::unordered_map<std::string, FEScriptVariableInfo> Variables = NATIVE_SCRIPT_SYSTEM.GetVariablesRegistry(Entity);

	auto VariableIterator = Variables.begin();
	while (VariableIterator != Variables.end())
	{
		NATIVE_SCRIPT_SYSTEM.SaveVariableToJSON(VariablesRoot, VariableIterator->second, NativeScriptComponent.GetCoreInstance());
		VariableIterator++;
	}
	Root["Variables"] = VariablesRoot;

	return Root;
}

void FENativeScriptSystem::LoadVariableFromJSON(Json::Value& Root, FEScriptVariableInfo& VariableInfo, FENativeScriptCore* Core)
{
	if (Core == nullptr)
	{
		LOG.Add("FENativeScriptSystem::LoadVariableFromJSON Core is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return;
	}

	std::string VariableName = VariableInfo.Name;
	if (VariableInfo.Type != Root["Type"].asString())
	{
		LOG.Add("FENativeScriptSystem::LoadVariableFromJSON failed to load variable with name: " + VariableName + " because types are different.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return;
	}

	if (VariableInfo.Type.find("vector<") != std::string::npos || VariableInfo.Type.find("std::vector<") != std::string::npos)
	{
		if (VariableInfo.Type.find("float>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<float>(Root["Value"]));
		else if (VariableInfo.Type.find("int>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<int>(Root["Value"]));
		else if (VariableInfo.Type.find("bool>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<bool>(Root["Value"]));
		else if (VariableInfo.Type.find("std::string>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<std::string>(Root["Value"]));
		else if (VariableInfo.Type.find("glm::vec2>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<glm::vec2>(Root["Value"]));
		else if (VariableInfo.Type.find("glm::vec3>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<glm::vec3>(Root["Value"]));
		else if (VariableInfo.Type.find("glm::vec4>") != std::string::npos) VariableInfo.Setter(Core, LoadArrayVariableTTypeToJSON<glm::vec4>(Root["Value"]));
	}
	else
	{
		if (VariableInfo.Type == "int") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<int>(Root["Value"]));
		else if (VariableInfo.Type == "float") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<float>(Root["Value"]));
		else if (VariableInfo.Type == "double") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<double>(Root["Value"]));
		else if (VariableInfo.Type == "bool") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<bool>(Root["Value"]));
		else if (VariableInfo.Type == "glm::vec2") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<glm::vec2>(Root["Value"]));
		else if (VariableInfo.Type == "glm::vec3") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<glm::vec3>(Root["Value"]));
		else if (VariableInfo.Type == "glm::vec4") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<glm::vec4>(Root["Value"]));
		else if (VariableInfo.Type == "glm::quat") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<glm::quat>(Root["Value"]));
		else if (VariableInfo.Type == "string") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<std::string>(Root["Value"]));
		else if (VariableInfo.Type == "FEPrefab*") VariableInfo.Setter(Core, LoadVariableTTypeToJSON<FEPrefab*>(Root["Value"]));
	}
}

void FENativeScriptSystem::NativeScriptComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::NativeScriptComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	std::string ModuleID = Root["ModuleID"].asString();
	if (ModuleID.empty())
	{
		LOG.Add("FENativeScriptSystem::NativeScriptComponentFromJson failed to get ModuleID from JSON.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	std::string ScriptName = Root["Name"].asString();
	if (ScriptName.empty())
	{
		LOG.Add("FENativeScriptSystem::NativeScriptComponentFromJson failed to get ScriptName from JSON.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	Entity->AddComponent<FENativeScriptComponent>();
	std::string DLLID = "";
	if (Root.isMember("DLLID"))
		DLLID = Root["DLLID"].asString();

	if (!NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(Entity, ModuleID, ScriptName, DLLID))
	{
		LOG.Add("FENativeScriptSystem::NativeScriptComponentFromJson failed to initialize script component.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		Entity->RemoveComponent<FENativeScriptComponent>();
		return;
	}

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
	std::unordered_map<std::string, FEScriptVariableInfo> Variables = NATIVE_SCRIPT_SYSTEM.GetVariablesRegistry(Entity);

	auto VariableIterator = Variables.begin();
	while (VariableIterator != Variables.end())
	{
		std::string VariableName = VariableIterator->first;
		Json::Value VariableRoot = Root["Variables"][VariableName];
		if (VariableRoot.isNull())
		{
			VariableIterator++;
			continue;
		}

		NATIVE_SCRIPT_SYSTEM.LoadVariableFromJSON(VariableRoot, VariableIterator->second, NativeScriptComponent.GetCoreInstance());
		VariableIterator++;
	}
}

bool FENativeScriptSystem::InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, std::string ActiveModuleID, FEScriptData& ScriptData)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeComponentInternal failed to add script component to null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModuleID.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeComponentInternal failed to add script component to entity with empty ModuleID.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	FEScene* Scene = Entity->GetParentScene();
	if (Scene == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeComponentInternal failed to add script component to entity without parent scene.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	
	NativeScriptComponent.CoreInstance = ScriptData.ConstructorFunction();
	NativeScriptComponent.ModuleID = ActiveModuleID;
	NativeScriptComponent.CoreInstance->ParentEntity = Entity;
	NativeScriptComponent.ScriptData = &ScriptData;

	if (Scene->HasFlag(FESceneFlag::EditorMode))
	{
		if (ScriptData.bRunInEditor)
			NativeScriptComponent.Awake();
	}
	else if (Scene->HasFlag(FESceneFlag::GameMode))
	{
		NativeScriptComponent.Awake();
	}

	return true;
}

FENativeScriptModule* FENativeScriptSystem::GetActiveModule(std::string ModuleID)
{
	if (ActiveModules.find(ModuleID) == ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::GetActiveModule failed to find active module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	return ActiveModules[ModuleID];
}

std::vector<std::string> FENativeScriptSystem::GetActiveModuleScriptNameList(std::string ModuleID)
{
	FENativeScriptModule* Module = GetActiveModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetActiveModuleScriptNameList failed to find active module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::vector<std::string>();
	}

	std::vector<std::string> Result;

	auto Iterator = Module->Registry.begin();
	while (Iterator != Module->Registry.end())
	{
		std::string ScriptName = Iterator->first;
		Result.push_back(ScriptName);

		Iterator++;
	}

	return Result;
}

std::vector<std::string> FENativeScriptSystem::GetActiveModuleIDList()
{
	std::vector<std::string> Result;

	auto Iterator = ActiveModules.begin();
	while (Iterator != ActiveModules.end())
	{
		std::string ModuleID = Iterator->first;
		Result.push_back(ModuleID);

		Iterator++;
	}

	return Result;
}

bool FENativeScriptSystem::InitializeScriptComponent(FEEntity* Entity, std::string ActiveModuleID, std::string ScriptName, std::string DLLID)
{
	if (Entity == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to null entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModuleID.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to entity with empty ActiveModuleID.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ScriptName.empty())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to add script component to entity with empty ScriptName.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	FENativeScriptModule* ActiveModule = GetActiveModule(ActiveModuleID);
	if (ActiveModule == nullptr)
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find active module with ID: " + ActiveModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);

		// If we can't find module ID, we will try to retrieve module by DLL module ID.
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent trying to find module with same DLL ID.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		if (DLLID.empty())
		{
			LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find active module with same DLL ID because DLLID is empty.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}

		ActiveModule = RESOURCE_MANAGER.GetNativeScriptModuleByDLLModuleID(DLLID);
		if (ActiveModule == nullptr)
		{
			LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find module with DLL ID: " + DLLID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
		else
		{
			// Because we found module by DLL ID, we should update ActiveModuleID.
			ActiveModuleID = ActiveModule->GetObjectID();
			LOG.Add("FENativeScriptSystem::InitializeScriptComponent found module with DLL ID: " + DLLID, "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
		}
	}

	if (ActiveModule->Registry.find(ScriptName) == ActiveModule->Registry.end())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed to find script with name: " + ScriptName + " in module with ID: " + ActiveModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!Entity->HasComponent<FENativeScriptComponent>())
	{
		LOG.Add("FENativeScriptSystem::InitializeScriptComponent failed because entity does not have FENativeScriptComponent.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	InitializeComponentInternal(Entity, Entity->GetComponent<FENativeScriptComponent>(), ActiveModuleID, ActiveModule->Registry[ScriptName]);
	return true;
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
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(NativeScriptComponent.ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find module with ID: " + NativeScriptComponent.ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	if (!Module->IsLoadedToMemory() || GetActiveModule(NativeScriptComponent.ModuleID) == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find active module with ID: " + NativeScriptComponent.ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	return Module->Registry[NativeScriptComponent.GetScriptData()->Name].VariablesRegistry;
}

std::unordered_map<std::string, FEScriptVariableInfo> FENativeScriptSystem::GetVariablesRegistry(std::string ModuleID, std::string ScriptName)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	if (!Module->IsLoadedToMemory() || GetActiveModule(ModuleID) == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find active module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	if (Module->Registry.find(ScriptName) == Module->Registry.end())
	{
		LOG.Add("FENativeScriptSystem::GetVariablesRegistry failed to find script with name: " + ScriptName + " in module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::unordered_map<std::string, FEScriptVariableInfo>();
	}

	return Module->Registry[ScriptName].VariablesRegistry;
}

bool FENativeScriptSystem::ActivateNativeScriptModule(std::string ModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModules.find(ModuleID) != ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because module with ID: " + ModuleID + " is already activated.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return ActivateNativeScriptModule(Module);
}

#include "../../ResourceManager/Timestamp.h"
bool FENativeScriptSystem::ActivateNativeScriptModule(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to activate null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModules.find(Module->GetObjectID()) != ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because module with ID: " + Module->GetObjectID() + " is already activated.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (Module->IsLoadedToMemory())
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because module with ID: " + Module->GetObjectID() + " is already loaded to memory.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// First we need to extract the DLL and PDB files to a temporary directory.
	std::string ExtractedFolderPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/ExtractedNativeScripts/";
	if (!FILE_SYSTEM.DoesDirectoryExist(ExtractedFolderPath))
		FILE_SYSTEM.CreateDirectory(ExtractedFolderPath);

	ExtractedFolderPath += Module->GetObjectID() + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(ExtractedFolderPath))
		FILE_SYSTEM.CreateDirectory(ExtractedFolderPath);

	FEAssetPackageAssetInfo AssetInfo;

#ifdef _DEBUG
	AssetInfo = Module->ScriptAssetPackage->GetAssetInfo(Module->DebugDLLAssetID);
	std::string DLLPath = ExtractedFolderPath + AssetInfo.Name;
	if (!Module->ScriptAssetPackage->ExportAssetToFile(Module->DebugDLLAssetID, DLLPath))
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to extract DLL file from asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::string PDBPath = "";
	if (!Module->DebugPDBAssetID.empty())
	{
		AssetInfo = Module->ScriptAssetPackage->GetAssetInfo(Module->DebugPDBAssetID);
		if (!Module->ScriptAssetPackage->ExportAssetToFile(Module->DebugPDBAssetID, ExtractedFolderPath + AssetInfo.Name))
		{
			LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to extract PDB file from asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	Module->ExtractedPDBPath = PDBPath;
#else
	AssetInfo = Module->ScriptAssetPackage->GetAssetInfo(Module->ReleaseDLLAssetID);
	std::string DLLPath = ExtractedFolderPath + AssetInfo.Name;
	if (!Module->ScriptAssetPackage->ExportAssetToFile(Module->ReleaseDLLAssetID, DLLPath))
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to extract DLL file from asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
#endif
	
	HMODULE DLLHandle = LoadLibraryA(DLLPath.c_str());
	if (!DLLHandle)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to load DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Ensuring that the DLL has the required functions.
	typedef char* (*Get_ModuleID_Function)(void);
	Get_ModuleID_Function GetModuleID = (Get_ModuleID_Function)GetProcAddress(DLLHandle, "GetModuleID");
	if (!GetModuleID)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get GetModuleID function from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	typedef bool (*IsCompiledInDebugMode_Function)(void);
	IsCompiledInDebugMode_Function IsCompiledInDebugMode = (IsCompiledInDebugMode_Function)GetProcAddress(DLLHandle, "IsCompiledInDebugMode");
	if (IsCompiledInDebugMode)
	{
		bool DLLInDebug = IsCompiledInDebugMode();

#ifdef _DEBUG
		if (!DLLInDebug)
		{
			LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because DLL with path: " + DLLPath + " was compiled in release mode, while engine is in debug mode.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			FreeLibrary(DLLHandle);
			return false;
		}
#else
		if (DLLInDebug)
		{
			LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because DLL with path: " + DLLPath + " was compiled in debug mode, while engine is in release mode.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			FreeLibrary(DLLHandle);
			return false;
		}
#endif
	}

	typedef unsigned long long (*Get_EngineHeaders_BuildVersion_Function)(void);
	Get_EngineHeaders_BuildVersion_Function GetEngineHeadersBuildVersion = (Get_EngineHeaders_BuildVersion_Function)GetProcAddress(DLLHandle, "GetEngineHeadersBuildVersion");
	if (GetEngineHeadersBuildVersion)
	{
		unsigned long long EngineHeadersBuildVersion = GetEngineHeadersBuildVersion();
		unsigned long long EngineBuildVersion = std::stoull(ENGINE_BUILD_TIMESTAMP);
		if (EngineHeadersBuildVersion != EngineBuildVersion)
		{
			// Currently we will just log this error, but in the future we should handle this more gracefully.
			LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule is loading DLL with path: " + DLLPath + " that was compiled with different engine headers version, that could lead to crashes.", "FE_SCRIPT_SYSTEM", FE_LOG_WARNING);
			//LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed because DLL with path: " + DLLPath + " was compiled with different engine headers version.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			//FreeLibrary(DLLHandle);
			//return false;
		}
	}

	std::string DLLModuleID = GetModuleID();
	if (DLLModuleID.empty() || DLLModuleID.size() != 24)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get proper DLLModuleID from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	typedef void* (*Get_Script_Registry_Function)(void);
	Get_Script_Registry_Function GetScriptRegistry = (Get_Script_Registry_Function)GetProcAddress(DLLHandle, "GetScriptRegistry");
	if (!GetScriptRegistry)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get GetScriptRegistry function from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	void* RawDLLRegistry = GetScriptRegistry();
	if (!RawDLLRegistry)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get ScriptRegistry from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	std::unordered_map<std::string, FEScriptData>* DLLRegistry = (std::unordered_map<std::string, FEScriptData>*)RawDLLRegistry;

	Module->bIsLoadedToMemory = true;
	Module->DLLHandle = DLLHandle;
	Module->ExtractedDLLPath = DLLPath;
	Module->DLLModuleID = DLLModuleID;

	// Manually copy the registry to the loaded modules.
	auto Iterator = DLLRegistry->begin();
	while (Iterator != DLLRegistry->end())
	{
		Module->Registry[Iterator->first].bRunInEditor = Iterator->second.bRunInEditor;
		Module->Registry[Iterator->first].Name = Iterator->second.Name;
		Module->Registry[Iterator->first].ConstructorFunction = Iterator->second.ConstructorFunction;
		Module->Registry[Iterator->first].VariablesRegistry = Iterator->second.VariablesRegistry;
		Iterator++;
	}

	ActiveModules[Module->GetObjectID()] = Module;
	return true;
}

bool FENativeScriptSystem::DeactivateNativeScriptModule(std::string ModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DeactivateNativeScriptModule failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModules.find(ModuleID) == ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::DeactivateNativeScriptModule failed because module with ID: " + ModuleID + " is not activated.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return DeactivateNativeScriptModule(Module);
}

bool FENativeScriptSystem::DeactivateNativeScriptModule(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DeactivateNativeScriptModule failed to deactivate null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModules.find(Module->GetObjectID()) == ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::DeactivateNativeScriptModule failed because module with ID: " + Module->GetObjectID() + " is not activated.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!Module->IsLoadedToMemory())
	{
		LOG.Add("FENativeScriptSystem::DeactivateNativeScriptModule failed because module with ID: " + Module->GetObjectID() + " is not loaded to memory.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// We should delete all script components associated with this module.
	ComponentsClearOnModuleDeactivate(Module);
	Module->Registry.clear();

	FreeLibrary(Module->DLLHandle);
	Module->bIsLoadedToMemory = false;
	Module->DLLHandle = nullptr;
	if (!Module->ExtractedDLLPath.empty())
	{
		FILE_SYSTEM.DeleteFile(Module->ExtractedDLLPath);
		Module->ExtractedDLLPath = "";
	}
		
	if (!Module->ExtractedPDBPath.empty())
	{
		FILE_SYSTEM.DeleteFile(Module->ExtractedPDBPath);
		Module->ExtractedPDBPath = "";
	}

	std::string ExtractedFolderPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/ExtractedNativeScripts/";
	ExtractedFolderPath += Module->GetObjectID() + "/";
	if (FILE_SYSTEM.DoesDirectoryExist(ExtractedFolderPath))
		FILE_SYSTEM.DeleteDirectory(ExtractedFolderPath);

	Module->DLLModuleID = "";

	ActiveModules.erase(Module->GetObjectID());
	return true;
}

void FENativeScriptSystem::RemoveComponentsFromScene(FEScene* Scene, std::string ModuleID)
{
	std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

	for (size_t i = 0; i < Entities.size(); i++)
	{
		FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
		if (NativeScriptComponent.ModuleID == ModuleID)
		{
			NativeScriptComponent.OnDestroy();
			Entities[i]->RemoveComponent<FENativeScriptComponent>();
		}
	}
}

void FENativeScriptSystem::ComponentsClearOnModuleDeactivate(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::ComponentsClearOnModuleDeactivate failed to clear components on null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	for (FEScene* Scene : ActiveScenes)
		RemoveComponentsFromScene(Scene, Module->GetObjectID());

	// Besides active scenes, we should also check prefab internal scenes.
	
	// TO DO: That is interesting way of handling prefabs scenes, but I am not sure if it is the best way.
	//std::vector<FEScene*> PrefabInternalScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::PrefabInternal);

	std::vector<std::string> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
	for (size_t i = 0; i < PrefabIDList.size(); i++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
		FEScene* PrefabInternalScene = CurrentPrefab->GetScene();

		if (PrefabInternalScene == nullptr)
			continue;

		RemoveComponentsFromScene(PrefabInternalScene, Module->GetObjectID());
	}
}

void FENativeScriptSystem::GetModuleScriptInstancesFromScene(std::vector<FEModuleScriptInstance>& Result, FEScene* Scene, std::string ModuleID)
{
	if (Scene == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetModuleScriptInstancesFromScene failed to get components of null scene.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

	for (size_t i = 0; i < Entities.size(); i++)
	{
		FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
		if (NativeScriptComponent.ModuleID == ModuleID)
		{
			FEModuleScriptInstance Instance;
			Instance.Scene = Scene;
			Instance.Entity = Entities[i];
			//Instance.NativeScriptComponent = &NativeScriptComponent;
			Instance.ScriptName = NativeScriptComponent.GetScriptData()->Name;

			Result.push_back(Instance);
		}
	}
}

// Returns array of information about components associated with module.
std::vector<FEModuleScriptInstance> FENativeScriptSystem::GetModuleScriptInstances(FENativeScriptModule* Module)
{
	std::vector<FEModuleScriptInstance> Result;

	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetModuleScriptInstances failed to get components of null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return Result;
	}

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	for (FEScene* Scene : ActiveScenes)
		GetModuleScriptInstancesFromScene(Result, Scene, Module->GetObjectID());

	// Besides active scenes, we should also check prefab internal scenes.
	std::vector<std::string> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
	for (size_t i = 0; i < PrefabIDList.size(); i++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
		FEScene* PrefabInternalScene = CurrentPrefab->GetScene();

		if (PrefabInternalScene == nullptr)
			continue;

		GetModuleScriptInstancesFromScene(Result, PrefabInternalScene, Module->GetObjectID());
	}

	return Result;
}

void FENativeScriptSystem::DeleteNativeScriptModule(std::string ModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DeleteNativeScriptModule failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	DeleteNativeScriptModule(Module);
}

void FENativeScriptSystem::DeleteNativeScriptModule(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::DeleteNativeScriptModule failed to delete null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (ActiveModules.find(Module->GetObjectID()) != ActiveModules.end())
		DeactivateNativeScriptModule(Module);

	RESOURCE_MANAGER.DeleteNativeScriptModuleInternal(Module);
}

bool FENativeScriptSystem::UpdateNativeScriptModule(std::string CurrentModuleID, std::string UpdatedModuleID)
{
	FENativeScriptModule* CurrentModule = RESOURCE_MANAGER.GetNativeScriptModule(CurrentModuleID);
	if (CurrentModule == nullptr)
	{
		LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to find current module with ID: " + CurrentModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	FENativeScriptModule* UpdatedModule = RESOURCE_MANAGER.GetNativeScriptModule(UpdatedModuleID);
	if (UpdatedModule == nullptr)
	{
		LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to find updated module with ID: " + UpdatedModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (ActiveModules.find(CurrentModuleID) == ActiveModules.end())
	{
		LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to find active module with ID: " + CurrentModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return UpdateNativeScriptModule(CurrentModule, UpdatedModule);
}

std::any FENativeScriptSystem::CreateEngineLocalScriptVariableCopy(FEScriptVariableInfo& Info, std::any Value)
{
	// TO-DO: Make it more general with more templated magic.
	if (Info.Type == "float") return CreateEngineLocalScriptVariableCopyTemplated<float>(Value);
	else if (Info.Type == "int") return CreateEngineLocalScriptVariableCopyTemplated<int>(Value);
	else if (Info.Type == "bool") return CreateEngineLocalScriptVariableCopyTemplated<bool>(Value);
	else if (Info.Type == "std::string") return CreateEngineLocalScriptVariableCopyTemplated<std::string>(Value);
	else if (Info.Type == "glm::vec2") return CreateEngineLocalScriptVariableCopyTemplated<glm::vec2>(Value);
	else if (Info.Type == "glm::vec3") return CreateEngineLocalScriptVariableCopyTemplated<glm::vec3>(Value);
	else if (Info.Type == "glm::vec4") return CreateEngineLocalScriptVariableCopyTemplated<glm::vec4>(Value);
	else if (Info.Type == "glm::quat") return CreateEngineLocalScriptVariableCopyTemplated<glm::quat>(Value);
	// ************************ FEObject children ************************
	else if (Info.Type == "FEShader*") return CreateEngineLocalScriptVariableCopyTemplated<FEShader*>(Value);
	else if (Info.Type == "FEMesh*") return CreateEngineLocalScriptVariableCopyTemplated<FEMesh*>(Value);
	else if (Info.Type == "FETexture*") return CreateEngineLocalScriptVariableCopyTemplated<FETexture*>(Value);
	else if (Info.Type == "FEMaterial*") return CreateEngineLocalScriptVariableCopyTemplated<FEMaterial*>(Value);
	else if (Info.Type == "FEGameModel*") return CreateEngineLocalScriptVariableCopyTemplated<FEGameModel*>(Value);
	// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
	//else if (Info.Type == "FEEntity*") return CreateLocalCopy<FEEntity*>(Value);
	else if (Info.Type == "FEFramebuffer*") return CreateEngineLocalScriptVariableCopyTemplated<FEFramebuffer*>(Value);
	else if (Info.Type == "FEPostProcess*") return CreateEngineLocalScriptVariableCopyTemplated<FEPostProcess*>(Value);
	else if (Info.Type == "FEPrefab*") return CreateEngineLocalScriptVariableCopyTemplated<FEPrefab*>(Value);
	// TO-DO: Check if it is working.
	else if (Info.Type == "FEScene*") return CreateEngineLocalScriptVariableCopyTemplated<FEScene*>(Value);
	else if (Info.Type == "FEAssetPackage*") return CreateEngineLocalScriptVariableCopyTemplated<FEAssetPackage*>(Value);
	else if (Info.Type == "FENativeScriptModule*") return CreateEngineLocalScriptVariableCopyTemplated<FENativeScriptModule*>(Value);
	// ************************ Vectors ************************
	else if (Info.Type == "std::vector<float>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<float>>(Value);
	else if (Info.Type == "std::vector<int>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<int>>(Value);
	else if (Info.Type == "std::vector<bool>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<bool>>(Value);
	else if (Info.Type == "std::vector<std::string>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<std::string>>(Value);
	else if (Info.Type == "std::vector<glm::vec2>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<glm::vec2>>(Value);
	else if (Info.Type == "std::vector<glm::vec3>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<glm::vec3>>(Value);
	else if (Info.Type == "std::vector<glm::vec4>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<glm::vec4>>(Value);
	else if (Info.Type == "std::vector<glm::quat>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<glm::quat>>(Value);
	// ************************ FEObject children ************************
	else if (Info.Type == "std::vector<FEShader*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEShader*>>(Value);
	else if (Info.Type == "std::vector<FEMesh*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEMesh*>>(Value);
	else if (Info.Type == "std::vector<FETexture*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FETexture*>>(Value);
	else if (Info.Type == "std::vector<FEMaterial*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEMaterial*>>(Value);
	else if (Info.Type == "std::vector<FEGameModel*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEGameModel*>>(Value);
	// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
	//else if (Info.Type == "std::vector<FEEntity*>") return CreateLocalCopy<std::vector<FEEntity*>>(Value);
	else if (Info.Type == "std::vector<FEFramebuffer*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEFramebuffer*>>(Value);
	else if (Info.Type == "std::vector<FEPostProcess*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEPostProcess*>>(Value);
	else if (Info.Type == "std::vector<FEPrefab*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEPrefab*>>(Value);
	// TO-DO: Check if it is working.
	else if (Info.Type == "std::vector<FEScene*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEScene*>>(Value);
	else if (Info.Type == "std::vector<FEAssetPackage*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FEAssetPackage*>>(Value);
	else if (Info.Type == "std::vector<FENativeScriptModule*>") return CreateEngineLocalScriptVariableCopyTemplated<std::vector<FENativeScriptModule*>>(Value);

	// If we can't find the type, we will return empty any.
	return std::any();
}

void FENativeScriptSystem::CheckForAlteredVariables(std::vector<FEModuleScriptInstance>& ModuleScriptInstancesToUpdate)
{
	for (size_t i = 0; i < ModuleScriptInstancesToUpdate.size(); i++)
	{
		FENativeScriptComponent& NativeScriptComponent = ModuleScriptInstancesToUpdate[i].Entity->GetComponent<FENativeScriptComponent>();
		std::unordered_map<std::string, FEScriptVariableInfo> Variables = NATIVE_SCRIPT_SYSTEM.GetVariablesRegistry(ModuleScriptInstancesToUpdate[i].Entity);

		FEScriptData* CurrentScriptData = NativeScriptComponent.ScriptData;
		// We need to create an instance of the original core.
		// To have access to the original variable values.
		FENativeScriptCore* OriginalCoreInstance = CurrentScriptData->ConstructorFunction();

		auto VariableIterator = Variables.begin();
		while (VariableIterator != Variables.end())
		{
			FEScriptVariableInfo& VariableInfo = VariableIterator->second;

			std::any OriginalValue = VariableInfo.Getter(OriginalCoreInstance);
			std::any CurrentValue = NativeScriptComponent.GetVariableValueRaw(VariableInfo.Name);

			if (CurrentScriptData->VariablesRegistry.find(VariableInfo.Name) != CurrentScriptData->VariablesRegistry.end())
			{
				if (!IsEqualScriptVariable(VariableInfo, OriginalValue, CurrentValue))
				{
					FEAlteredScriptVariable AlteredVariable;
					AlteredVariable.Name = VariableInfo.Name;

					// We need to create a local(Engine local, not DLL resident) copy of the altered variable.
					// Because after we unload the DLL, we will lose access to the original variable.
					AlteredVariable.AlteredValue = CreateEngineLocalScriptVariableCopy(VariableInfo, CurrentValue);
					
					if (!AlteredVariable.AlteredValue.has_value())
					{
						LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to create local copy of altered variable.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
						VariableIterator++;
						continue;
					}

					ModuleScriptInstancesToUpdate[i].AlteredVariables.push_back(AlteredVariable);
				}

			}

			VariableIterator++;
		}

		// Now we can delete the original core instance.
		delete OriginalCoreInstance;
	}
}

bool FENativeScriptSystem::UpdateNativeScriptModule(FENativeScriptModule* CurrentModule, FENativeScriptModule* UpdatedModule)
{
	if (CurrentModule == nullptr)
	{
		LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to update null current module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (UpdatedModule == nullptr)
	{
		LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to update null updated module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// First we need to get the list of components associated with the current module.
	std::vector<FEModuleScriptInstance> ComponentToRestore = GetModuleScriptInstances(CurrentModule);
	// Now we need to loop through the components and save script variables that was altered by user.
	CheckForAlteredVariables(ComponentToRestore);

	// Then we can deactivate and delete current module.
	DeactivateNativeScriptModule(CurrentModule);
	DeleteNativeScriptModule(CurrentModule);

	// And activate updated module.
	ActivateNativeScriptModule(UpdatedModule);

	// Now we can restore the components and altered variables.
	bool bAtLeastOneComponentRestored = false;
	for (size_t i = 0; i < ComponentToRestore.size(); i++)
	{
		ComponentToRestore[i].Entity->AddComponent<FENativeScriptComponent>();
		if (InitializeScriptComponent(ComponentToRestore[i].Entity, UpdatedModule->GetObjectID(), ComponentToRestore[i].ScriptName))
		{
			bAtLeastOneComponentRestored = true;

			for (size_t j = 0; j < ComponentToRestore[i].AlteredVariables.size(); j++)
			{
				FENativeScriptComponent& NativeScriptComponent = ComponentToRestore[i].Entity->GetComponent<FENativeScriptComponent>();
				NativeScriptComponent.SetVariableValue(ComponentToRestore[i].AlteredVariables[j].Name, ComponentToRestore[i].AlteredVariables[j].AlteredValue);
			}
		}
		else
		{
			LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to restore component with script name: " + ComponentToRestore[i].ScriptName + " to entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			ComponentToRestore[i].Entity->RemoveComponent<FENativeScriptComponent>();
		}
	}

	return bAtLeastOneComponentRestored;
}

std::string FENativeScriptSystem::GetAssociatedDLLID(std::string ScriptModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ScriptModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetAssociatedDLLID failed to find module with ID: " + ScriptModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return "";
	}

	return Module->DLLModuleID;
}

std::string FENativeScriptSystem::GetAssociatedScriptModuleID(std::string DLLModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModuleByDLLModuleID(DLLModuleID);
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetAssociatedScriptModuleID failed to find module with DLL ID: " + DLLModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return "";
	}

	return Module->GetObjectID();
}

bool FENativeScriptSystem::IsEqualScriptVariable(FEScriptVariableInfo& VariableInfo, std::any FirstScriptVariable, std::any SecondScriptVariable)
{
	if (VariableInfo.Type.find("vector<") != std::string::npos || VariableInfo.Type.find("std::vector<") != std::string::npos)
	{
		//return true;
		if (VariableInfo.Type.find("float>") != std::string::npos) return IsEqualArrayTType<float>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("int>") != std::string::npos) return IsEqualArrayTType<int>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("bool>") != std::string::npos) return IsEqualArrayTType<bool>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("std::string>") != std::string::npos) return IsEqualArrayTType<std::string>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("glm::vec2>") != std::string::npos) return IsEqualArrayTType<glm::vec2>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("glm::vec3>") != std::string::npos) return IsEqualArrayTType<glm::vec3>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("glm::vec4>") != std::string::npos) return IsEqualArrayTType<glm::vec4>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("glm::quat>") != std::string::npos) return IsEqualArrayTType<glm::quat>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type.find("FEPrefab*>") != std::string::npos) return IsEqualArrayTType<FEPrefab*>(FirstScriptVariable, SecondScriptVariable);
	}
	else
	{
		if (VariableInfo.Type == "int") return IsEqualTType<int>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "float") return IsEqualTType<float>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "double") return IsEqualTType<double>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "bool") return IsEqualTType<bool>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "glm::vec2") return IsEqualTType<glm::vec2>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "glm::vec3") return IsEqualTType<glm::vec3>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "glm::vec4") return IsEqualTType<glm::vec4>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "glm::quat") return IsEqualTType<glm::quat>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "string") return IsEqualTType<std::string>(FirstScriptVariable, SecondScriptVariable);
		else if (VariableInfo.Type == "FEPrefab*") return IsEqualTType<FEPrefab*>(FirstScriptVariable, SecondScriptVariable);
	}

	return false;
}