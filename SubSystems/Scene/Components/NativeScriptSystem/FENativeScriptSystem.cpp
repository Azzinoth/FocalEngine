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

	if (TargetComponent == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CopyVariableValuesInternal failed to copy variable values to null component.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
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

Json::Value FENativeScriptSystem::NativeScriptComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	Root["ModuleID"] = NativeScriptComponent.ModuleID;
	Root["Name"] = NativeScriptComponent.GetScriptData()->Name;

	Json::Value VariablesRoot;
	std::unordered_map<std::string, FEScriptVariableInfo> Variables = NATIVE_SCRIPT_SYSTEM.GetVariablesRegistry(Entity);

	auto VariableIterator = Variables.begin();
	while (VariableIterator != Variables.end())
	{
		std::string VariableName = VariableIterator->first;
		VariablesRoot[VariableName]["Name"] = VariableName;
		VariablesRoot[VariableName]["Type"] = VariableIterator->second.Type;

		FEScriptVariableInfo VariableInfo = VariableIterator->second;
		std::any VariableValue = VariableInfo.Getter(NativeScriptComponent.GetCoreInstance());

		if (VariableInfo.Type == "int")
		{
			int Value = std::any_cast<int>(VariableValue);
			VariablesRoot[VariableName]["Value"] = Value;
		}
		else if (VariableInfo.Type == "float")
		{
			float Value = std::any_cast<float>(VariableValue);
			VariablesRoot[VariableName]["Value"] = Value;
		}
		else if (VariableInfo.Type == "double")
		{
			double Value = std::any_cast<double>(VariableValue);
			VariablesRoot[VariableName]["Value"] = Value;
		}
		else if (VariableInfo.Type == "bool")
		{
			bool Value = std::any_cast<bool>(VariableValue);
			VariablesRoot[VariableName]["Value"] = Value;
		}
		else if (VariableInfo.Type == "glm::vec2")
		{
			glm::vec2 Value = std::any_cast<glm::vec2>(VariableValue);
			VariablesRoot[VariableName]["Value"]["X"] = Value.x;
			VariablesRoot[VariableName]["Value"]["Y"] = Value.y;
		}
		else if (VariableInfo.Type == "glm::vec3")
		{
			glm::vec3 Value = std::any_cast<glm::vec3>(VariableValue);
			VariablesRoot[VariableName]["Value"]["X"] = Value.x;
			VariablesRoot[VariableName]["Value"]["Y"] = Value.y;
			VariablesRoot[VariableName]["Value"]["Z"] = Value.z;
		}
		else if (VariableInfo.Type == "glm::vec4")
		{
			glm::vec4 Value = std::any_cast<glm::vec4>(VariableValue);
			VariablesRoot[VariableName]["Value"]["X"] = Value.x;
			VariablesRoot[VariableName]["Value"]["Y"] = Value.y;
			VariablesRoot[VariableName]["Value"]["Z"] = Value.z;
			VariablesRoot[VariableName]["Value"]["W"] = Value.w;
		}
		else if (VariableInfo.Type == "glm::quat")
		{
			glm::quat Value = std::any_cast<glm::quat>(VariableValue);
			VariablesRoot[VariableName]["Value"]["X"] = Value.x;
			VariablesRoot[VariableName]["Value"]["Y"] = Value.y;
			VariablesRoot[VariableName]["Value"]["Z"] = Value.z;
			VariablesRoot[VariableName]["Value"]["W"] = Value.w;
		}
		else if (VariableInfo.Type == "std::string")
		{
			std::string Value = std::any_cast<std::string>(VariableValue);
			VariablesRoot[VariableName]["Value"] = Value;
		}

		VariableIterator++;
	}
	Root["Variables"] = VariablesRoot;

	return Root;
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
	if (!NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(Entity, ModuleID, ScriptName))
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
		FEScriptVariableInfo VariableInfo = VariableIterator->second;

		Json::Value VariableRoot = Root["Variables"][VariableName];

		if (VariableRoot.isNull())
		{
			VariableIterator++;
			continue;
		}

		if (VariableInfo.Type != VariableRoot["Type"].asString())
		{
			VariableIterator++;
			continue;
		}

		if (VariableInfo.Type == "int")
		{
			int Value = VariableRoot["Value"].asInt();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "float")
		{
			float Value = VariableRoot["Value"].asFloat();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "double")
		{
			double Value = VariableRoot["Value"].asDouble();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "bool")
		{
			bool Value = VariableRoot["Value"].asBool();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "glm::vec2")
		{
			glm::vec2 Value;
			Value.x = VariableRoot["Value"]["X"].asFloat();
			Value.y = VariableRoot["Value"]["Y"].asFloat();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "glm::vec3")
		{
			glm::vec3 Value;
			Value.x = VariableRoot["Value"]["X"].asFloat();
			Value.y = VariableRoot["Value"]["Y"].asFloat();
			Value.z = VariableRoot["Value"]["Z"].asFloat();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "glm::vec4")
		{
			glm::vec4 Value;
			Value.x = VariableRoot["Value"]["X"].asFloat();
			Value.y = VariableRoot["Value"]["Y"].asFloat();
			Value.z = VariableRoot["Value"]["Z"].asFloat();
			Value.w = VariableRoot["Value"]["W"].asFloat();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "glm::quat")
		{
			glm::quat Value;
			Value.x = VariableRoot["Value"]["X"].asFloat();
			Value.y = VariableRoot["Value"]["Y"].asFloat();
			Value.z = VariableRoot["Value"]["Z"].asFloat();
			Value.w = VariableRoot["Value"]["W"].asFloat();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}
		else if (VariableInfo.Type == "std::string")
		{
			std::string Value = VariableRoot["Value"].asString();
			VariableInfo.Setter(NativeScriptComponent.GetCoreInstance(), Value);
		}

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
	
	NativeScriptComponent.CoreInstance = ScriptData.ConstructorFunction();
	NativeScriptComponent.ModuleID = ActiveModuleID;
	NativeScriptComponent.CoreInstance->ParentEntity = Entity;
	NativeScriptComponent.ScriptData = &ScriptData;

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

bool FENativeScriptSystem::InitializeScriptComponent(FEEntity* Entity, std::string ActiveModuleID, std::string ScriptName)
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
		return false;
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

	FEAssetPackageAssetInfo AssetInfo = Module->ScriptAssetPackage->GetAssetInfo(Module->DLLAssetID);
	std::string DLLPath = ExtractedFolderPath + AssetInfo.Name;
	if (!Module->ScriptAssetPackage->ExportAssetToFile(Module->DLLAssetID, DLLPath))
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to extract DLL file from asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::string PDBPath = "";
	if (!Module->PDBAssetID.empty())
	{
		AssetInfo = Module->ScriptAssetPackage->GetAssetInfo(Module->PDBAssetID);
		if (!Module->ScriptAssetPackage->ExportAssetToFile(Module->PDBAssetID, ExtractedFolderPath + AssetInfo.Name))
		{
			LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to extract PDB file from asset package.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

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
		return false;
	}

	std::string DLLModuleID = GetModuleID();
	if (DLLModuleID.empty() || DLLModuleID.size() != 24)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get proper DLLModuleID from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	typedef void* (*Get_Script_Registry_Function)(void);
	Get_Script_Registry_Function GetScriptRegistry = (Get_Script_Registry_Function)GetProcAddress(DLLHandle, "GetScriptRegistry");
	if (!GetScriptRegistry)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get GetScriptRegistry function from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	void* RawDLLRegistry = GetScriptRegistry();
	if (!RawDLLRegistry)
	{
		LOG.Add("FENativeScriptSystem::ActivateNativeScriptModule failed to get ScriptRegistry from DLL: " + DLLPath, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::unordered_map<std::string, FEScriptData>* DLLRegistry = (std::unordered_map<std::string, FEScriptData>*)RawDLLRegistry;

	Module->bIsLoadedToMemory = true;
	Module->DLLHandle = DLLHandle;
	Module->ExtractedDLLPath = DLLPath;
	Module->ExtractedPDBPath = PDBPath;
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

void FENativeScriptSystem::ComponentsClearOnModuleDeactivate(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::ComponentsClearOnModuleDeactivate failed to clear components on null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return;
	}

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	for (FEScene* Scene : ActiveScenes)
	{
		std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.ModuleID == Module->GetObjectID())
			{
				NativeScriptComponent.OnDestroy();
				Entities[i]->RemoveComponent<FENativeScriptComponent>();
			}
		}
	}
}

// Returns array of information about components associated with module.
std::vector<FEModuleScriptInstance> FENativeScriptSystem::GetComponentsOfModule(FENativeScriptModule* Module)
{
	std::vector<FEModuleScriptInstance> Result;

	if (Module == nullptr)
	{
		LOG.Add("FENativeScriptSystem::GetComponentsOfModule failed to get components of null module.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return Result;
	}

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	for (FEScene* Scene : ActiveScenes)
	{
		std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.ModuleID == Module->GetObjectID())
			{
				FEModuleScriptInstance Instance;
				Instance.Scene = Scene;
				Instance.Entity = Entities[i];
				Instance.NativeScriptComponent = &NativeScriptComponent;
				Instance.ScriptName = NativeScriptComponent.GetScriptData()->Name;

				Result.push_back(Instance);
			}
		}
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
	std::vector<FEModuleScriptInstance> ComponentToRestore = GetComponentsOfModule(CurrentModule);

	// Then we can deactivate and delete current module.
	DeactivateNativeScriptModule(CurrentModule);
	DeleteNativeScriptModule(CurrentModule);

	// And activate updated module.
	ActivateNativeScriptModule(UpdatedModule);

	// Now we can restore the components.
	bool bAtLeastOneComponentRestored = false;
	for (size_t i = 0; i < ComponentToRestore.size(); i++)
	{
		ComponentToRestore[i].Entity->AddComponent<FENativeScriptComponent>();
		if (InitializeScriptComponent(ComponentToRestore[i].Entity, UpdatedModule->GetObjectID(), ComponentToRestore[i].ScriptName))
		{
			bAtLeastOneComponentRestored = true;
		}
		else
		{
			LOG.Add("FENativeScriptSystem::UpdateNativeScriptModule failed to restore component with script name: " + ComponentToRestore[i].ScriptName + " to entity.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
			ComponentToRestore[i].Entity->RemoveComponent<FENativeScriptComponent>();
		}
	}

	return bAtLeastOneComponentRestored;
}

std::string FENativeScriptSystem::GetDLLMoudleIDByNativeScriptModuleID(std::string ModuleID)
{
	FENativeScriptModule* Module = RESOURCE_MANAGER.GetNativeScriptModule(ModuleID);
	if (Module == nullptr)
	{
		LOG.Add("GetDLLMoudleIDByNativeScriptModuleID failed to find module with ID: " + ModuleID, "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return "";
	}

	return Module->DLLModuleID;
}