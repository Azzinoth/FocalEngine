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
		virtual void OnUnload() {}
		virtual void OnUpdate(double DeltaTime) {}
	};
}