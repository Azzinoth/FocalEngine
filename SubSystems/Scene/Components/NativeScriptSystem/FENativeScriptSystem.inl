#pragma once

template<typename T>
T* FENativeScriptSystem::CastScript(FENativeScriptCore* Core)
{
	if (Core == nullptr)
	{
		LOG.Add("FENativeScriptSystem::CastScript: Core is nullptr", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	static_assert(std::is_base_of<FENativeScriptCore, T>::value,
		"T must inherit from FENativeScriptCore");

	T* CastedScript = dynamic_cast<T*>(Core);
	if (CastedScript == nullptr)
		LOG.Add("FENativeScriptSystem::CastScript: Failed to cast to " + std::string(typeid(T).name()), "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);

	return CastedScript;
}

template<typename T>
T* FENativeScriptSystem::CastToScriptClass(FENativeScriptComponent& Component)
{
	return CastScript<T>(Component.GetCoreInstance());
}

template<typename T>
void FENativeScriptSystem::SaveVariableTTypeToJSON(Json::Value& Root, std::any AnyValue)
{
	T Value = std::any_cast<T>(AnyValue);

	if constexpr (std::is_same_v<T, int>) Root = Value;
	else if constexpr (std::is_same_v<T, float>) Root = Value;
	else if constexpr (std::is_same_v<T, double>) Root = Value;
	else if constexpr (std::is_same_v<T, bool>) Root = Value;
	else if constexpr (std::is_same_v<T, std::string>) Root = Value;
	else if constexpr (std::is_same_v<T, glm::vec2>)
	{
		Root["X"] = Value.x;
		Root["Y"] = Value.y;
	}
	else if constexpr (std::is_same_v<T, glm::vec3>)
	{
		Root["X"] = Value.x;
		Root["Y"] = Value.y;
		Root["Z"] = Value.z;
	}
	else if constexpr (std::is_same_v<T, glm::vec4>)
	{
		Root["X"] = Value.x;
		Root["Y"] = Value.y;
		Root["Z"] = Value.z;
		Root["W"] = Value.w;
	}
	else if constexpr (std::is_same_v<T, glm::quat>)
	{
		Root["X"] = Value.x;
		Root["Y"] = Value.y;
		Root["Z"] = Value.z;
		Root["W"] = Value.w;
	}
	else if constexpr (std::is_same_v<T, FEPrefab*>)
	{
		Root["ID"] = Value == nullptr ? "" : Value->GetObjectID();
	}
}

template<typename T>
void FENativeScriptSystem::SaveArrayVariableTTypeToJSON(Json::Value& Root, std::any AnyValue)
{
	std::vector<T> Value = std::any_cast<std::vector<T>>(AnyValue);
	for (size_t i = 0; i < Value.size(); i++)
	{
		Json::Value Element;
		SaveVariableTTypeToJSON<T>(Element, Value[i]);
		Root[std::to_string(i)] = Element;
	}
}

template<typename T>
std::any FENativeScriptSystem::LoadVariableTTypeToJSON(const Json::Value& Root)
{
	if constexpr (std::is_same_v<T, int>)
	{
		T Value = Root.as<T>();
		return Value;
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		T Value = Root.as<T>();
		return Value;
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		T Value = Root.as<T>();
		return Value;
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		T Value = Root.as<T>();
		return Value;
	}
	else if constexpr (std::is_same_v<T, std::string>)
	{
		T Value = Root.asString();
		return Value;
	}
	else if constexpr (std::is_same_v<T, glm::vec2>)
	{
		glm::vec2 Value;
		Value.x = Root["X"].as<float>();
		Value.y = Root["Y"].as<float>();

		return Value;
	}
	else if constexpr (std::is_same_v<T, glm::vec3>)
	{
		glm::vec3 Value;
		Value.x = Root["X"].as<float>();
		Value.y = Root["Y"].as<float>();
		Value.z = Root["Z"].as<float>();

		return Value;
	}
	else if constexpr (std::is_same_v<T, glm::vec4>)
	{
		glm::vec4 Value;
		Value.x = Root["X"].as<float>();
		Value.y = Root["Y"].as<float>();
		Value.z = Root["Z"].as<float>();
		Value.w = Root["W"].as<float>();

		return Value;
	}
	else if constexpr (std::is_same_v<T, glm::quat>)
	{
		glm::quat Value;
		Value.x = Root["X"].as<float>();
		Value.y = Root["Y"].as<float>();
		Value.z = Root["Z"].as<float>();
		Value.w = Root["W"].as<float>();

		return Value;
	}
	else if constexpr (std::is_same_v<T, FEPrefab*>)
	{
		FEPrefab* Value = nullptr;
		std::string PrefabID = Root["ID"].asString();
		if (!PrefabID.empty())
			Value = RESOURCE_MANAGER.GetPrefab(PrefabID);

		return Value;
	}

	return T();
}

template<typename T>
std::any FENativeScriptSystem::LoadArrayVariableTTypeToJSON(const Json::Value& Root)
{
	std::vector<T> Value;
	for (size_t i = 0; i < Root.size(); i++)
	{
		T Element = std::any_cast<T>(LoadVariableTTypeToJSON<T>(Root[std::to_string(i)]));
		Value.push_back(Element);
	}

	return Value;
}

template<typename T>
bool FENativeScriptSystem::IsEqualTType(std::any FirstScriptVariable, std::any SecondScriptVariable)
{
	if (FirstScriptVariable.type() != typeid(T) || SecondScriptVariable.type() != typeid(T))
		return false;

	try
	{
		return std::any_cast<T>(FirstScriptVariable) == std::any_cast<T>(SecondScriptVariable);
	}
	catch (const std::bad_any_cast& e)
	{
		LOG.Add("FENativeScriptSystem::IsEqual: " + std::string(e.what()), "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

template<typename T>
bool FENativeScriptSystem::IsEqualArrayTType(std::any FirstScriptVariable, std::any SecondScriptVariable)
{
	if (FirstScriptVariable.type() != typeid(std::vector<T>) || SecondScriptVariable.type() != typeid(std::vector<T>))
		return false;

	std::vector<T> FirstArray = std::any_cast<std::vector<T>>(FirstScriptVariable);
	std::vector<T> SecondArray = std::any_cast<std::vector<T>>(SecondScriptVariable);

	if (FirstArray.size() != SecondArray.size())
		return false;

	for (size_t i = 0; i < FirstArray.size(); i++)
	{
		if (FirstArray[i] != SecondArray[i])
			return false;
	}

	return true;
}

template<typename T>
std::any FENativeScriptSystem::CreateEngineLocalScriptVariableCopyTemplated(std::any Value)
{
	try
	{
		if constexpr (std::is_pointer_v<T>)
		{
			// Handle pointer types
			auto Pointer = std::any_cast<T>(Value);
			if (Pointer == nullptr)
			{
				return std::any();
			}
			else
			{
				// Currently we support only childs of FEObject.
				if (std::is_base_of<FEObject, std::remove_pointer_t<T>>::value)
				{
					std::string ObjectID = Pointer->GetObjectID();
					FEObject* LocalPointer = OBJECT_MANAGER.GetFEObject(ObjectID);
					return std::any(static_cast<T>(LocalPointer));
				}
				else
				{
					LOG.Add("FENativeScriptSystem::CreateEngineLocalScriptVariableCopyTemplated failed to create local copy of pointer type because it is not a child of FEObject.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
					return std::any();
				}
			}
		}
		else
		{
			// Handle non-pointer types
			T LocalCopy = std::any_cast<T>(Value);
			return std::any_cast<T>(LocalCopy);
		}
	}
	catch (const std::bad_any_cast& Error)
	{
		LOG.Add("FENativeScriptSystem::CreateEngineLocalScriptVariableCopyTemplated failed to create local copy of variable. Error: " + std::string(Error.what()), "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::any();
	}
}

template<typename T>
std::vector<FEEntity*> FENativeScriptSystem::GetEntityListWithScript(FEScene* Scene)
{
	std::vector<FEEntity*> Result;
	for (auto& EntityID : Scene->GetEntityIDList())
	{
		FEEntity* Entity = Scene->GetEntity(EntityID);
		if (Entity->HasComponent<FENativeScriptComponent>())
		{
			FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
			T* Script = CastScript<T>(NativeScriptComponent.GetCoreInstance());
			if (Script != nullptr)
				Result.push_back(Entity);
		}
	}
	return Result;
}

template<typename T>
std::vector<T*> FENativeScriptSystem::GetScriptList(FEScene* Scene)
{
	std::vector<T*> Result;
	std::vector<FEEntity*> EntityList = GetEntityListWithScript<T>(Scene);
	for (auto& Entity : EntityList)
	{
		FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
		Result.push_back(CastToScriptClass<T>(NativeScriptComponent));
	}

	return Result;
}