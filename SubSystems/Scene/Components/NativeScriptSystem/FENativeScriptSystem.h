#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	struct FEModuleScriptInstance
	{
		FEScene* Scene;
		FEEntity* Entity;
		FENativeScriptComponent* NativeScriptComponent;
		std::string ScriptName;
	};

	class FOCAL_ENGINE_API FENativeScriptSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FENativeScriptSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void Update(double DeltaTime);

		static Json::Value NativeScriptComponentToJson(FEEntity* Entity);
		static void NativeScriptComponentFromJson(FEEntity* Entity, Json::Value Root);
		static void DuplicateNativeScriptComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		bool InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, std::string ActiveModuleID, FEScriptData& ScriptData);
		FENativeScriptModule* GetActiveModule(std::string ModuleID);

		void CopyVariableValuesInternal(FENativeScriptComponent* SourceComponent, FENativeScriptComponent* TargetComponent);

		// Returns array of information about components associated with module.
		std::vector<FEModuleScriptInstance> GetComponentsOfModule(FENativeScriptModule* Module);

		// We should delete all script components associated with module.
		void ComponentsClearOnModuleDeactivate(FENativeScriptModule* Module);
	public:
		SINGLETON_PUBLIC_PART(FENativeScriptSystem)

		std::unordered_map<std::string, FENativeScriptModule*> ActiveModules;

		bool ActivateNativeScriptModule(std::string ModuleID);
		bool ActivateNativeScriptModule(FENativeScriptModule* Module);

		bool DeactivateNativeScriptModule(std::string ModuleID);
		bool DeactivateNativeScriptModule(FENativeScriptModule* Module);

		void DeleteNativeScriptModule(std::string ModuleID);
		void DeleteNativeScriptModule(FENativeScriptModule* Module);

		// Try to update module with new one. It will succeed only if new module has some script names that old module has.
		// Should be used only for hot-reloading of new version of same module.
		bool UpdateNativeScriptModule(std::string CurrentModuleID, std::string UpdatedModuleID);
		// Try to update module with new one. It will succeed only if new module has some script names that old module has.
		// Should be used only for hot-reloading of new version of same module.
		bool UpdateNativeScriptModule(FENativeScriptModule* CurrentModule, FENativeScriptModule* UpdatedModule);

		std::vector<std::string> GetActiveModuleIDList();
		std::vector<std::string> GetActiveModuleScriptNameList(std::string ModuleID);

		bool InitializeScriptComponent(FEEntity* Entity, std::string ActiveModuleID, std::string ScriptName);

		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(FEEntity* Entity);
		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(std::string ModuleID, std::string ScriptName);
		
		std::string GetDLLMoudleIDByNativeScriptModuleID(std::string ModuleID);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetNativeScriptSystem();
	#define NATIVE_SCRIPT_SYSTEM (*static_cast<FENativeScriptSystem*>(GetNativeScriptSystem()))
#else
	#define NATIVE_SCRIPT_SYSTEM FENativeScriptSystem::GetInstance()
#endif
}