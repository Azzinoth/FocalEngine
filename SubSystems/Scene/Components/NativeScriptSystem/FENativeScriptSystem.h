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
		void AddFailedToLoadData(FEEntity* Entity, const FEUUID& ModuleID, Json::Value RawData);

		bool InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, const FEUUID& ActiveModuleID, FEScriptData& ScriptData);
		FENativeScriptModule* GetActiveModule(const FEUUID& ModuleID);

		void CopyVariableValuesInternal(FENativeScriptComponent* SourceComponent, FENativeScriptComponent* TargetComponent);

		// Returns array of information about components associated with module.
		std::vector<FEModuleScriptInstance> GetModuleScriptInstances(FENativeScriptModule* Module);
		void GetModuleScriptInstancesFromScene(std::vector<FEModuleScriptInstance>& Result, FEScene* Scene, const FEUUID& ModuleID);

		// Returns array of information about components associated with module that was not loaded properly.
		std::vector<FEModuleScriptInstance> GetFailedToLoadModuleScriptInstances(FENativeScriptModule* Module);
		void GetFailedToLoadModuleScriptInstancesFromScene(std::vector<FEModuleScriptInstance>& Result, FEScene* Scene, const FEUUID& ModuleID);

		// Function will check component for user altered variables, if any found, it will update list of components accordingly.
		void CheckForAlteredVariables(std::vector<FEModuleScriptInstance>& ModuleScriptInstancesToUpdate);
		std::any CreateEngineLocalScriptVariableCopy(FEScriptVariableInfo& Info, std::any Value);
		template<typename T>
		std::any CreateEngineLocalScriptVariableCopyTemplated(std::any Value);

		// We should delete all script components associated with module.
		void ComponentsClearOnModuleDeactivate(FENativeScriptModule* Module);

		void RemoveComponentsFromScene(FEScene* Scene, const FEUUID& ModuleID);

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

		std::unordered_map<FEUUID, FENativeScriptModule*> ActiveModules;

		bool ActivateNativeScriptModule(const FEUUID& ModuleID);
		bool ActivateNativeScriptModule(FENativeScriptModule* Module);

		bool DeactivateNativeScriptModule(const FEUUID& ModuleID);
		bool DeactivateNativeScriptModule(FENativeScriptModule* Module);

		void DeleteNativeScriptModule(const FEUUID& ModuleID);
		void DeleteNativeScriptModule(FENativeScriptModule* Module);

		bool ReloadDLL(FENativeScriptModule* ModuleToUpdate);

		std::vector<FEUUID> GetActiveModuleIDList();
		std::vector<std::string> GetActiveModuleScriptNameList(const FEUUID& ModuleID);

		bool InitializeScriptComponent(FEEntity* Entity, const FEUUID& ActiveModuleID, std::string ScriptName);

		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(FEEntity* Entity);
		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(const FEUUID& ModuleID, std::string ScriptName);
		
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