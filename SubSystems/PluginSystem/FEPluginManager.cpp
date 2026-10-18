#include "FEPluginManager.h"
#include "../../ResourceManager/FEResourceManager.h"
#include <shellapi.h>
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

#ifdef _DEBUG
	const std::string ConfigurationPostfix = "_Debug";
#else
	const std::string ConfigurationPostfix = "_Release";
#endif

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

		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			const PluginModule& Module = Manifest.Modules[j];
			if (Module.Type == "Engine")
			{
				// Plugin folder contains DLLs for both configurations, named with configuration postfix.
				const std::string DLLFileName = FILE_SYSTEM.GetFileName(Module.EntryPointFileName, false) + ConfigurationPostfix + FILE_SYSTEM.GetFileExtension(Module.EntryPointFileName);
				std::string DLLPath = PluginFolderPath + "/" + DLLFileName;

				if (!FILE_SYSTEM.DoesFileExist(DLLPath))
				{
					LOG.Add("FEPluginManager::Initialize: DLL file \"" + DLLPath + "\" does not exist", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
					continue;
				}

				HMODULE DLLHandle = LoadLibraryA(DLLPath.c_str());
				if (!DLLHandle)
				{
					DWORD ErrorMessageID = ::GetLastError();
					if (ErrorMessageID == 0)
					{
						LOG.Add("FEPluginManager::Initialize: Failed to load DLL \"" + DLLPath + "\" without error message", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
						continue;
					}

					LPSTR MessageBuffer = nullptr;
					size_t Size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, ErrorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&MessageBuffer, 0, NULL);

					std::string StringMessage(MessageBuffer, Size);
					LocalFree(MessageBuffer);

					LOG.Add("FEPluginManager::Initialize: Failed to load DLL \"" + DLLPath + "\" with error: " + StringMessage, "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
					continue;
				}

				typedef bool (*IsCompiledInDebugMode_Function)(void);
				IsCompiledInDebugMode_Function IsCompiledInDebugMode = (IsCompiledInDebugMode_Function)GetProcAddress(DLLHandle, "IsCompiledInDebugMode");
				if (IsCompiledInDebugMode)
				{
					bool bDLLInDebug = IsCompiledInDebugMode();

#ifdef _DEBUG
					if (!bDLLInDebug)
					{
						LOG.Add("FEPluginManager::Initialize: DLL \"" + DLLPath + "\" was compiled in release mode, while engine is in debug mode", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
						FreeLibrary(DLLHandle);
						continue;
					}
#else
					if (bDLLInDebug)
					{
						LOG.Add("FEPluginManager::Initialize: DLL \"" + DLLPath + "\" was compiled in debug mode, while engine is in release mode", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
						FreeLibrary(DLLHandle);
						continue;
					}
#endif
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
						LOG.Add("FEPluginManager::Initialize: DLL \"" + DLLPath + "\" was compiled with different engine headers version, that could lead to crashes", "FE_PLUGIN_MANAGER", FE_LOG_WARNING);
					}
				}

				// Ensuring that the DLL has the required functions.
				typedef FEPluginCore* (*Create_Plugin_Function)(void);
				Create_Plugin_Function CreatePlugin = (Create_Plugin_Function)GetProcAddress(DLLHandle, "CreatePlugin");

				typedef void (*Destroy_Plugin_Function)(FEPluginCore*);
				Destroy_Plugin_Function DestroyPlugin = (Destroy_Plugin_Function)GetProcAddress(DLLHandle, "DestroyPlugin");

				if (!CreatePlugin || !DestroyPlugin)
				{
					LOG.Add("FEPluginManager::Initialize: DLL \"" + DLLPath + "\" does not export CreatePlugin/DestroyPlugin", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
					FreeLibrary(DLLHandle);
					continue;
				}

				FEPluginCore* Instance = CreatePlugin();
				if (Instance == nullptr)
				{
					LOG.Add("FEPluginManager::Initialize: CreatePlugin returned nullptr for DLL \"" + DLLPath + "\"", "FE_PLUGIN_MANAGER", FE_LOG_ERROR);
					FreeLibrary(DLLHandle);
					continue;
				}

				FEPlugin Plugin;
				Plugin.Manifest = new PluginManifest(Manifest);
				Plugin.Module = &Plugin.Manifest->Modules[j];
				Plugin.DLLHandle = DLLHandle;
				Plugin.Instance = Instance;
				Plugin.DestroyFunction = DestroyPlugin;
				ActivePlugins.push_back(Plugin);

				Instance->OnLoad();
			}
		}
	}
}

void FEPluginManager::Update(double DeltaTime)
{
	for (size_t i = 0; i < ActivePlugins.size(); i++)
		ActivePlugins[i].Instance->OnUpdate(DeltaTime);
}

void FEPluginManager::Shutdown()
{
	// Unload in reverse order of loading.
	for (size_t i = ActivePlugins.size(); i > 0; i--)
	{
		FEPlugin& Plugin = ActivePlugins[i - 1];
		Plugin.Instance->OnUnload();
		Plugin.DestroyFunction(Plugin.Instance);
		FreeLibrary(Plugin.DLLHandle);
		delete Plugin.Manifest;
	}

	ActivePlugins.clear();
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

bool FEPluginManager::CreateNewVSProject(std::string& PluginName, const std::string& VSProjectDirectory)
{
	if (PluginName.empty())
	{
		LOG.Add("FEPluginManager::CreateNewVSProject: Plugin name is empty", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
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
		LOG.Add("FEPluginManager::CreateNewVSProject: VSProjectDirectory does not exist", "FE_PLUGIN_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string ProjectDirectory = VSProjectDirectory;
	if (ProjectDirectory.back() != '/' && ProjectDirectory.back() != '\\')
		ProjectDirectory += '/';

	if (!GeneratePluginSourceFilesFromTemplate(PluginName, ProjectDirectory))
	{
		LOG.Add("FEPluginManager::CreateNewVSProject: Error generating plugin source files from template", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!GenerateManifestFromTemplate(PluginName, ProjectDirectory))
	{
		LOG.Add("FEPluginManager::CreateNewVSProject: Error generating manifest from template", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!InitializeProject(PluginName, ProjectDirectory, { ProjectDirectory + PluginName + ".h", ProjectDirectory + PluginName + ".cpp" }))
	{
		LOG.Add("FEPluginManager::CreateNewVSProject: Error initializing project", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!ConfigureAndBuildCMake(ProjectDirectory))
		return false;

	return RunVSProject(PluginName, ProjectDirectory);
}

bool FEPluginManager::GeneratePluginSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory)
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

bool FEPluginManager::GenerateManifestFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory)
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
	CurrentInstruction.ReplacementText = UNIQUE_ID.ToString(UNIQUE_ID.GenerateID());
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
	std::vector<std::string> FoldersToCreate = { "SubSystems/", "SubSystems/FocalEngine/", "SubSystems/FocalEngine/SubSystems/", "SubSystems/FocalEngine/SubSystems/PluginSystem/", "BuildManagement/" };
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
	FilesToCopy.push_back({ EnginePath + "/SubSystems/PluginSystem/FEPluginConnector.cpp", VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/PluginSystem/FEPluginConnector.cpp" });

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

	if (!RESOURCE_MANAGER.CopyEngineFiles(true, false, true, VSProjectDirectory))
	{
		LOG.Add("FEPluginManager::InitializeProject: Error copying engine files in plugin project", "FE_PLUGIN_SYSTEM", FE_LOG_ERROR);
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