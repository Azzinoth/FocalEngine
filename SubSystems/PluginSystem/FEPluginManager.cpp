#include "FEPluginManager.h"
#include "../../ResourceManager/FEResourceManager.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPluginManager()
{
	return FEPluginManager::GetInstancePointer();
}
#endif

FEPluginManager::FEPluginManager()
{
}

FEPluginManager::~FEPluginManager()
{
}

void FEPluginManager::Initialize()
{
	const std::string PluginDirectoryPath = RESOURCE_MANAGER.GetEngineFolder() + PluginDirectory;
	if (!FILE_SYSTEM.DoesDirectoryExist(PluginDirectoryPath))
	{
		LOG.Add("FEPluginManager::Initialize: Plugin directory \"" + PluginDirectoryPath + "\" does not exist", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
		return;
	}

	std::vector<std::string> PluginFolders = FILE_SYSTEM.GetDirectoryList(PluginDirectoryPath);
	std::vector<PluginManifest> Manifests;

	for (int i = 0; i < PluginFolders.size(); i++)
	{
		const std::string PluginFolderPath = PluginDirectoryPath + "/" + PluginFolders[i];

		std::vector<std::string> ManifestPaths;
		for (const std::string& FileName : FILE_SYSTEM.GetFileList(PluginFolderPath))
		{
			const std::string FilePath = PluginFolderPath + "/" + FileName;
			if (FILE_SYSTEM.GetFileExtension(FileName) == ".json" && IsValidManifest(FilePath))
				ManifestPaths.push_back(FilePath);
		}

		if (ManifestPaths.empty())
		{
			LOG.Add("FEPluginManager::Initialize: No valid manifest found in \"" + PluginFolderPath + "\"", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
			continue;
		}

		if (ManifestPaths.size() > 1)
		{
			LOG.Add("FEPluginManager::Initialize: More than one valid manifest found in \"" + PluginFolderPath + "\"", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
			continue;
		}

		PluginManifest Manifest;
		if (!ParseManifest(ManifestPaths[0], Manifest))
		{
			LOG.Add("FEPluginManager::Initialize: Failed to parse manifest \"" + ManifestPaths[0] + "\"", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
			continue;
		}

		Manifests.push_back(Manifest);
	}
}

static bool ReadJsonFile(const std::string& FilePath, Json::Value& OutRoot)
{
	std::ifstream File(FilePath);
	if (!File.is_open())
		return false;

	const std::string FileData((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
	File.close();

	JSONCPP_STRING Error;
	Json::CharReaderBuilder Builder;
	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	return Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &OutRoot, &Error);
}

bool FEPluginManager::IsValidManifest(const std::string& ManifestPath)
{
	Json::Value Root;
	if (!ReadJsonFile(ManifestPath, Root))
		return false;

	auto IsNonEmptyString = [](const Json::Value& Value)
	{
		return Value.isString() && !Value.asString().empty();
	};

	auto IsValidID = [](const Json::Value& Value)
	{
		return Value.isString() && !UNIQUE_ID.IsNull(UNIQUE_ID.FromString(Value.asString()));
	};

	auto IsValidIDList = [&](const Json::Value& Value)
	{
		if (!Value.isArray())
			return false;

		for (const Json::Value& Element : Value)
		{
			if (!IsValidID(Element))
				return false;
		}

		return true;
	};

	auto IsValidOSList = [](const Json::Value& Value)
	{
		if (!Value.isArray() || Value.empty())
			return false;

		for (const Json::Value& Element : Value)
		{
			if (!Element.isString() || OSFromString(Element.asString()) == OS::Unknown)
				return false;
		}

		return true;
	};

	if (!Root.isObject())
		return false;

	if (!IsValidID(Root["ID"]) || !IsNonEmptyString(Root["Name"]) || !IsNonEmptyString(Root["Version"]) ||
		!IsValidOSList(Root["SupportedOS"]) || !IsValidIDList(Root["Dependencies"]))
		return false;

	const Json::Value& Modules = Root["Modules"];
	if (!Modules.isArray() || Modules.empty())
		return false;

	for (const Json::Value& Module : Modules)
	{
		if (!Module.isObject())
			return false;

		const Json::Value& Type = Module["Type"];
		if (!Type.isString() || (Type.asString() != "Engine" && Type.asString() != "Editor"))
			return false;

		if (!IsValidID(Module["ID"]) || !IsNonEmptyString(Module["Name"]) || !IsNonEmptyString(Module["EntryPointFileName"]) ||
			!IsValidOSList(Module["SupportedOS"]) || !IsValidIDList(Module["Dependencies"]))
			return false;
	}

	return true;
}

bool FEPluginManager::ParseManifest(const std::string& ManifestPath, PluginManifest& OutManifest)
{
	if (!IsValidManifest(ManifestPath))
		return false;

	Json::Value Root;
	if (!ReadJsonFile(ManifestPath, Root))
		return false;

	auto ParseIDList = [](const Json::Value& Value)
	{
		std::vector<FEUUID> Result;
		for (const Json::Value& Element : Value)
			Result.push_back(UNIQUE_ID.FromString(Element.asString()));

		return Result;
	};

	auto ParseOSList = [](const Json::Value& Value)
	{
		std::vector<OS> Result;
		for (const Json::Value& Element : Value)
			Result.push_back(OSFromString(Element.asString()));

		return Result;
	};

	PluginManifest Manifest;
	Manifest.ID = UNIQUE_ID.FromString(Root["ID"].asString());
	Manifest.Dependencies = ParseIDList(Root["Dependencies"]);
	Manifest.SupportedOS = ParseOSList(Root["SupportedOS"]);
	Manifest.Name = Root["Name"].asString();
	Manifest.Version = Root["Version"].asString();

	for (const Json::Value& ModuleData : Root["Modules"])
	{
		PluginModule Module;
		Module.ID = UNIQUE_ID.FromString(ModuleData["ID"].asString());
		Module.PluginID = Manifest.ID;
		Module.SupportedOS = ParseOSList(ModuleData["SupportedOS"]);
		Module.Dependencies = ParseIDList(ModuleData["Dependencies"]);
		Module.Type = ModuleData["Type"].asString();
		Module.Name = ModuleData["Name"].asString();
		Module.EntryPointFileName = ModuleData["EntryPointFileName"].asString();

		Manifest.Modules.push_back(Module);
	}

	OutManifest = Manifest;
	return true;
}