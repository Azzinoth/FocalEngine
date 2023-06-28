#pragma once

template<typename T>
void FEShader::UpdateParameterData(std::string Name, const T& Data)
{
	LOG.Add("FEShader::UpdateParameterData() unknown type", "FE_LOG_RENDERING", FE_LOG_ERROR);
	return;
}

template<>
void FEShader::UpdateParameterData<void*>(std::string Name, void* const& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	switch (Type)
	{
		case FE_BOOL_UNIFORM:
		{
			*static_cast<bool*>(Parameter->Data) = *static_cast<bool*>(Data);
			break;
		}
	
		case FE_INT_SCALAR_UNIFORM:
		{
			*static_cast<int*>(Parameter->Data) = *static_cast<int*>(Data);
			break;
		}
	
		case FE_FLOAT_SCALAR_UNIFORM:
		{
			*static_cast<float*>(Parameter->Data) = *static_cast<float*>(Data);
			break;
		}
	
		case FE_VECTOR2_UNIFORM:
		{
			*static_cast<glm::vec2*>(Parameter->Data) = *static_cast<glm::vec2*>(Data);
			break;
		}
	
		case FE_VECTOR3_UNIFORM:
		{
			*static_cast<glm::vec3*>(Parameter->Data) = *static_cast<glm::vec3*>(Data);
			break;
		}
	
		case FE_VECTOR4_UNIFORM:
		{
			*static_cast<glm::vec4*>(Parameter->Data) = *static_cast<glm::vec4*>(Data);
			break;
		}
	
		case FE_MAT4_UNIFORM:
		{
			*static_cast<glm::mat4*>(Parameter->Data) = *static_cast<glm::mat4*>(Data);
			break;
		}
	
		default:
			break;
	}
}

template<>
void FEShader::UpdateParameterData<bool>(std::string Name, const bool& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_BOOL_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<bool*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<int>(std::string Name, const int& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_INT_SCALAR_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<int*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<float>(std::string Name, const float& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_FLOAT_SCALAR_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<float*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<glm::vec2>(std::string Name, const glm::vec2& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_VECTOR2_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<glm::vec2*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<glm::vec3>(std::string Name, const glm::vec3& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_VECTOR3_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<glm::vec3*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<glm::vec4>(std::string Name, const glm::vec4& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_VECTOR4_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<glm::vec4*>(Parameter->Data) = Data;
}

template<>
void FEShader::UpdateParameterData<glm::mat4>(std::string Name, const glm::mat4& Data)
{
	FEShaderParam* Parameter = GetParameter(Name);
	if (Parameter == nullptr)
	{
		LOG.Add("FEShader::UpdateParameterData() parameter not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Parameter->Type != FE_MAT4_UNIFORM)
	{
		LOG.Add("FEShader::UpdateParameterData() incorrect type", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	*static_cast<glm::mat4*>(Parameter->Data) = Data;
}