// DO NOT CHANGE THIS FILE.
#include "../../../FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
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
#else
	class FEStaticCoreScriptManager
	{
	public:
		SINGLETON_PUBLIC_PART(FEStaticCoreScriptManager)

		using ScriptCreator = std::function<FENativeScriptCore* ()>;

		void RegisterScript(const std::string& Name, ScriptCreator ConstructorFunction)
		{
			auto& CurrentRegistry = GetRegistryForCurrentModule();
			CurrentRegistry[Name].ConstructorFunction = ConstructorFunction;
			CurrentRegistry[Name].Name = Name;
		}

		static std::unordered_map<std::string, FEScriptData>& GetRegistry()
		{
			static std::unordered_map<std::string, FEScriptData> Registry;
			return Registry;
		}

		std::unordered_map<std::string, FEScriptData>& GetRegistryForCurrentModule()
		{
			if (CurrentModuleID.empty())
				throw std::runtime_error("FEStaticCoreScriptManager::GetRegistryForCurrentModule Module ID is not set. Use SET_MODULE_ID macro before registering scripts.");
			
			return Registry[CurrentModuleID];
		}

		bool HaveModuleWithID(std::string& ModuleID)
		{
			return Registry.find(ModuleID) != Registry.end();
		}

		std::unordered_map<std::string, FEScriptData>& GetRegistryForModuleWithID(std::string& ModuleID)
		{
			if (!HaveModuleWithID(ModuleID))
			{
				LOG.Add("FEStaticCoreScriptManager::GetRegistryForModuleWithID: Module with ID " + ModuleID + " does not exist.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
				return std::unordered_map<std::string, FEScriptData>();
			}

			return Registry[ModuleID];
		}

		void SetCurrentModuleID(const std::string& ModuleID)
		{
			CurrentModuleID = ModuleID;
		}

	private:
		SINGLETON_PRIVATE_PART(FEStaticCoreScriptManager)

		std::string CurrentModuleID = "";
		std::unordered_map<std::string, std::unordered_map<std::string, FEScriptData>> Registry;
	};

	#define STATIC_CORE_SCRIPT_MANAGER FEStaticCoreScriptManager::GetInstance()


	#define REGISTER_SCRIPT(ScriptClass)													    \
    FENativeScriptCore* Create##ScriptClass() {		                                            \
        return new ScriptClass();														        \
    }																					        \
    namespace {																			        \
        struct Register##ScriptClass {													        \
            Register##ScriptClass() {													        \
				STATIC_CORE_SCRIPT_MANAGER.RegisterScript(#ScriptClass, Create##ScriptClass);	\
            }																			        \
        } register##ScriptClass;														        \
    }

	#define REGISTER_SCRIPT_FIELD(ScriptClass, FieldType, FieldName)																		\
    namespace {																																\
        struct Register##ScriptClass##FieldName {																							\
            Register##ScriptClass##FieldName() {																							\
                auto& ScriptData = STATIC_CORE_SCRIPT_MANAGER.GetRegistryForCurrentModule()[#ScriptClass].VariablesRegistry[#FieldName];	\
				ScriptData.Name = #FieldName;																								\
				ScriptData.Type = #FieldType;																								\
                    																														\
                ScriptData.Getter = [](FENativeScriptCore* Base) -> std::any {																\
                    auto* Script = static_cast<ScriptClass*>(Base);																			\
                    return Script->FieldName;																								\
                };																															\
																																			\
				ScriptData.Setter = [](FENativeScriptCore* Base, const std::any& Value) {													\
                    auto* Script = static_cast<ScriptClass*>(Base);																			\
                    Script->FieldName = std::any_cast<FieldType>(Value);																	\
                };																															\
            }																																\
        } register##ScriptClass##FieldName;																									\
    }

	#define RUN_IN_EDITOR_MODE(ScriptClass)															\
    namespace {																						\
        struct RegisterRunInEditor##ScriptClass {													\
            RegisterRunInEditor##ScriptClass() {													\
				auto& CurrentRegistry = STATIC_CORE_SCRIPT_MANAGER.GetRegistryForCurrentModule();	\
				if (CurrentRegistry.find(#ScriptClass) != CurrentRegistry.end())					\
					CurrentRegistry[#ScriptClass].bRunInEditor = true;								\
            }																						\
        } registerRunInEditor##ScriptClass;															\
    }

	#define SET_MODULE_ID(ModuleID)                                         \
    namespace {														        \
        struct FocalEngineRegisterModuleID {						        \
            FocalEngineRegisterModuleID() {							        \
				STATIC_CORE_SCRIPT_MANAGER.SetCurrentModuleID(ModuleID);	\
            }														        \
        } focalEngineRegisterModuleID;								        \
    }
#endif