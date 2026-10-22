#include "FEPluginManager.h"
#include "../../ResourceManager/FEResourceManager.h"
#include "../Scene/FESceneManager.h"
#include <shellapi.h>
#include <cctype>
#include <cstring>
using namespace FocalEngine;

#include "../../Core/VersionInfo/FOCAL_ENGINE_Version.h"
#include "../../Core/VersionInfo/FEVersionInfo.h"
FE_DEFINE_VERSION_INFO(FOCAL_ENGINE_)

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPluginManager()
{
	return FEPluginManager::GetInstancePointer();
}
#endif

static bool IsDLLDependencyAvailable(const std::string& DependencyName, const std::vector<std::string>& SearchFolders)
{
	std::string LowerCaseName = DependencyName;
	std::transform(LowerCaseName.begin(), LowerCaseName.end(), LowerCaseName.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
	if (LowerCaseName.rfind("api-ms-", 0) == 0 || LowerCaseName.rfind("ext-ms-", 0) == 0)
		return true;

	// Already loaded DLL is used as it is.
	if (GetModuleHandleA(DependencyName.c_str()) != nullptr)
		return true;

	for (size_t i = 0; i < SearchFolders.size(); i++)
	{
		if (FILE_SYSTEM.DoesFileExist(SearchFolders[i] + "/" + DependencyName))
			return true;
	}

	return false;
}

static std::vector<std::string> FindMissingDLLDependencies(const std::string& DLLPath, const std::vector<std::string>& SearchFolders)
{
	std::vector<std::string> Result;

	std::ifstream File(DLLPath, std::ios::binary);
	if (!File.is_open())
		return Result;

	const std::vector<char> Data((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
	File.close();

	if (Data.size() < sizeof(IMAGE_DOS_HEADER))
		return Result;

	const IMAGE_DOS_HEADER* DOSHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(Data.data());
	if (DOSHeader->e_magic != IMAGE_DOS_SIGNATURE || DOSHeader->e_lfanew <= 0 || static_cast<size_t>(DOSHeader->e_lfanew) + sizeof(IMAGE_NT_HEADERS64) > Data.size())
		return Result;

	const IMAGE_NT_HEADERS64* NTHeaders = reinterpret_cast<const IMAGE_NT_HEADERS64*>(Data.data() + DOSHeader->e_lfanew);
	if (NTHeaders->Signature != IMAGE_NT_SIGNATURE || NTHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		return Result;

	const IMAGE_DATA_DIRECTORY& ImportDirectory = NTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (ImportDirectory.VirtualAddress == 0)
		return Result;

	const IMAGE_SECTION_HEADER* Sections = IMAGE_FIRST_SECTION(NTHeaders);
	const WORD SectionCount = NTHeaders->FileHeader.NumberOfSections;
	if (reinterpret_cast<const char*>(Sections + SectionCount) > Data.data() + Data.size())
		return Result;

	// Import table refers to data by relative virtual addresses, they are converted to offsets in the file through section table.
	// Returns 0 if address is not inside of any section.
	auto RelativeAddressToFileOffset = [&](const DWORD RelativeVirtualAddress) -> size_t
	{
		for (WORD i = 0; i < SectionCount; i++)
		{
			const DWORD SectionStart = Sections[i].VirtualAddress;
			const DWORD SectionSize = Sections[i].Misc.VirtualSize > Sections[i].SizeOfRawData ? Sections[i].Misc.VirtualSize : Sections[i].SizeOfRawData;
			if (RelativeVirtualAddress >= SectionStart && RelativeVirtualAddress < SectionStart + SectionSize)
				return static_cast<size_t>(RelativeVirtualAddress - SectionStart + Sections[i].PointerToRawData);
		}

		return 0;
	};

	// Import table is an array of descriptors, one for each DLL, it ends with descriptor that has no name.
	size_t DescriptorOffset = RelativeAddressToFileOffset(ImportDirectory.VirtualAddress);
	while (DescriptorOffset != 0 && DescriptorOffset + sizeof(IMAGE_IMPORT_DESCRIPTOR) <= Data.size())
	{
		const IMAGE_IMPORT_DESCRIPTOR* Descriptor = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(Data.data() + DescriptorOffset);
		if (Descriptor->Name == 0)
			break;

		const size_t NameOffset = RelativeAddressToFileOffset(Descriptor->Name);
		if (NameOffset != 0 && NameOffset < Data.size())
		{
			const std::string DependencyName(Data.data() + NameOffset, strnlen(Data.data() + NameOffset, Data.size() - NameOffset));
			if (!IsDLLDependencyAvailable(DependencyName, SearchFolders))
				Result.push_back(DependencyName);
		}

		DescriptorOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	}

	return Result;
}

FEPluginManager::FEPluginManager()
{
}

FEPluginManager::~FEPluginManager()
{
}

void FEPluginManager::Initialize()
{
	const std::string PluginDirectoryPath = GetPluginDirectory();
	if (!FILE_SYSTEM.DoesDirectoryExist(PluginDirectoryPath))
	{
		LOG.Add("FEPluginManager::Initialize: Plugin directory \"" + PluginDirectoryPath + "\" does not exist", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
		return;
	}

	std::vector<std::string> PluginFolders = FILE_SYSTEM.GetDirectoryList(PluginDirectoryPath);

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

		DiscoveredPlugin NewPlugin;
		NewPlugin.Manifest = Manifest;
		NewPlugin.FolderPath = PluginFolderPath;
		DiscoveredPlugins.push_back(NewPlugin);
	}

	for (size_t i = 0; i < DiscoveredPlugins.size(); i++)
	{
		const PluginManifest& Manifest = DiscoveredPlugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			if (Manifest.Modules[j].Type != "Engine")
				continue;

			FEPlugin Plugin;
			if (!LoadPluginModule(DiscoveredPlugins[i], j, Plugin))
				continue;

			ActivePlugins.push_back(Plugin);
			Plugin.Instance->OnLoad();
			const std::string Source = Plugin.DLLPath.empty() ? std::string("is statically linked") : "loaded from \"" + Plugin.DLLPath + "\"";
			LOG.Add("FEPluginManager::Initialize: Plugin \"" + Manifest.Name + "\" version " + Manifest.Version + ", module \"" + Manifest.Modules[j].Name + "\" " + Source, "FE_PLUGIN_MANAGER", FE_LOG_INFO);
		}
	}
}

const std::vector<DiscoveredPlugin>& FEPluginManager::GetDiscoveredPlugins() const
{
	return DiscoveredPlugins;
}

void FEPluginManager::SetPluginDirectory(const std::string& NewValue)
{
	PluginDirectory = NewValue;
}

std::string FEPluginManager::GetPluginDirectory() const
{
	if (PluginDirectory.empty())
		return RESOURCE_MANAGER.GetEngineFolder() + "Plugins";

	return PluginDirectory;
}

void FEPluginManager::RegisterStaticPluginModule(const std::string& ModuleName, CreatePluginFunction CreateFunction, DestroyPluginFunction DestroyFunction)
{
	StaticPluginModule& Module = GetStaticPluginModules()[ModuleName];
	Module.CreateFunction = CreateFunction;
	Module.DestroyFunction = DestroyFunction;
}

std::unordered_map<std::string, FEPluginManager::StaticPluginModule>& FEPluginManager::GetStaticPluginModules()
{
	static std::unordered_map<std::string, StaticPluginModule> StaticPluginModules;
	return StaticPluginModules;
}

bool FEPluginManager::LoadPluginModule(const DiscoveredPlugin& Plugin, size_t ModuleIndex, FEPlugin& OutPlugin)
{
	if (ModuleIndex >= Plugin.Manifest.Modules.size())
	{
		LOG.Add("FEPluginManager::LoadPluginModule: Module index is out of range for plugin \"" + Plugin.Manifest.Name + "\"", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	// Statically linked module is compiled together with the engine, so there is no DLL to load and no compatibility to check.
	const std::unordered_map<std::string, StaticPluginModule>& StaticModules = GetStaticPluginModules();
	const auto StaticModuleIterator = StaticModules.find(Plugin.Manifest.Modules[ModuleIndex].Name);
	if (StaticModuleIterator != StaticModules.end())
	{
		OutPlugin.Manifest = new PluginManifest(Plugin.Manifest);
		OutPlugin.Module = &OutPlugin.Manifest->Modules[ModuleIndex];
		OutPlugin.Instance = StaticModuleIterator->second.CreateFunction();
		OutPlugin.DestroyFunction = StaticModuleIterator->second.DestroyFunction;
		OutPlugin.Instance->PluginDirectory = FILE_SYSTEM.GetAbsolutePath(Plugin.FolderPath) + "/";
		return true;
	}

#ifdef _DEBUG
	const std::string ConfigurationPostfix = "_Debug";
	const std::string ConfigurationFolder = "Debug";
#else
	const std::string ConfigurationPostfix = "_Release";
	const std::string ConfigurationFolder = "Release";
#endif

	const std::string& PluginFolderPath = Plugin.FolderPath;
	const PluginModule& Module = Plugin.Manifest.Modules[ModuleIndex];

	// Plugin folder contains DLLs for both configurations, named with configuration postfix.
	const std::string DLLFileName = FILE_SYSTEM.GetFileName(Module.EntryPointFileName, false) + ConfigurationPostfix + FILE_SYSTEM.GetFileExtension(Module.EntryPointFileName);
	std::string DLLPath = PluginFolderPath + "/" + DLLFileName;

	if (!FILE_SYSTEM.DoesFileExist(DLLPath))
	{
		LOG.Add("FEPluginManager::LoadPluginModule: DLL file \"" + DLLPath + "\" does not exist", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
		return false;
	}

	// DLLs that plugin depends on are stored in subfolder named after configuration.
	// Windows needs that folder in DLL search path to resolve them while plugin DLL is loading.
	const std::string DependencyFolderPath = PluginFolderPath + "/" + ConfigurationFolder;
	DLL_DIRECTORY_COOKIE DependencyDirectoryCookie = nullptr;
	if (FILE_SYSTEM.DoesDirectoryExist(DependencyFolderPath))
	{
		const std::string AbsoluteDependencyFolderPath = FILE_SYSTEM.GetAbsolutePath(DependencyFolderPath);
		const int WideLength = MultiByteToWideChar(CP_UTF8, 0, AbsoluteDependencyFolderPath.c_str(), -1, nullptr, 0);
		std::wstring WideDependencyFolderPath(WideLength > 0 ? WideLength - 1 : 0, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, AbsoluteDependencyFolderPath.c_str(), -1, WideDependencyFolderPath.data(), WideLength);

		DependencyDirectoryCookie = AddDllDirectory(WideDependencyFolderPath.c_str());
		if (DependencyDirectoryCookie == nullptr)
			LOG.Add("FEPluginManager::LoadPluginModule: Failed to add \"" + DependencyFolderPath + "\" to DLL search path", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
	}

	// LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR requires absolute path.
	HMODULE DLLHandle = LoadLibraryExA(FILE_SYSTEM.GetAbsolutePath(DLLPath).c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
	DWORD ErrorMessageID = DLLHandle == nullptr ? ::GetLastError() : 0;

	// Dependencies are resolved during loading, so folder is no longer needed in DLL search path.
	if (DependencyDirectoryCookie != nullptr)
		RemoveDllDirectory(DependencyDirectoryCookie);

	if (!DLLHandle)
	{
		if (ErrorMessageID == 0)
		{
			LOG.Add("FEPluginManager::LoadPluginModule: Failed to load DLL \"" + DLLPath + "\" without error message", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}

		LPSTR MessageBuffer = nullptr;
		size_t Size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, ErrorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&MessageBuffer, 0, NULL);

		std::string StringMessage(MessageBuffer, Size);
		LocalFree(MessageBuffer);

		// System message ends with line break.
		while (!StringMessage.empty() && (StringMessage.back() == '\n' || StringMessage.back() == '\r' || StringMessage.back() == ' '))
			StringMessage.pop_back();

		// DLL file exists (checked above), so this error means that some DLL that it depends on was not found.
		if (ErrorMessageID == ERROR_MOD_NOT_FOUND)
		{
			char ExecutablePath[MAX_PATH] = {};
			GetModuleFileNameA(nullptr, ExecutablePath, MAX_PATH);
			char SystemFolderPath[MAX_PATH] = {};
			GetSystemDirectoryA(SystemFolderPath, MAX_PATH);

			const std::vector<std::string> SearchFolders = { PluginFolderPath, DependencyFolderPath, FILE_SYSTEM.GetDirectoryPath(ExecutablePath), SystemFolderPath };
			const std::vector<std::string> MissingDependencies = FindMissingDLLDependencies(DLLPath, SearchFolders);
			if (MissingDependencies.empty())
			{
				StringMessage += " DLL that plugin depends on, or one of its own dependencies, was not found.";
			}
			else
			{
				StringMessage += " DLLs that plugin depends on were not found:";
				for (size_t k = 0; k < MissingDependencies.size(); k++)
					StringMessage += (k == 0 ? " " : ", ") + MissingDependencies[k];

				StringMessage += ".";
			}

			StringMessage += " Plugin dependencies can be placed in \"" + DependencyFolderPath + "\".";
		}

		LOG.Add("FEPluginManager::LoadPluginModule: Failed to load DLL \"" + DLLPath + "\" with error: " + StringMessage, "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	typedef bool (*IsCompiledInDebugMode_Function)(void);
	IsCompiledInDebugMode_Function IsCompiledInDebugMode = (IsCompiledInDebugMode_Function)GetProcAddress(DLLHandle, "IsCompiledInDebugMode");
	if (IsCompiledInDebugMode)
	{
		bool bDLLInDebug = IsCompiledInDebugMode();

#ifdef _DEBUG
		if (!bDLLInDebug)
		{
			LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" was compiled in release mode, while engine is in debug mode", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
			FreeLibrary(DLLHandle);
			return false;
		}
#else
		if (bDLLInDebug)
		{
			LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" was compiled in debug mode, while engine is in release mode", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
			FreeLibrary(DLLHandle);
			return false;
		}
#endif
	}

	// Plugin compiled with different FEPluginCore layout would crash when engine calls its functions, so it is not loaded.
	typedef int (*Get_Plugin_Interface_Version_Function)(void);
	Get_Plugin_Interface_Version_Function GetPluginInterfaceVersion = (Get_Plugin_Interface_Version_Function)GetProcAddress(DLLHandle, "GetPluginInterfaceVersion");
	if (!GetPluginInterfaceVersion)
	{
		LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" does not export GetPluginInterfaceVersion, plugin should be rebuilt with current engine", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	const int DLLPluginInterfaceVersion = GetPluginInterfaceVersion();
	if (DLLPluginInterfaceVersion != FEPluginCore::InterfaceVersion)
	{
		LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" was compiled with plugin interface version " + std::to_string(DLLPluginInterfaceVersion) +
				", while engine uses version " + std::to_string(FEPluginCore::InterfaceVersion) + ", plugin should be rebuilt with current engine", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	// FE_FIX_ME: Instead of timestamp we should use engine git commit hash and dirty flag.
	typedef unsigned long long (*Get_EngineHeaders_BuildVersion_Function)(void);
	Get_EngineHeaders_BuildVersion_Function GetEngineHeadersBuildVersion = (Get_EngineHeaders_BuildVersion_Function)GetProcAddress(DLLHandle, "GetEngineHeadersBuildVersion");
	if (GetEngineHeadersBuildVersion)
	{
		unsigned long long EngineHeadersBuildVersion = GetEngineHeadersBuildVersion();
		std::string BuildTimestamp = GetFOCAL_ENGINE_VersionInfo().BuildTimestamp;
		// Remove quotes from the timestamp.
		BuildTimestamp.erase(std::remove_if(BuildTimestamp.begin(), BuildTimestamp.end(),
			[](char Character) { return Character == '\"'; }),
			BuildTimestamp.end());

		unsigned long long EngineBuildVersion = std::stoull(BuildTimestamp);
		if (EngineHeadersBuildVersion != EngineBuildVersion)
		{
			// Currently we will just log this error, but in the future we should handle this more gracefully.
			LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" was compiled with different engine headers version, that could lead to crashes", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
		}
	}

	// Ensuring that the DLL has the required functions.
	typedef FEPluginCore* (*Create_Plugin_Function)(void);
	Create_Plugin_Function CreatePlugin = (Create_Plugin_Function)GetProcAddress(DLLHandle, "CreatePlugin");

	typedef void (*Destroy_Plugin_Function)(FEPluginCore*);
	Destroy_Plugin_Function DestroyPlugin = (Destroy_Plugin_Function)GetProcAddress(DLLHandle, "DestroyPlugin");

	if (!CreatePlugin || !DestroyPlugin)
	{
		LOG.Add("FEPluginManager::LoadPluginModule: DLL \"" + DLLPath + "\" does not export CreatePlugin/DestroyPlugin", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	FEPluginCore* Instance = CreatePlugin();
	if (Instance == nullptr)
	{
		LOG.Add("FEPluginManager::LoadPluginModule: CreatePlugin returned nullptr for DLL \"" + DLLPath + "\"", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
		FreeLibrary(DLLHandle);
		return false;
	}

	OutPlugin.Manifest = new PluginManifest(Plugin.Manifest);
	OutPlugin.Module = &OutPlugin.Manifest->Modules[ModuleIndex];
	OutPlugin.DLLPath = DLLPath;
	OutPlugin.DLLHandle = DLLHandle;
	OutPlugin.Instance = Instance;
	OutPlugin.DestroyFunction = DestroyPlugin;
	Instance->PluginDirectory = FILE_SYSTEM.GetAbsolutePath(PluginFolderPath) + "/";

	return true;
}

void FEPluginManager::Update(double DeltaTime)
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
		ActivePlugins[i].Instance->OnUpdate(DeltaTime);
}

void FEPluginManager::BeforeRender()
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
		ActivePlugins[i].Instance->OnBeforeRender();
}

void FEPluginManager::AfterRender()
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
		ActivePlugins[i].Instance->OnAfterRender();
}

void FEPluginManager::CheckScenesForModification()
{
	if (ActivePlugins.empty())
		return;

	std::vector<FEScene*> Scenes = SCENE_MANAGER.GetAllScenes();
	for (size_t i = 0; i < Scenes.size(); i++)
	{
		FEScene* Scene = Scenes[i];
		auto ReportedScene = ReportedScenes.find(Scene->GetID());
		if (ReportedScene == ReportedScenes.end())
		{
			ReportedScenes[Scene->GetID()] = Scene->Flags;
			for (size_t j = 0; j < ActivePlugins.size(); j++)
				ActivePlugins[j].Instance->OnSceneAdded(Scene);

			continue;
		}

		if (ReportedScene->second != Scene->Flags)
		{
			// Stored flags are updated before plugins are called, because they could add scenes and invalidate iterator.
			const FESceneFlag PreviousFlags = ReportedScene->second;
			ReportedScene->second = Scene->Flags;
			for (size_t j = 0; j < ActivePlugins.size(); j++)
				ActivePlugins[j].Instance->OnSceneFlagsChanged(Scene, PreviousFlags);
		}
	}
}

void FEPluginManager::SceneDeleting(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	// Plugins are notified only about scenes that were reported to them with OnSceneAdded.
	auto ReportedScene = ReportedScenes.find(Scene->GetID());
	if (ReportedScene == ReportedScenes.end())
		return;

	// New scene with the same ID (for example after project reload) should be reported again as added.
	ReportedScenes.erase(ReportedScene);
	for (size_t i = 0; i < ActivePlugins.size(); i++)
		ActivePlugins[i].Instance->OnSceneDeleting(Scene);
}

void FEPluginManager::Shutdown()
{
	// Modules loaded by applications could depend on engine modules, so they are unloaded first.
	if (BeforeShutdownCallback)
		BeforeShutdownCallback();

	// Unload in reverse order of loading.
	for (size_t i = ActivePlugins.size(); i > 0; i--)
	{
		FEPlugin& Plugin = ActivePlugins[i - 1];
		Plugin.Instance->OnUnload();
		UnloadPluginModule(Plugin);
	}

	ActivePlugins.clear();
	ReportedScenes.clear();
}

void FEPluginManager::UnloadPluginModule(FEPlugin& Plugin)
{
	if (Plugin.Instance != nullptr && Plugin.DestroyFunction != nullptr)
		Plugin.DestroyFunction(Plugin.Instance);

	if (Plugin.DLLHandle != nullptr)
		FreeLibrary(Plugin.DLLHandle);

	delete Plugin.Manifest;
	Plugin = FEPlugin();
}

bool FEPluginManager::IsModuleLoaded(const FEUUID& ModuleID) const
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
	{
		if (ActivePlugins[i].Module->ID == ModuleID)
			return true;
	}

	return false;
}

void FEPluginManager::SetBeforeShutdownCallback(std::function<void()> Callback)
{
	BeforeShutdownCallback = Callback;
}

FEPluginCore* FEPluginManager::GetPlugin(const FEUUID& PluginID)
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
	{
		if (ActivePlugins[i].Manifest->ID == PluginID)
			return ActivePlugins[i].Instance;
	}

	return nullptr;
}

FEPluginCore* FEPluginManager::GetPluginByName(const std::string& PluginName)
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
	{
		if (ActivePlugins[i].Manifest->Name == PluginName)
			return ActivePlugins[i].Instance;
	}

	return nullptr;
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

	if (!Root["Description"].isNull() && !Root["Description"].isString())
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
	Manifest.Description = Root["Description"].asString();

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

bool FEPluginManager::CreatePluginProject(std::string& PluginName, const std::string& VSProjectDirectory, bool bAddPublicInterface, bool bConfigureAndBuild)
{
	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::CreatePluginProject: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	// Replace all spaces with underscores.
	for (size_t i = 0; i < PluginName.size(); i++)
	{
		if (PluginName[i] == ' ')
			PluginName[i] = '_';
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::CreatePluginProject: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string ProjectDirectory = VSProjectDirectory;
	if (ProjectDirectory.back() != '/' && ProjectDirectory.back() != '\\')
		ProjectDirectory += '/';

	if (!GeneratePluginSourceFilesFromTemplate(PluginName, ProjectDirectory, bAddPublicInterface))
	{
		LOG.Add("FEPluginManager::CreatePluginProject: Error generating plugin source files from template", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Plugin ID is generated here, because manifest and public interface should have the same ID.
	const FEUUID PluginID = UNIQUE_ID.GenerateID();
	if (!GenerateManifestFromTemplate(PluginName, PluginID, ProjectDirectory))
	{
		LOG.Add("FEPluginManager::CreatePluginProject: Error generating manifest from template", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (bAddPublicInterface && !GeneratePublicInterfaceFromTemplate(PluginName, PluginID, ProjectDirectory))
	{
		LOG.Add("FEPluginManager::CreatePluginProject: Error generating public interface from template", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!InitializeProject(PluginName, ProjectDirectory, { ProjectDirectory + PluginName + ".h", ProjectDirectory + PluginName + ".cpp" }))
	{
		LOG.Add("FEPluginManager::CreatePluginProject: Error initializing project", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!bConfigureAndBuild)
		return true;

	if (!ConfigureAndBuildCMake(ProjectDirectory))
		return false;

	return RunVSProject(PluginName, ProjectDirectory);
}

bool FEPluginManager::GeneratePluginSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory, bool bAddPublicInterface)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::string TemplateHeaderFilePath = EnginePath + "/SubSystems/PluginSystem/Templates/PluginTemplate.h";
	std::string TemplateCPPFilePath = EnginePath + "/SubSystems/PluginSystem/Templates/PluginTemplate.cpp";

	if (!FILE_SYSTEM.CopyFileTo(TemplateHeaderFilePath, VSProjectDirectory + PluginName + ".h"))
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: Error copying file " + TemplateHeaderFilePath + " to " + VSProjectDirectory + PluginName + ".h", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFileTo(TemplateCPPFilePath, VSProjectDirectory + PluginName + ".cpp"))
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: Error copying file " + TemplateCPPFilePath + " to " + VSProjectDirectory + PluginName + ".cpp", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "class PLACE_HOLDER : public FEPluginCore";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	if (bAddPublicInterface)
	{
		const std::string InterfaceName = PluginName + "Interface";

		const std::string ConnectorInclude = "#include \"SubSystems/FocalEngine/SubSystems/PluginSystem/FEPluginConnector.h\"";
		CurrentInstruction.ContextPattern = ConnectorInclude;
		CurrentInstruction.TargetText = ConnectorInclude;
		CurrentInstruction.ReplacementText = ConnectorInclude + "\n#include \"Public/" + InterfaceName + ".h\"";
		Instructions.push_back(CurrentInstruction);

		// Rules are applied one after another, so class name is already replaced at this point.
		CurrentInstruction.ContextPattern = " : public FEPluginCore";
		CurrentInstruction.TargetText = " : public FEPluginCore";
		CurrentInstruction.ReplacementText = " : public FEPluginCore, public " + InterfaceName;
		Instructions.push_back(CurrentInstruction);
	}

	std::string PluginHeaderFilePath = VSProjectDirectory + PluginName + ".h";
	if (!FILE_SYSTEM.PerformTextReplacements(PluginHeaderFilePath, Instructions))
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: Error initializing " + PluginName + ".h", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.ContextPattern = "#include \"PluginTemplate.h\"";
	CurrentInstruction.TargetText = "PluginTemplate";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "REGISTER_PLUGIN(PLACE_HOLDER)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::OnLoad()";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::OnUnload()";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "void PLACE_HOLDER::OnUpdate(double DeltaTime)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	std::string PluginSourceFilePath = VSProjectDirectory + PluginName + ".cpp";
	if (!FILE_SYSTEM.PerformTextReplacements(PluginSourceFilePath, Instructions))
	{
		LOG.Add("FEPluginManager::GeneratePluginSourceFilesFromTemplate: Error initializing " + PluginName + ".cpp", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FEPluginManager::GenerateManifestFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::GenerateManifestFromTemplate: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::GenerateManifestFromTemplate: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::string TemplateManifestFilePath = EnginePath + "/SubSystems/PluginSystem/Templates/Manifest_Template.json";
	std::string ManifestFilePath = VSProjectDirectory + PluginName + ".json";

	if (!FILE_SYSTEM.CopyFileTo(TemplateManifestFilePath, ManifestFilePath))
	{
		LOG.Add("FEPluginManager::GenerateManifestFromTemplate: Error copying file " + TemplateManifestFilePath + " to " + ManifestFilePath, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = UNIQUE_ID.ToString(PluginID);
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "MODULE_ID_PLACE_HOLDER";
	CurrentInstruction.TargetText = "MODULE_ID_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = UNIQUE_ID.ToString(UNIQUE_ID.GenerateID());
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "MODULE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "MODULE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "ENTRY_POINT_FILE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "ENTRY_POINT_FILE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName + ".dll";
	Instructions.push_back(CurrentInstruction);

	if (!FILE_SYSTEM.PerformTextReplacements(ManifestFilePath, Instructions))
	{
		LOG.Add("FEPluginManager::GenerateManifestFromTemplate: Error initializing " + ManifestFilePath, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FEPluginManager::GeneratePublicInterfaceFromTemplate(const std::string& PluginName, const FEUUID& PluginID, const std::string& VSProjectDirectory)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::GeneratePublicInterfaceFromTemplate: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::GeneratePublicInterfaceFromTemplate: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string PublicDirectory = VSProjectDirectory + "Public/";
	if (!FILE_SYSTEM.DoesDirectoryExist(PublicDirectory))
	{
		if (!FILE_SYSTEM.MakeDirectory(PublicDirectory))
		{
			LOG.Add("FEPluginManager::GeneratePublicInterfaceFromTemplate: Error creating " + PublicDirectory + " directory", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	const std::string InterfaceName = PluginName + "Interface";
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::string TemplateInterfaceFilePath = EnginePath + "/SubSystems/PluginSystem/Templates/PluginInterfaceTemplate.h";
	std::string InterfaceFilePath = PublicDirectory + InterfaceName + ".h";

	if (!FILE_SYSTEM.CopyFileTo(TemplateInterfaceFilePath, InterfaceFilePath))
	{
		LOG.Add("FEPluginManager::GeneratePublicInterfaceFromTemplate: Error copying file " + TemplateInterfaceFilePath + " to " + InterfaceFilePath, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = InterfaceName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = UNIQUE_ID.ToString(PluginID);
	Instructions.push_back(CurrentInstruction);

	if (!FILE_SYSTEM.PerformTextReplacements(InterfaceFilePath, Instructions))
	{
		LOG.Add("FEPluginManager::GeneratePublicInterfaceFromTemplate: Error initializing " + InterfaceFilePath, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FEPluginManager::InitializeProject(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList)
{
	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::InitializeProject: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::InitializeProject: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (SourceFileFullPathList.empty())
	{
		LOG.Add("FEPluginManager::InitializeProject: Source file list is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(SourceFileFullPathList[i]))
		{
			LOG.Add("FEPluginManager::InitializeProject: Source file " + SourceFileFullPathList[i] + " does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	// Create all needed folders, the plugin folder is chosen by the user, so some of them could already exist.
	// Engine files are not copied, engine is added as git submodule.
	std::vector<std::string> FoldersToCreate = { "BuildManagement/" };
	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory + FoldersToCreate[i]))
			continue;

		if (!FILE_SYSTEM.MakeDirectory(VSProjectDirectory + FoldersToCreate[i]))
		{
			LOG.Add("FEPluginManager::InitializeProject: Error creating " + FoldersToCreate[i] + " directory", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ EnginePath + "/SubSystems/PluginSystem/Templates/CMakeLists_Template.txt", VSProjectDirectory + "CMakeLists.txt" });
	FilesToCopy.push_back({ EnginePath + "/SubSystems/PluginSystem/Templates/BuildOtherConfiguration.cmake", VSProjectDirectory + "BuildManagement/BuildOtherConfiguration.cmake" });
	// Plugin folder could already be a git repository with its own .gitignore.
	if (!FILE_SYSTEM.DoesFileExist(VSProjectDirectory + ".gitignore"))
		FilesToCopy.push_back({ EnginePath + "/SubSystems/PluginSystem/Templates/Gitignore_Template", VSProjectDirectory + ".gitignore" });

	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		FilesToCopy.push_back({ SourceFileFullPathList[i], VSProjectDirectory + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) });
		if (FilesToCopy.back().first == FilesToCopy.back().second)
			FilesToCopy.pop_back();
	}

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFileTo(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FEPluginManager::InitializeProject: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	if (!InitializeCMakeFile(PluginName, VSProjectDirectory, SourceFileFullPathList))
	{
		LOG.Add("FEPluginManager::InitializeProject: Error initializing CMakeLists.txt", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!AddEngineSubmodule(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::InitializeProject: Error adding engine submodule to plugin project", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEPluginManager::AddEngineSubmodule(const std::string& VSProjectDirectory)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::AddEngineSubmodule: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	// Trailing separator is removed, otherwise "D:\Plugin\" would escape closing quote on command line.
	std::string GitDirectory = VSProjectDirectory;
	while (!GitDirectory.empty() && (GitDirectory.back() == '/' || GitDirectory.back() == '\\'))
		GitDirectory.pop_back();

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory + ".git"))
	{
		std::string InitCommand = "git -C \"" + GitDirectory + "\" init";
		if (std::system(InitCommand.c_str()) != 0)
		{
			LOG.Add("FEPluginManager::AddEngineSubmodule: Error running git init, git should be installed and available in PATH", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory + "SubSystems/FocalEngine"))
	{
		std::string AddCommand = "git -C \"" + GitDirectory + "\" submodule add -b " + EngineRepositoryBranch + " " + EngineRepositoryURL + " SubSystems/FocalEngine";
		if (std::system(AddCommand.c_str()) != 0)
		{
			LOG.Add("FEPluginManager::AddEngineSubmodule: Error adding engine submodule from " + EngineRepositoryURL, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Engine has its own submodules, FEBasicApplication is needed to build engine.
	std::string UpdateCommand = "git -C \"" + GitDirectory + "\" submodule update --init --recursive";
	if (std::system(UpdateCommand.c_str()) != 0)
	{
		LOG.Add("FEPluginManager::AddEngineSubmodule: Error updating engine submodule", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEPluginManager::InitializeCMakeFile(const std::string& PluginName, const std::string& VSProjectDirectory, const std::vector<std::string>& SourceFileFullPathList)
{
	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::InitializeCMakeFile: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEPluginManager::InitializeCMakeFile: VSProjectDirectory is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	std::string SourceFilesList = "file(GLOB Main_SRC\n";
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFilesList += "\t\"" + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) + "\"\n";
	}
	SourceFilesList += ")";

	CurrentInstruction.ContextPattern = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.TargetText = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.ReplacementText = SourceFilesList;
	Instructions.push_back(CurrentInstruction);

	std::string CMakeFilePath = VSProjectDirectory + "CMakeLists.txt";
	if (!FILE_SYSTEM.PerformTextReplacements(CMakeFilePath, Instructions))
	{
		LOG.Add("FEPluginManager::InitializeCMakeFile: Error initializing " + CMakeFilePath, "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FEPluginManager::ConfigureAndBuildCMake(const std::string& VSProjectDirectory)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEPluginManager::ConfigureAndBuildCMake: VSProjectDirectory is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 18 2026";

	// CMake configure command.
	std::string ConfigureCommand = "cmake -S \"" + VSProjectDirectory + "\" -B \"" + VSProjectDirectory + "\" -G \"" + Generator + "\"";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FEPluginManager::ConfigureAndBuildCMake: Error running CMake configure command", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Post build step of Debug configuration also builds Release configuration.
	std::string BuildCommand = "cmake --build \"" + VSProjectDirectory + "\" --config Debug";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FEPluginManager::ConfigureAndBuildCMake: Error running CMake build command", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEPluginManager::RunVSProject(const std::string& PluginName, const std::string& VSProjectDirectory)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEPluginManager::RunVSProject: VSProjectDirectory is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string SolutionPath = VSProjectDirectory + PluginName + ".slnx";

	// Use ShellExecute to open the solution file
	HINSTANCE Result = ShellExecuteA(
		NULL,
		"open",
		SolutionPath.c_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL
	);

	// Check if ShellExecute was successful
	if ((INT_PTR)Result <= 32)
	{
		LOG.Add("FEPluginManager::RunVSProject: Error running solution", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}