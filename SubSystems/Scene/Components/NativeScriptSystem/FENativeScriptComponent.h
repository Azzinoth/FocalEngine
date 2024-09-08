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

        std::any GetVariableValueRaw(const std::string& VariableName) const;

        template<typename T>
		bool GetVariableValue(const std::string& VariableName, T& OutValue) const;
		bool SetVariableValue(std::string VariableName, std::any Value);

		void Awake();
		void OnDestroy();
		void OnUpdate(double DeltaTime);
	};
#include "FENativeScriptComponent.inl"
}