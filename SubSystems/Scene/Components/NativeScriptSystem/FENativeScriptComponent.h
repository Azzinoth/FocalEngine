#pragma once

#include "FENativeScriptCore.h"

namespace FocalEngine
{
	class FENativeScriptFailedToLoadData
	{
		friend class FENativeScriptSystem;

		Json::Value RawData;
		FEUUID ModuleID;
	public:
		Json::Value GetRawData()
		{
			return RawData;
		}

		FEUUID GetModuleID()
		{
			return ModuleID;
		}
	};

	class FENativeScriptComponent
	{
		friend class FENativeScriptSystem;

		class FENativeScriptCore* CoreInstance = nullptr;
		FEUUID ModuleID;
		struct FEScriptData* ScriptData = nullptr;
		FENativeScriptFailedToLoadData* FailedToLoadData = nullptr;
	public:
		bool IsInitialized();
		FEUUID GetModuleID();

		class FENativeScriptCore* GetCoreInstance() const;
		const struct FEScriptData* GetScriptData() const;

        std::any GetVariableValueRaw(const std::string& VariableName) const;

        template<typename T>
		bool GetVariableValue(const std::string& VariableName, T& OutValue) const;
		bool SetVariableValue(std::string VariableName, std::any Value);

		void Awake();
		void OnDestroy();
		void OnUpdate(double DeltaTime);

		bool IsFailedToLoad();
		FENativeScriptFailedToLoadData* GetFailedToLoadData();
	};
#include "FENativeScriptComponent.inl"
}