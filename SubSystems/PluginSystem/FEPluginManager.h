#pragma once
#include "../../Core/FECoreIncludes.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEPluginManager
	{
		friend class FEngine;

		SINGLETON_PRIVATE_PART(FEPluginManager)
	public:
		SINGLETON_PUBLIC_PART(FEPluginManager)
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPluginManager();
	#define PLUGIN_MANAGER (*static_cast<FEPluginManager*>(GetPluginManager()))
#else
	#define PLUGIN_MANAGER FEPluginManager::GetInstance()
#endif
}