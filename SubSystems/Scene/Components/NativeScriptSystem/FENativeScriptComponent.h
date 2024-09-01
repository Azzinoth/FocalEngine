#pragma once

#include "FENativeScriptCore.h"

namespace FocalEngine
{
	class FENativeScriptComponent
	{
		friend class FENativeScriptSystem;
		class FENativeScriptCore* CoreInstance = nullptr;
		std::string ModuleID;
		struct FEScriptData* ScriptData = nullptr;
	public:
		bool IsInitialized();
		std::string GetModuleID();

		class FENativeScriptCore* GetCoreInstance() const;
		const struct FEScriptData* GetScriptData() const;

		bool SetVariableValue(std::string VariableName, std::any Value);

		void OnCreate();
		void OnDestroy();
		void OnUpdate(double DeltaTime);
	};
}