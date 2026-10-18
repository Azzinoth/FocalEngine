#pragma once
#include "../../Core/FECoreIncludes.h"

namespace FocalEngine
{
	class FEPluginCore
	{
		friend class FEPluginManager;
	public:
		virtual ~FEPluginCore() = default;
		virtual void OnLoad() {}
		// Plugin must remove everything it registered with the engine, its DLL is unloaded right after.
		virtual void OnUnload() {}
		virtual void OnUpdate(double DeltaTime) {}
	};
}