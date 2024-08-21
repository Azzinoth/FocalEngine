#pragma once

#include "../Core/FEObject.h"
#include "../Core/FEGeometricTools.h"
#include <any>

namespace FocalEngine
{
	class FENativeScriptComponent
	{
	public:
		class FENativeScriptCore* CoreInstance = nullptr;
		void (*CreateScript)(FENativeScriptComponent&);
		void (*DestroyScript)(FENativeScriptComponent&);

		/*template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}*/
	};

	class FENativeScriptCore
	{
	public:
		virtual ~FENativeScriptCore() = default;
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(float DeltaTime) {}

		FEEntity* ParentEntity;

		

		//std::unordered_map<std::string, std::vector<FieldInfo>> MyRegistry;

		//virtual void RegisterFields() {}

		/*static std::unordered_map<std::string, std::vector<FieldInfo>>& GetFieldRegistry() {
			static std::unordered_map<std::string, std::vector<FieldInfo>> registry;
			return registry;
		}*/
	};

	//static std::string GetModuleID()
	//{
	//	static std::string ModuleID = "Engine";
	//	return ModuleID;
	//}
	//static std::string ModuleID = "Engine";

	struct ModuleInfo
	{
		std::string Name;
		std::string Path;
		std::string ID;
	};

	struct ScriptFieldInfo {
		std::string name;
		std::string type;
		std::function<std::any(FENativeScriptCore*)> getter;
		std::function<void(FENativeScriptCore*, const std::any&)> setter;
	};

	struct ScriptData
	{
		std::function<FENativeScriptCore*()> CreateScript;
		std::unordered_map<std::string, std::vector<ScriptFieldInfo>> FieldRegistry;
	};

	class FOCAL_ENGINE_API CoreScriptManager
	{
	public:
		SINGLETON_PUBLIC_PART(CoreScriptManager)

		bool bFirstRegistration = true;

		using ScriptCreator = std::function<FENativeScriptCore* ()>;

		static void RegisterScript(const std::string& name, ScriptCreator creator) {
			GetRegistry()[name] = creator;
			GetRegistryNew()[name].CreateScript = creator;
		}

		static void RegisterScriptNew(const std::string& ModuleID, const std::string& name, ScriptCreator creator) {
			
			GetRegistryNewNew()[ModuleID][name].CreateScript = creator;
		}


		static FENativeScriptCore* CreateScript(const std::string& name) {
			auto it = GetRegistry().find(name);
			if (it != GetRegistry().end()) {
				return it->second();
			}
			return nullptr;
		}

		std::unordered_map<std::string, ScriptCreator> GetAllScripts() const;

		static std::unordered_map<std::string, std::unordered_map<std::string, ScriptData>>& GetRegistryNewNew() {
			static std::unordered_map<std::string, std::unordered_map<std::string, ScriptData>> registry;
			return registry;
		}

		static std::unordered_map<std::string, ScriptData>& GetRegistryNew() {
			static std::unordered_map<std::string, ScriptData> registry;
			return registry;
		}

		static std::unordered_map<std::string, ScriptCreator>& GetRegistry() {
			static std::unordered_map<std::string, ScriptCreator> registry;
			return registry;
		}

	private:
		SINGLETON_PRIVATE_PART(CoreScriptManager)

		
	};

#define CORE_SCRIPT_MANAGER CoreScriptManager::GetInstance()


// Expose functions to query script information
extern "C" __declspec(dllexport) size_t GetScriptCount();
extern "C" __declspec(dllexport) char** GetScriptMap();

#define REGISTER_SCRIPT(ModuleID, ScriptClass) \
    extern "C" __declspec(dllexport) FENativeScriptCore* Create##ScriptClass() { \
        return new ScriptClass(); \
    } \
    namespace { \
        struct Register##ScriptClass { \
            Register##ScriptClass() { \
                CORE_SCRIPT_MANAGER.RegisterScript(#ScriptClass, Create##ScriptClass); \
				CORE_SCRIPT_MANAGER.RegisterScriptNew(#ModuleID, #ScriptClass, Create##ScriptClass);\
            } \
        } register##ScriptClass; \
    }

#define REGISTER_SCRIPT_FIELD(ModuleID, ScriptClass, FieldType, FieldName) \
    namespace { \
        struct Register##ScriptClass##FieldName { \
            Register##ScriptClass##FieldName() { \
                CORE_SCRIPT_MANAGER.GetRegistryNewNew()[#ModuleID][#ScriptClass].FieldRegistry[#FieldName].push_back({ \
                    #FieldName, \
                    #FieldType, \
                    [](FENativeScriptCore* base) -> std::any { \
                        auto* script = static_cast<ScriptClass*>(base); \
                        return script->FieldName; \
                    }, \
                    [](FENativeScriptCore* base, const std::any& value) { \
                        auto* script = static_cast<ScriptClass*>(base); \
                        script->FieldName = std::any_cast<FieldType>(value); \
                    } \
                }); \
            } \
        } register##ScriptClass##FieldName; \
    }


	class PlayerScript : public FENativeScriptCore
	{
		
	public:
		int test = 0;
		void OnCreate() override
		{
			test = -1000;
		}

		void OnUpdate(float DeltaTime) override
		{
			test = test + 1;
			int y = 0;
			y++;
		}
		void OnDestroy() override { /* ... */ }
	};

	REGISTER_SCRIPT_FIELD(Engine, PlayerScript, int, test)

	/*class FENativeScriptComponent
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FENativeScriptSystem;
	public:
		FENativeScriptComponent();
		~FENativeScriptComponent();

	protected:
		void OnCreate();
		void OnDestroy();
		void OnUpdate(float DeltaTime);

	private:
		FENativeScriptComponent(const FENativeScriptComponent& Other);
		FENativeScriptComponent& operator=(const FENativeScriptComponent& Other);

		void FENativeScriptComponent::CopyFrom(const FENativeScriptComponent& Other);

		FEEntity* ParentEntity = nullptr;
	};*/
}