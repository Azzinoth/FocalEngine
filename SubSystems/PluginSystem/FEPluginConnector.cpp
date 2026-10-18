// DO NOT CHANGE THIS FILE.
#include "FEPluginConnector.h"
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