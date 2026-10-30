#pragma once
#include "FEPluginCore.h"

namespace FocalEngine
{

	struct PluginModule
	{
		FEUUID ID;
		FEUUID PluginID;
		std::vector<OS> SupportedOS;
		std::vector<FEGraphicsMode> SupportedGraphicsModes;
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
		std::string Description;
	};

	// Plugin folder with valid manifest, found by plugin manager during initialization.
	struct DiscoveredPlugin
	{
		PluginManifest Manifest;
		std::string FolderPath;
	};

	struct FEPluginProjectSettings
	{
		// Main module is engine module named after plugin. Without it manifest has no modules,
		// application that creates project should add its own modules through AdditionalModules.cmake and manifest.
		bool bAddMainModule = true;
		// Plugin gets Public/<PluginName>Interface.h that applications can include to call API of main module.
		bool bAddPublicInterface = false;
		// If false, project is only generated, so application could add its own modules and then call ConfigureAndBuildCMake and RunVSProject.
		bool bConfigureAndBuild = true;
		std::string Description;
		// Graphics modes of main module.
		std::vector<FEGraphicsMode> SupportedGraphicsModes = { FEGraphicsMode::OnScreen };
	};

	class FEPlugin
	{
		friend class FEPluginManager;

		PluginManifest* Manifest = nullptr;
		PluginModule* Module = nullptr;
		std::string DLLPath;

		HMODULE DLLHandle = nullptr;
		FEPluginCore* Instance = nullptr;
		void (*DestroyFunction)(FEPluginCore*) = nullptr;
	public:
		FEPluginCore* GetInstance() const { return Instance; }
		const PluginManifest* GetManifest() const { return Manifest; }
		const PluginModule* GetModule() const { return Module; }
		const std::string& GetDLLPath() const { return DLLPath; }
	};

	class FOCAL_ENGINE_API FEPluginManager
	{
		friend class FEngine;
		friend class FESceneManager;
		friend class FERenderer;

		SINGLETON_PRIVATE_PART(FEPluginManager)

		std::string PluginDirectory;
		std::vector<DiscoveredPlugin> DiscoveredPlugins;
		std::vector<FEPlugin> ActivePlugins;
		std::function<void()> BeforeShutdownCallback;

		// Flags of scenes that plugins were notified about with OnSceneAdded, used to detect changes of flags.
		std::unordered_map<FEUUID, FESceneFlag> ReportedScenes;
		void CheckScenesForModification();
		void SceneDeleting(FEScene* Scene);

		// Plugin projects include engine as git submodule.
		std::string EngineRepositoryURL = "https://github.com/Azzinoth/FocalEngine";
		std::string EngineRepositoryBranch = "master";

		void Update(double DeltaTime);
		void BeforeRender();
		void AfterRender();

		// Set by FEngine while it renders internal frame of RenderTo, plugins are not called for its cameras.
		bool bInternalFrame = false;
		void CameraRenderStage(FECameraRenderStage Stage, FEEntity* CameraEntity);

		bool LoadEngineModule(const DiscoveredPlugin& Plugin, size_t ModuleIndex);

		bool GeneratePluginSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory, bool bAddPublicInterface);
		bool GenerateManifestFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory, const FEPluginProjectSettings& Settings);
		bool GeneratePublicInterfaceFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory);

		bool InitializeProject(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool InitializeCMakeFile(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList);
		bool AddEngineSubmodule(const std::string& VSProjectDirectory);

		std::vector<std::string> GetValidManifestPaths(const std::string& FolderPath);
	public:
		SINGLETON_PUBLIC_PART(FEPluginManager)

		void Initialize();

		// Default is Plugins folder of the engine, should be set before engine is initialized.
		void SetPluginDirectory(const std::string& NewValue);
		std::string GetPluginDirectory() const;

		using CreatePluginFunction = FEPluginCore* (*)();
		using DestroyPluginFunction = void (*)(FEPluginCore*);
		// Called by REGISTER_PLUGIN when plugin is linked into application statically, LoadPluginModule then uses it instead of DLL.
		static void RegisterStaticPluginModule(const std::string& ModuleName, int PluginInterfaceVersion, CreatePluginFunction CreateFunction, DestroyPluginFunction DestroyFunction);

		bool IsValidManifest(const std::string& ManifestPath);
		bool ParseManifest(const std::string& ManifestPath, PluginManifest& OutManifest);

		const std::vector<DiscoveredPlugin>& GetDiscoveredPlugins() const;

		// Loads module DLL and creates plugin instance, OnLoad is not called, that is responsibility of the caller.
		// Used by plugin manager for engine modules and by applications for other module types (for example editor modules).
		bool LoadPluginModule(const DiscoveredPlugin& Plugin, size_t ModuleIndex, FEPlugin& OutPlugin);
		// Destroys plugin instance and unloads its DLL, OnUnload should be called by the caller before that.
		void UnloadPluginModule(FEPlugin& Plugin);

		// Checks only engine modules, because only they are loaded by plugin manager.
		bool IsModuleLoaded(const FEUUID& ModuleID) const;

		// Called at the beginning of plugin manager shutdown, before engine modules are unloaded.
		// Applications use it to unload modules that depend on engine modules, it could be called more than once.
		void SetBeforeShutdownCallback(std::function<void()> Callback);
		// Unloads engine modules in reverse order of loading, safe to call more than once.
		void Shutdown();

		bool CreatePluginProject(std::string& PluginName, const std::string& VSProjectDirectory, const FEPluginProjectSettings& Settings = FEPluginProjectSettings());
		bool ConfigureAndBuildCMake(const std::string& VSProjectDirectory);
		bool RunVSProject(const std::string& PluginName, const std::string& VSProjectDirectory);

		bool CreatePluginPackage(const std::string& PluginFolderPath, const std::string& PackageFilePath, std::string& OutErrorMessage);
		bool ReadPluginPackageManifest(const std::string& PackageFilePath, PluginManifest& OutManifest, std::string& OutErrorMessage);
		// Installed plugin is loaded on next start.
		bool InstallPluginPackage(const std::string& PackageFilePath, std::string& OutErrorMessage);

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
	private:
		struct StaticPluginModule
		{
			int PluginInterfaceVersion = 0;
			CreatePluginFunction CreateFunction = nullptr;
			DestroyPluginFunction DestroyFunction = nullptr;
		};

		// Modules register during static initialization, so registry is a function local static.
		static std::unordered_map<std::string, StaticPluginModule>& GetStaticPluginModules();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPluginManager();
	#define PLUGIN_MANAGER (*static_cast<FEPluginManager*>(GetPluginManager()))
#else
	#define PLUGIN_MANAGER FEPluginManager::GetInstance()
#endif
}