#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	struct FEAlteredScriptVariable
	{
		std::string Name;
		std::any AlteredValue;
	};

	struct FEModuleScriptInstance
	{
		FEScene* Scene;
		FEEntity* Entity;
		std::string ScriptName;
		std::vector<FEAlteredScriptVariable> AlteredVariables;
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
		void AddFailedToLoadData(FEEntity* Entity, std::string ModuleID, Json::Value RawData);

		bool InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, std::string ActiveModuleID, FEScriptData& ScriptData);
		FENativeScriptModule* GetActiveModule(std::string ModuleID);

		void CopyVariableValuesInternal(FENativeScriptComponent* SourceComponent, FENativeScriptComponent* TargetComponent);

		// Returns array of information about components associated with module.
		std::vector<FEModuleScriptInstance> GetModuleScriptInstances(FENativeScriptModule* Module);
		void GetModuleScriptInstancesFromScene(std::vector<FEModuleScriptInstance>& Result, FEScene* Scene, std::string ModuleID);

		// Function will check component for user altered variables, if any found, it will update list of components accordingly.
		void CheckForAlteredVariables(std::vector<FEModuleScriptInstance>& ModuleScriptInstancesToUpdate);
		std::any CreateEngineLocalScriptVariableCopy(FEScriptVariableInfo& Info, std::any Value);
		template<typename T>
		std::any CreateEngineLocalScriptVariableCopyTemplated(std::any Value);

		// We should delete all script components associated with module.
		void ComponentsClearOnModuleDeactivate(FENativeScriptModule* Module);

		void RemoveComponentsFromScene(FEScene* Scene, std::string ModuleID);

		template<typename T>
		T* CastScript(FENativeScriptCore* Core);

		template<typename T>
		std::any LoadVariableTTypeToJSON(const Json::Value& Root);
		template<typename T>
		std::any LoadArrayVariableTTypeToJSON(const Json::Value& Root);
		void LoadVariableFromJSON(Json::Value& Root, FEScriptVariableInfo& VariableInfo, FENativeScriptCore* Core);

		template<typename T>
		void SaveVariableTTypeToJSON(Json::Value& Root, std::any AnyValue);
		template<typename T>
		void SaveArrayVariableTTypeToJSON(Json::Value& Root, std::any AnyValue);
		void SaveVariableToJSON(Json::Value& Root, FEScriptVariableInfo& VariableInfo, FENativeScriptCore* Core);

		template<typename T>
		bool IsEqualTType(std::any FirstScriptVariable, std::any SecondScriptVariable);

		template<typename T>
		bool IsEqualArrayTType(std::any FirstScriptVariable, std::any SecondScriptVariable);

		bool IsEqualScriptVariable(FEScriptVariableInfo& VariableInfo, std::any FirstScriptVariable,  std::any SecondScriptVariable);
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

		bool ReloadDLL(FENativeScriptModule* ModuleToUpdate);

		std::vector<std::string> GetActiveModuleIDList();
		std::vector<std::string> GetActiveModuleScriptNameList(std::string ModuleID);

		bool InitializeScriptComponent(FEEntity* Entity, std::string ActiveModuleID, std::string ScriptName);

		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(FEEntity* Entity);
		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(std::string ModuleID, std::string ScriptName);
		
		template<typename T>
		T* CastToScriptClass(FENativeScriptComponent& Component);

		template<typename T>
		std::vector<FEEntity*> GetEntityListWithScript(FEScene* Scene);

		template<typename T>
		std::vector<T*> GetScriptList(FEScene* Scene);
	};

#include "FENativeScriptSystem.inl"

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetNativeScriptSystem();
	#define NATIVE_SCRIPT_SYSTEM (*static_cast<FENativeScriptSystem*>(GetNativeScriptSystem()))
#else
	#define NATIVE_SCRIPT_SYSTEM FENativeScriptSystem::GetInstance()
#endif
}