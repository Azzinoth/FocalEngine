#pragma once

#include "../Core/FEObject.h"
#include "../Core/Geometry/FEGeometry.h"
#include <any>

namespace FocalEngine
{
	class FENativeScriptCore
	{
		friend class FENativeScriptComponent;
		friend class FENativeScriptSystem;
	public:
		virtual ~FENativeScriptCore() = default;
		virtual void Awake() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(double DeltaTime) {}

		class FEEntity* ParentEntity = nullptr;
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
		std::function<FENativeScriptCore* ()> ConstructorFunction;
		std::string Name;
		bool bRunInEditor = false;
		std::unordered_map<std::string, FEScriptVariableInfo> VariablesRegistry;
	};
}