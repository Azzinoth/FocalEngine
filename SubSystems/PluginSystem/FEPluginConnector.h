// DO NOT CHANGE THIS FILE.
#pragma once
#include "../../FEngine.h"
using namespace FocalEngine;

extern "C" __declspec(dllexport) unsigned long long GetEngineHeadersBuildVersion();
extern "C" __declspec(dllexport) bool IsCompiledInDebugMode();

#define REGISTER_PLUGIN(PluginClass)                                              \
	extern "C" __declspec(dllexport) FEPluginCore* CreatePlugin() {               \
		return new PluginClass();                                                 \
	}                                                                             \
	extern "C" __declspec(dllexport) void DestroyPlugin(FEPluginCore* Plugin) {   \
		delete Plugin;                                                            \
	}