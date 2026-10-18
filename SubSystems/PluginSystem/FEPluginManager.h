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

		void Update(double DeltaTime);
		void Shutdown();

		bool GeneratePluginSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory);
		bool GenerateManifestFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory);

		bool InitializeProject(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool InitializeCMakeFile(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool ConfigureAndBuildCMake(const std::string& VSProjectDirectory);
		bool RunVSProject(const std::string& PluginName, const std::string& VSProjectDirectory);
	public:
		SINGLETON_PUBLIC_PART(FEPluginManager)

		void Initialize();
		bool IsValidManifest(const std::string& ManifestPath);
		bool ParseManifest(const std::string& ManifestPath, PluginManifest& OutManifest);

		bool CreateNewVSProject(std::string& PluginName, const std::string& VSProjectDirectory);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPluginManager();
	#define PLUGIN_MANAGER (*static_cast<FEPluginManager*>(GetPluginManager()))
#else
	#define PLUGIN_MANAGER FEPluginManager::GetInstance()
#endif
}