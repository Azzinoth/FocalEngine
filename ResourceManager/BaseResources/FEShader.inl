#pragma once

template<typename T>
bool FEShader::UpdateUniformData(std::string UniformName, const T& Data)
{
	FEShaderUniform* Uniform = GetUniform(UniformName);
	if (Uniform == nullptr)
	{
		LOG.Add("FEShader::UpdateUniformData() uniform with name: " + UniformName + " not found!", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	return Uniform->SetValue(Data);
}