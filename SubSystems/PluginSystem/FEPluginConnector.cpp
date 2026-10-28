// DO NOT CHANGE THIS FILE.
#include "FEPluginConnector.h"

// Statically linked plugin passes its interface version through REGISTER_PLUGIN instead, and exports of several plugins would conflict in one executable.
#ifdef FOCAL_ENGINE_SHARED
#include "Core/VersionInfo/FOCAL_ENGINE_Version.h"

extern "C" __declspec(dllexport) unsigned long long GetEngineHeadersBuildVersion()
{
	std::string StringRepresentation = FOCAL_ENGINE__BUILD_TIMESTAMP;
	unsigned long long BuildVersion = std::stoull(StringRepresentation.c_str());
	return BuildVersion;
}

extern "C" __declspec(dllexport) bool IsCompiledInDebugMode()
{
#ifdef _DEBUG
	return true;
#else
	return false;
#endif
}

// Returns version that plugin was compiled with, plugin manager compares it with its own.
extern "C" __declspec(dllexport) int GetPluginInterfaceVersion()
{
	return FEPluginCore::InterfaceVersion;
}
#endif