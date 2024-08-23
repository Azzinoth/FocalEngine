#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
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

		void InitializeComponentInternal(FEEntity* Entity, FENativeScriptComponent& NativeScriptComponent, std::string ModuleID, FEScriptData& ScriptData);
		FENativeScriptModuleData* GetModuleData(std::string ModuleID);

		void UpdateScriptComponentsDueToModuleUpdate(std::string ModuleID);
	public:
		SINGLETON_PUBLIC_PART(FENativeScriptSystem)

		std::unordered_map<std::string, FENativeScriptModuleData*> Modules;

		bool LoadCompiledNativeScriptModule(std::string DLLPath);

		std::vector<std::string> GetModuleIDList();
		std::vector<std::string> GetModuleScriptNameList(std::string ModuleID);

		void InitializeScriptComponent(FEEntity* Entity, std::string ModuleID, std::string ScriptName);

		FEScriptData& GetScriptData(FENativeScriptComponent& NativeScriptComponent);

		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(FEEntity* Entity);
		std::unordered_map<std::string, FEScriptVariableInfo> GetVariablesRegistry(std::string ModuleID, std::string ScriptName);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetNativeScriptSystem();
	#define NATIVE_SCRIPT_SYSTEM (*static_cast<FENativeScriptSystem*>(GetNativeScriptSystem()))
#else
	#define NATIVE_SCRIPT_SYSTEM FENativeScriptSystem::GetInstance()
#endif
}