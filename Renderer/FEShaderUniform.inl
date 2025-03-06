#pragma once

template<typename T>
FEShaderUniformValue::FEShaderUniformValue(const std::string& Name, const T& Data)
{
	this->Data = Data;
	this->Name = Name;
	TypeIndex = std::type_index(typeid(T));

	auto& TypeMap = GetCTypeToUniformTypesMap();
	auto TypeMapIterator = TypeMap.find(TypeIndex);
	if (TypeMapIterator != TypeMap.end())
	{
		CompatibleTypes = TypeMapIterator->second;
	}
	else
	{
		throw std::runtime_error("Unsupported uniform type");
	}
}

template<typename T>
const T& FEShaderUniformValue::GetValue() const
{
	return std::any_cast<const T&>(Data);
}

template<typename T>
bool FEShaderUniformValue::SetValue(const T& Value)
{
	if (TypeIndex != std::type_index(typeid(T)))
	{
		LOG.Add("FEShaderUniformValue::SetValue() failed, type mismatch", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	Data = Value;
	return true;
}

template<typename T>
bool FEShaderUniformValue::IsType() const
{
	return TypeIndex == std::type_index(typeid(T));
}

template<typename T>
FEShaderUniform::FEShaderUniform(FE_SHADER_UNIFORM_TYPE Type, const T& DataToInitialize, const std::string& Name, std::vector<GLuint>& Locations)
{
	if (Locations.empty())
	{
		LOG.Add("FEShaderUniform::FEShaderUniform() failed, Locations vector is empty!", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Name.empty())
	{
		LOG.Add("FEShaderUniform::FEShaderUniform() failed, uniform name is empty!", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	auto& TypeMap = FEShaderUniformValue::GetCTypeToUniformTypesMap();
	auto TypeMapIterator = TypeMap.find(std::type_index(typeid(T)));
	if (TypeMapIterator == TypeMap.end())
	{
		LOG.Add("FEShaderUniform::FEShaderUniform() failed, unsupported uniform type!", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	TypeIndex = std::type_index(typeid(T));
	this->Type = Type;

	CurrentValue = FEShaderUniformValue(Name, DataToInitialize);
	this->Name = Name;
	this->Locations = Locations;
}

template<typename T>
const T& FEShaderUniform::GetValue() const
{
	if (!CurrentValue.IsType<T>())
	{
		//LOG.Add("FEShaderUniform::GetValue() failed, type mismatch", "FE_LOG_RENDERING", FE_LOG_ERROR);
		//return T();
		// FIXME: Should return nullptr or similar instead of throwing an exception.
		throw std::runtime_error("Type mismatch");
	}

	return CurrentValue.GetValue<T>();
}

template<typename T>
bool FEShaderUniform::SetValue(const T& Value)
{
	if (TypeIndex != std::type_index(typeid(T)))
	{
		LOG.Add("FEShaderUniform::SetValue() failed, type mismatch", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	return CurrentValue.SetValue(Value);
}

template<typename T>
bool FEShaderUniform::IsType() const
{
	return TypeIndex == std::type_index(typeid(T));
}