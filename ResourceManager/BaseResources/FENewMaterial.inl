#pragma once

template<typename T>
bool FENewMaterial::UpdateUniformOverrideData(const std::string& UniformName, const T& Value)
{
	return SetUniformOverride(FEShaderUniformValue(UniformName, Value));
}