// DO NOT CHANGE THIS FILE.
#pragma once
#include "../../FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) unsigned long long GetEngineHeadersBuildVersion();
	extern "C" __declspec(dllexport) bool IsCompiledInDebugMode();
	extern "C" __declspec(dllexport) int GetPluginInterfaceVersion();

	#define REGISTER_PLUGIN(PluginClass)                                              \
		extern "C" __declspec(dllexport) FEPluginCore* CreatePlugin() {               \
			return new PluginClass();                                                 \
		}                                                                             \
		extern "C" __declspec(dllexport) void DestroyPlugin(FEPluginCore* Plugin) {   \
			delete Plugin;                                                            \
		}
#else
	// Version is the one plugin library was compiled with, plugin manager compares it with its own.
	#define REGISTER_PLUGIN(PluginClass)                                              \
		namespace {                                                                   \
			struct Register##PluginClass {                                            \
				Register##PluginClass() {                                             \
					FEPluginManager::RegisterStaticPluginModule(#PluginClass,          \
						FEPluginCore::InterfaceVersion,                               \
						[]() -> FEPluginCore* { return new PluginClass(); },          \
						[](FEPluginCore* Plugin) { delete Plugin; });                 \
				}                                                                     \
			} Register##PluginClass##Instance;                                        \
		}
#endif