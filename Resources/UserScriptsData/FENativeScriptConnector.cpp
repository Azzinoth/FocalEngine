// DO NOT CHANGE THIS FILE.
#include "FENativeScriptConnector.h"

FECoreScriptManager::FECoreScriptManager() {};
FECoreScriptManager::~FECoreScriptManager() {};

extern "C" __declspec(dllexport) const char* GetModuleID()
{
	return CORE_SCRIPT_MANAGER.CurrentModuleID.c_str();
}

extern "C" __declspec(dllexport) void* GetScriptRegistry()
{
	return &CORE_SCRIPT_MANAGER.GetRegistry();
}

#include "ResourceManager/Timestamp.h"
extern "C" __declspec(dllexport) unsigned long long GetEngineHeadersBuildVersion()
{
	std::string StringRepresentation = ENGINE_BUILD_TIMESTAMP;
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