#pragma once

#include "../Core/FEObject.h"
#include "../Core/FEGeometricTools.h"
#include <any>

namespace FocalEngine
{
	class FENativeScriptCore
	{
		friend class FENativeScriptComponent;
		friend class FENativeScriptSystem;
	public:
		virtual ~FENativeScriptCore() = default;
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(double DeltaTime) {}

		class FEEntity* ParentEntity;
	};

	struct FEScriptVariableInfo
	{
		std::string Name;
		std::string Type;
		std::function<std::any(FENativeScriptCore*)> Getter;
		std::function<void(FENativeScriptCore*, const std::any&)> Setter;
	};

	struct FEScriptData
	{
	//private:
		friend class FENativeScriptComponent;
		friend class FENativeScriptSystem;

		std::function<FENativeScriptCore* ()> ConstructorFunction;
	//public:
		std::string Name;
		bool bRunInEditor = false;
		std::unordered_map<std::string, FEScriptVariableInfo> VariablesRegistry;
	};

	struct FENativeScriptModuleData
	{
		std::string Name;
		std::string DLLPath;
		std::string PDBPath;
		std::string ID;

		HMODULE DLLHandle;
		std::unordered_map<std::string, FEScriptData> Registry;
	};
}