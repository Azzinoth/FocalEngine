#include "FEPluginManager.h"
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