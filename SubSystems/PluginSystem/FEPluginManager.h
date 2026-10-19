#pragma once
#include "FEPluginCore.h"

namespace FocalEngine
{

	struct PluginModule
	{
		FEUUID ID;
		FEUUID PluginID;
		std::vector<OS> SupportedOS;
		std::vector<FEUUID> Dependencies;

		std::string Type;
		std::string Name;
		std::string EntryPointFileName;
	};

	struct PluginManifest
	{
		FEUUID ID;
		std::vector<FEUUID> Dependencies;
		std::vector<OS> SupportedOS;
		std::vector<PluginModule> Modules;

		std::string Name;
		std::string Version;
	};

	class FEPlugin
	{
		friend class FEPluginManager;

		PluginManifest* Manifest = nullptr;
		PluginModule* Module = nullptr;

		HMODULE DLLHandle = nullptr;
		FEPluginCore* Instance = nullptr;
		void (*DestroyFunction)(FEPluginCore*) = nullptr;
	};

	class FOCAL_ENGINE_API FEPluginManager
	{
		friend class FEngine;

		SINGLETON_PRIVATE_PART(FEPluginManager)

		std::string PluginDirectory = "Plugins";
		std::vector<FEPlugin> ActivePlugins;

		// Plugin projects include engine as git submodule.
		// TO-DO: Plugins should target stable engine version, plugins branch is used while plugin system is in development.
		std::string EngineRepositoryURL = "https://github.com/Azzinoth/FocalEngine";
		std::string EngineRepositoryBranch = "plugins";

		void Update(double DeltaTime);
		void Shutdown();

		bool GeneratePluginSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory, bool bAddPublicInterface);
		bool GenerateManifestFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory);
		bool GeneratePublicInterfaceFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory);

		bool InitializeProject(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool InitializeCMakeFile(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool AddEngineSubmodule(const std::string& VSProjectDirectory);
		bool ConfigureAndBuildCMake(const std::string& VSProjectDirectory);
		bool RunVSProject(const std::string& PluginName, const std::string& VSProjectDirectory);
	public:
		SINGLETON_PUBLIC_PART(FEPluginManager)

		void Initialize();
		bool IsValidManifest(const std::string& ManifestPath);
		bool ParseManifest(const std::string& ManifestPath, PluginManifest& OutManifest);

		// If bAddPublicInterface is true, plugin gets Public/<PluginName>Interface.h that applications can include to call plugin API.
		bool CreatePluginProject(std::string& PluginName, const std::string& VSProjectDirectory, bool bAddPublicInterface = false);

		// Returns nullptr if plugin is not loaded, result should not be stored because plugin could be unloaded.
		FEPluginCore* GetPlugin(const FEUUID& PluginID);
		FEPluginCore* GetPluginByName(const std::string& PluginName);

		// T is usually a public interface that plugin ships in its own header.
		template<typename T>
		T* GetPlugin(const FEUUID& PluginID)
		{
			return dynamic_cast<T*>(GetPlugin(PluginID));
		}

		template<typename T>
		T* GetPluginByName(const std::string& PluginName)
		{
			return dynamic_cast<T*>(GetPluginByName(PluginName));
		}
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPluginManager();
	#define PLUGIN_MANAGER (*static_cast<FEPluginManager*>(GetPluginManager()))
#else
	#define PLUGIN_MANAGER FEPluginManager::GetInstance()
#endif
}