#include "FEShaderUniform.h"
using namespace FocalEngine;

FEShaderUniformValue::FEShaderUniformValue()
{
}

Json::Value FEShaderUniformValue::ToJson() const
{
	Json::Value Result;
	if (IsType<bool>())
	{
		Result = GetValue<bool>();
	}
	else if (IsType<int>())
	{
		Result = GetValue<int>();
	}
	else if (IsType<unsigned int>())
	{
		Result = GetValue<unsigned int>();
	}
	else if (IsType<float>())
	{
		Result = GetValue<float>();
	}
	else if (IsType<glm::vec2>())
	{
		const glm::vec2 Vector = GetValue<glm::vec2>();
		Result["X"] = Vector.x;
		Result["Y"] = Vector.y;
	}
	else if (IsType<glm::vec3>())
	{
		const glm::vec3 Vector = GetValue<glm::vec3>();
		Result["X"] = Vector.x;
		Result["Y"] = Vector.y;
		Result["Z"] = Vector.z;
	}
	else if (IsType<glm::vec4>())
	{
		const glm::vec4 Vector = GetValue<glm::vec4>();
		Result["X"] = Vector.x;
		Result["Y"] = Vector.y;
		Result["Z"] = Vector.z;
		Result["W"] = Vector.w;
	}
	else if (IsType<glm::mat4>())
	{
		const glm::mat4 Matrix = GetValue<glm::mat4>();
		for (int Column = 0; Column < 4; Column++)
		{
			for (int Row = 0; Row < 4; Row++)
				Result.append(Matrix[Column][Row]);
		}
	}
	else
	{
		LOG.Add("FEShaderUniformValue::ToJson() unsupported uniform type for value: " + Name, "FE_LOG_RENDERING", FE_LOG_WARNING);
	}

	return Result;
}

void FEShaderUniformValue::FromJson(const Json::Value& Root)
{
	if (IsType<bool>())
	{
		SetValue<bool>(Root.asBool());
	}
	else if (IsType<int>())
	{
		SetValue<int>(Root.asInt());
	}
	else if (IsType<unsigned int>())
	{
		SetValue<unsigned int>(Root.asUInt());
	}
	else if (IsType<float>())
	{
		SetValue<float>(Root.asFloat());
	}
	else if (IsType<glm::vec2>())
	{
		glm::vec2 Vector;
		Vector.x = Root["X"].asFloat();
		Vector.y = Root["Y"].asFloat();
		SetValue<glm::vec2>(Vector);
	}
	else if (IsType<glm::vec3>())
	{
		glm::vec3 Vector;
		Vector.x = Root["X"].asFloat();
		Vector.y = Root["Y"].asFloat();
		Vector.z = Root["Z"].asFloat();
		SetValue<glm::vec3>(Vector);
	}
	else if (IsType<glm::vec4>())
	{
		glm::vec4 Vector;
		Vector.x = Root["X"].asFloat();
		Vector.y = Root["Y"].asFloat();
		Vector.z = Root["Z"].asFloat();
		Vector.w = Root["W"].asFloat();
		SetValue<glm::vec4>(Vector);
	}
	else if (IsType<glm::mat4>())
	{
		glm::mat4 Matrix;
		int Index = 0;
		for (int Column = 0; Column < 4; Column++)
		{
			for (int Row = 0; Row < 4; Row++)
			{
				Matrix[Column][Row] = Root[Index].asFloat();
				Index++;
			}
		}
		SetValue<glm::mat4>(Matrix);
	}
	else
	{
		LOG.Add("FEShaderUniformValue::FromJson() unsupported uniform type for value: " + Name, "FE_LOG_RENDERING", FE_LOG_WARNING);
	}
}

std::string FEShaderUniformValue::GetName() const
{
	return Name;
}

void FEShaderUniformValue::SetName(const std::string NewName)
{
	Name = NewName;
}

std::vector<FE_SHADER_UNIFORM_TYPE> FEShaderUniformValue::GetCompatibleTypes() const
{
	return CompatibleTypes;
}

FEShaderUniform::FEShaderUniform()
{
}

std::string FEShaderUniform::GetName() const
{
	return Name;
}

void FEShaderUniform::SetName(const std::string NewName)
{
	Name = NewName;
}

FEShaderUniform::~FEShaderUniform()
{
}

void FEShaderUniform::LoadUniformToGPU()
{
	switch (Type)
	{
		case FE_SHADER_UNIFORM_TYPE::FE_BOOL:
		{
			FE_GL_ERROR(glUniform1f(Locations[0], static_cast<float>(GetValue<bool>())));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_BOOL_ARRAY:
		{
			const auto& Array = GetValue<std::vector<bool>>();
			std::vector<float> FloatArray(Array.size());
			for (size_t i = 0; i < Array.size(); i++)
			{
				FloatArray[i] = static_cast<float>(Array[i]);
			}
			FE_GL_ERROR(glUniform1fv(Locations[0], static_cast<GLsizei>(Array.size()), FloatArray.data()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT:
		{
			FE_GL_ERROR(glUniform1ui(Locations[0], GetValue<unsigned int>()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT_ARRAY:
		{
			const auto& Array = GetValue<std::vector<unsigned int>>();
			FE_GL_ERROR(glUniform1uiv(Locations[0], static_cast<GLsizei>(Array.size()), Array.data()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_INT:
		{
			FE_GL_ERROR(glUniform1i(Locations[0], GetValue<int>()));
			break;
		}
	
		case FE_SHADER_UNIFORM_TYPE::FE_INT_ARRAY:
		{
			const auto& Array = GetValue<std::vector<int>>();
			// FE_FIX_ME: in 64-bit systems Array.size() will be 64 bit, but GL expects 32 bit, it could cause problems
			FE_GL_ERROR(glUniform1iv(Locations[0], static_cast<GLsizei>(Array.size()), Array.data()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT:
		{
			FE_GL_ERROR(glUniform1f(Locations[0], GetValue<float>()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_ARRAY:
		{
			const auto& Array = GetValue<std::vector<float>>();
			FE_GL_ERROR(glUniform1fv(Locations[0], static_cast<GLsizei>(Array.size()), Array.data()));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2:
		{
			const auto& Value = GetValue<glm::vec2>();
			FE_GL_ERROR(glUniform2f(Locations[0], Value.x, Value.y));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2_ARRAY:
		{
			const auto& Array = GetValue<std::vector<glm::vec2>>();
			FE_GL_ERROR(glUniform2fv(Locations[0], static_cast<GLsizei>(Array.size()), glm::value_ptr(Array[0])));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3:
		{
			const auto& Value = GetValue<glm::vec3>();
			FE_GL_ERROR(glUniform3f(Locations[0], Value.x, Value.y, Value.z));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3_ARRAY:
		{
			const auto& Array = GetValue<std::vector<glm::vec3>>();
			FE_GL_ERROR(glUniform3fv(Locations[0], static_cast<GLsizei>(Array.size()), glm::value_ptr(Array[0])));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4:
		{
			const auto& Value = GetValue<glm::vec4>();
			FE_GL_ERROR(glUniform4f(Locations[0], Value.x, Value.y, Value.z, Value.w));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4_ARRAY:
		{
			const auto& Array = GetValue<std::vector<glm::vec4>>();
			FE_GL_ERROR(glUniform4fv(Locations[0], static_cast<GLsizei>(Array.size()), glm::value_ptr(Array[0])));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_MAT4:
		{
			const glm::mat4& Matrix = GetValue<glm::mat4>();
			FE_GL_ERROR(glUniformMatrix4fv(Locations[0], 1, false, glm::value_ptr(Matrix)));
			break;
		}

		case FE_SHADER_UNIFORM_TYPE::FE_MAT4_ARRAY:
		{
			const auto& Array = GetValue<std::vector<glm::mat4>>();
			FE_GL_ERROR(glUniformMatrix4fv(Locations[0], static_cast<GLsizei>(Array.size()), false, glm::value_ptr(Array[0])));
			break;
		}

		default:
		{
			LOG.Add("FEShaderUniform::LoadUniformToGPU: Unknown uniform type for uniform: " + Name, "FE_LOG_RENDERING", FE_LOG_ERROR);
			break;
		}
	}
}

FE_SHADER_UNIFORM_TYPE FEShaderUniform::GetType() const
{
	return Type;
}

bool FEShaderUniform::IsTextureType(FE_SHADER_UNIFORM_TYPE Type)
{
	return std::find(ShaderUniformTextureTypes.begin(), ShaderUniformTextureTypes.end(), Type) != ShaderUniformTextureTypes.end();
}

bool FEShaderUniform::IsProvidedByEngine() const
{
	return bProvidedByEngine;
}