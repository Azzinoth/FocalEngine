#pragma once

#include "../Core/FEObject.h"
#include "../Core/FEGeometricTools.h"
#include <any>

namespace FocalEngine
{
	class FENativeScriptCore
	{
		friend class FENativeScriptComponent;
		friend class FECoreScriptManager;
		friend class FENativeScriptSystem;
	public:
		virtual ~FENativeScriptCore() = default;
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(double DeltaTime) {}

		class FEEntity* ParentEntity;
	};

	struct FEScriptVariableInfo
	{
		std::string Name;
		std::string Type;
		std::function<std::any(FENativeScriptCore*)> Getter;
		std::function<void(FENativeScriptCore*, const std::any&)> Setter;
	};

	struct FEScriptData
	{
	private:
		friend class FENativeScriptComponent;
		friend class FECoreScriptManager;
		friend class FENativeScriptSystem;

		std::function<FENativeScriptCore* ()> ConstructorFunction;
	public:
		std::string Name;
		std::unordered_map<std::string, FEScriptVariableInfo> VariablesRegistry;
	};

	struct FENativeScriptModuleData
	{
		std::string Name;
		std::string DLLPath;
		std::string PDBPath;
		std::string ID;

		HMODULE DLLHandle;
		std::unordered_map<std::string, FEScriptData> Registry;
	};

	class FOCAL_ENGINE_API FECoreScriptManager
	{
	public:
		SINGLETON_PUBLIC_PART(FECoreScriptManager)

		using ScriptCreator = std::function<FENativeScriptCore* ()>;

		static void RegisterScript(const std::string& Name, ScriptCreator ConstructorFunction)
		{
			GetRegistry()[Name].ConstructorFunction = ConstructorFunction;
			GetRegistry()[Name].Name = Name;
		}

		static std::unordered_map<std::string, FEScriptData>& GetRegistry()
		{
			static std::unordered_map<std::string, FEScriptData> Registry;
			return Registry;
		}

	private:
		SINGLETON_PRIVATE_PART(FECoreScriptManager)
	};

// TO DO: Decide if it would be available in the DLL, or leave it as it is.
#define CORE_SCRIPT_MANAGER FECoreScriptManager::GetInstance()

//#ifdef FOCAL_ENGINE_SHARED
//	extern "C" __declspec(dllexport) void* GetCoreScriptManager();
//	#define CORE_SCRIPT_MANAGER (*static_cast<FECoreScriptManager*>(GetCoreScriptManager()))
//#else
//	#define CORE_SCRIPT_MANAGER FECoreScriptManager::GetInstance()
//#endif

#define REGISTER_SCRIPT(ScriptClass)													\
    extern "C" __declspec(dllexport) FENativeScriptCore* Create##ScriptClass() {		\
        return new ScriptClass();														\
    }																					\
    namespace {																			\
        struct Register##ScriptClass {													\
            Register##ScriptClass() {													\
				CORE_SCRIPT_MANAGER.RegisterScript(#ScriptClass, Create##ScriptClass);	\
            }																			\
        } register##ScriptClass;														\
    }

#define REGISTER_SCRIPT_FIELD(ScriptClass, FieldType, FieldName)																							\
    namespace {																																				\
        struct Register##ScriptClass##FieldName {																											\
            Register##ScriptClass##FieldName() {																											\
                CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Name = #FieldName;											\
				CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Type = #FieldType;											\
                    																																		\
                CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Getter = [](FENativeScriptCore* base) -> std::any {			\
                    auto* script = static_cast<ScriptClass*>(base);																							\
                    return script->FieldName;																												\
                };																																			\
				CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Setter = [](FENativeScriptCore* base, const std::any& value) {\
                    auto* script = static_cast<ScriptClass*>(base);																							\
                    script->FieldName = std::any_cast<FieldType>(value);																					\
                };																																			\
            }																																				\
        } register##ScriptClass##FieldName;																													\
    }
}