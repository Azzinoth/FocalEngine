// DO NOT CHANGE THIS FILE.
#define	FEBASICAPPLICATION_SHARED
#define FOCAL_ENGINE_SHARED
#include "FEngine.h"
using namespace FocalEngine;

// Expose function to query script information
extern "C" __declspec(dllexport) const char* GetModuleID();
extern "C" __declspec(dllexport) void* GetScriptRegistry();
extern "C" __declspec(dllexport) unsigned long long GetEngineHeadersBuildVersion();

extern "C" __declspec(dllexport) bool IsCompiledInDebugMode();

class FECoreScriptManager
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

	std::string CurrentModuleID = "";

private:
	SINGLETON_PRIVATE_PART(FECoreScriptManager)
};

#define CORE_SCRIPT_MANAGER FECoreScriptManager::GetInstance()


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
                CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Getter = [](FENativeScriptCore* Base) -> std::any {			\
                    auto* Script = static_cast<ScriptClass*>(Base);																							\
                    return Script->FieldName;																												\
                };																																			\
				CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].VariablesRegistry[#FieldName].Setter = [](FENativeScriptCore* Base, const std::any& Value) {\
                    auto* Script = static_cast<ScriptClass*>(Base);																							\
                    Script->FieldName = std::any_cast<FieldType>(Value);																					\
                };																																			\
            }																																				\
        } register##ScriptClass##FieldName;																													\
    }

#define RUN_IN_EDITOR_MODE(ScriptClass)																					\
    namespace {																											\
        struct RegisterRunInEditor##ScriptClass {																		\
            RegisterRunInEditor##ScriptClass() {																		\
				if (CORE_SCRIPT_MANAGER.GetRegistry().find(#ScriptClass) != CORE_SCRIPT_MANAGER.GetRegistry().end())	\
					CORE_SCRIPT_MANAGER.GetRegistry()[#ScriptClass].bRunInEditor = true;								\
            }																											\
        } registerRunInEditor##ScriptClass;																				\
    }

#define SET_MODULE_ID(ModuleID)                                     \
    namespace {														\
        struct FocalEngineRegisterModuleID {						\
            FocalEngineRegisterModuleID() {							\
					CORE_SCRIPT_MANAGER.CurrentModuleID = ModuleID;	\
            }														\
        } focalEngineRegisterModuleID;								\
    }
	
// Project specific information
SET_MODULE_ID("PLACE_HOLDER");