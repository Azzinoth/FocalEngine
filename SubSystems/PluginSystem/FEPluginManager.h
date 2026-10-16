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

	class FOCAL_ENGINE_API FEPluginManager
	{
		friend class FEngine;

		SINGLETON_PRIVATE_PART(FEPluginManager)

		std::string PluginDirectory = "Plugins";
	public:
		SINGLETON_PUBLIC_PART(FEPluginManager)

		void Initialize();
		bool IsValidManifest(const std::string& ManifestPath);
		bool ParseManifest(const std::string& ManifestPath, PluginManifest& OutManifest);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPluginManager();
	#define PLUGIN_MANAGER (*static_cast<FEPluginManager*>(GetPluginManager()))
#else
	#define PLUGIN_MANAGER FEPluginManager::GetInstance()
#endif
}