#include "FEShader.h"
using namespace FocalEngine;

FEShaderParam::FEShaderParam(int Data, std::string Name)
{
	data = new int(Data);
	type = FE_INT_SCALAR_UNIFORM;
	paramName = Name;
}

FEShaderParam::FEShaderParam(float Data, std::string Name)
{
	data = new float(Data);
	type = FE_FLOAT_SCALAR_UNIFORM;
	paramName = Name;
}

FEShaderParam::FEShaderParam(glm::vec2 Data, std::string Name)
{
	data = new glm::vec2(Data);
	type = FE_VECTOR2_UNIFORM;
	paramName = Name;
}

FEShaderParam::FEShaderParam(glm::vec3 Data, std::string Name)
{
	data = new glm::vec3(Data);
	type = FE_VECTOR3_UNIFORM;
	paramName = Name;
}

FEShaderParam::FEShaderParam(glm::vec4 Data, std::string Name)
{
	data = new glm::vec4(Data);
	type = FE_VECTOR4_UNIFORM;
	paramName = Name;
}

FEShaderParam::FEShaderParam(glm::mat4 Data, std::string Name)
{
	data = new glm::mat4(Data);
	type = FE_MAT4_UNIFORM;
	paramName = Name;
}

void FEShaderParam::updateData(int Data)
{
	if (type != FE_INT_SCALAR_UNIFORM)
		return; // to-do: error
	*(int*)data = Data;
}

void FEShaderParam::updateData(float Data)
{
	if (type != FE_FLOAT_SCALAR_UNIFORM)
		return; // to-do: error
	*(float*)data = Data;
}

void FEShaderParam::updateData(glm::vec2 Data)
{
	if (type != FE_VECTOR2_UNIFORM)
		return; // to-do: error
	*(glm::vec2*)data = Data;
}

void FEShaderParam::updateData(glm::vec3 Data)
{
	if (type != FE_VECTOR3_UNIFORM)
		return; // to-do: error
	*(glm::vec3*)data = Data;
}

void FEShaderParam::updateData(glm::vec4 Data)
{
	if (type != FE_VECTOR4_UNIFORM)
		return; // to-do: error
	*(glm::vec4*)data = Data;
}

void FEShaderParam::updateData(glm::mat4 Data)
{
	if (type != FE_MAT4_UNIFORM)
		return; // to-do: error
	*(glm::mat4*)data = Data;
}

void FEShaderParam::copyCode(const FEShaderParam& copy)
{
	switch (copy.type)
	{
		case FE_INT_SCALAR_UNIFORM:
		{
			data = new int;
			*(int*)data = *((int*)copy.data);
			break;
		}

		case FE_FLOAT_SCALAR_UNIFORM:
		{
			data = new float;
			*(float*)data = *((float*)copy.data);
			break;
		}

		case FE_VECTOR2_UNIFORM:
		{
			data = new glm::vec2;
			*(glm::vec2*)data = *((glm::vec2*)copy.data);
			break;
		}

		case FE_VECTOR3_UNIFORM:
		{
			data = new glm::vec3;
			*(glm::vec3*)data = *((glm::vec3*)copy.data);
			break;
		}

		case FE_VECTOR4_UNIFORM:
		{
			data = new glm::vec4;
			*(glm::vec4*)data = *((glm::vec4*)copy.data);
			break;
		}

		case FE_MAT4_UNIFORM:
		{
			data = new glm::mat4;
			*(glm::mat4*)data = *((glm::mat4*)copy.data);
			break;
		}

		default:
			break;
	}
}

FEShaderParam::FEShaderParam(const FEShaderParam& copy)
{
	this->type = copy.type;
	this->paramName = copy.paramName;

	copyCode(copy);
}

void FEShaderParam::operator=(const FEShaderParam& assign)
{
	this->~FEShaderParam();
	this->type = assign.type;
	this->paramName = assign.paramName;

	copyCode(assign);
}

FEShaderParam::~FEShaderParam()
{
	switch (type)
	{
		case FE_INT_SCALAR_UNIFORM:
		{
			delete (int*)data;
			break;
		}
	
		case FE_FLOAT_SCALAR_UNIFORM:
		{
			delete (float*)data;
			break;
		}
	
		case FE_VECTOR2_UNIFORM:
		{
			delete (glm::vec2*)data;
			break;
		}
	
		case FE_VECTOR3_UNIFORM:
		{
			delete (glm::vec3*)data;
			break;
		}
	
		case FE_VECTOR4_UNIFORM:
		{
			delete (glm::vec4*)data;
			break;
		}
	
		case FE_MAT4_UNIFORM:
		{
			delete (glm::mat4*)data;
			break;
		}
	
		default:
			break;
	}
}

std::string FEShaderParam::getParamName()
{
	return paramName;
}

void FEShaderParam::setParamName(std::string newName)
{
	paramName = newName;
}

FEShader::FEShader(const char* vertexText, const char* fragmentText)
{
	vertexShaderID = loadShader(vertexText, GL_VERTEX_SHADER);
	fragmentShaderID = loadShader(fragmentText, GL_FRAGMENT_SHADER);

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	bindAttributes();
	glLinkProgram(programID);
	glValidateProgram(programID); // too slow ?

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}

FEShader::~FEShader()
{
	cleanUp();
}

GLuint FEShader::loadShader(const char* shaderText, GLuint shaderType)
{
	GLuint shaderID;
	shaderID = glCreateShader(shaderType);

	std::string tempString = parseShaderForMacro(shaderText);
	const char *parsedShaderText = tempString.c_str();
	glShaderSource(shaderID, 1, &parsedShaderText, nullptr);
	glCompileShader(shaderID);
	GLint status = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<GLchar> errorLog(logSize);

		glGetShaderInfoLog(shaderID, logSize, &logSize, &errorLog[0]);
		assert(status);
	}

	return shaderID;
}

void FEShader::cleanUp()
{
	stop();
	//GL_ERROR(glDetachShader(programID, vertexShaderID));
	//GL_ERROR(glDetachShader(programID, fragmentShaderID));
	//GL_ERROR(glDeleteShader(vertexShaderID));
	//GL_ERROR(glDeleteShader(fragmentShaderID));
	glDeleteProgram(programID);
}

void FEShader::bindAttributes()
{
	if ((vertexAttributes & FE_POSITION) == FE_POSITION) glBindAttribLocation(programID, 0, "FEPosition");
	if ((vertexAttributes & FE_COLOR) == FE_COLOR) glBindAttribLocation(programID, 1, "FEColor");
	if ((vertexAttributes & FE_NORMAL) == FE_NORMAL) glBindAttribLocation(programID, 2, "FENormal");
	if ((vertexAttributes & FE_TANGENTS) == FE_TANGENTS) glBindAttribLocation(programID, 3, "FETangent");
	if ((vertexAttributes & FE_UV) == FE_UV) glBindAttribLocation(programID, 4, "FETexCoord");
}

void FEShader::start()
{
	glUseProgram(programID);
}

void FEShader::stop()
{
	glUseProgram(0);
}

std::string FEShader::parseShaderForMacro(const char* shaderText)
{
	size_t index = -1;
	std::string parsedShaderText = shaderText;

	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_POSITION);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_POSITION), "layout (location = 0) in vec3 FEPosition;");
		vertexAttributes |= FE_POSITION;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_COLOR);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_COLOR), "layout (location = 1) in vec3 FEColor;");
		vertexAttributes |= FE_COLOR;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_NORMAL);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_NORMAL), "layout (location = 2) in vec3 FENormal;");
		vertexAttributes |= FE_NORMAL;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_TANGENT);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_TANGENT), "layout (location = 3) in vec3 FETangent;");
		vertexAttributes |= FE_TANGENTS;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_UV);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_UV), "layout (location = 4) in vec3 FETexCoord;");
		vertexAttributes |= FE_UV;
	}

	index = parsedShaderText.find(FE_WORLD_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_WORLD_MATRIX_MACRO), "uniform mat4 FEWorldMatrix;");
		params.push_back(FEShaderParam(glm::mat4(1.0f), "FEWorldMatrix"));
	}

	index = parsedShaderText.find(FE_VIEW_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VIEW_MATRIX_MACRO), "uniform mat4 FEViewMatrix;");
		params.push_back(FEShaderParam(glm::mat4(1.0f), "FEViewMatrix"));
	}

	index = parsedShaderText.find(FE_PROJECTION_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_PROJECTION_MATRIX_MACRO), "uniform mat4 FEProjectionMatrix;");
		params.push_back(FEShaderParam(glm::mat4(1.0f), "FEProjectionMatrix"));
	}

	return parsedShaderText;
}

GLuint FEShader::getUniformLocation(const char* name)
{
	return glGetUniformLocation(programID, name);
}

void FEShader::loadScalar(const char* uniformName, GLfloat& value)
{
	glUniform1f(getUniformLocation(uniformName), value);
}

void FEShader::loadScalar(const char* uniformName, GLint& value)
{
	glUniform1i(getUniformLocation(uniformName), value);
}

void FEShader::loadVector(const char* uniformName, glm::vec2& vector)
{
	glUniform2f(getUniformLocation(uniformName), vector.x, vector.y);
}

void FEShader::loadVector(const char* uniformName, glm::vec3& vector)
{
	glUniform3f(getUniformLocation(uniformName), vector.x, vector.y, vector.z);
}

void FEShader::loadVector(const char* uniformName, glm::vec4& vector)
{
	glUniform4f(getUniformLocation(uniformName), vector.x, vector.y, vector.z, vector.w);
}

void FEShader::loadMatrix(const char* uniformName, glm::mat4& matrix)
{
	glUniformMatrix4fv(getUniformLocation(uniformName), 1, false, glm::value_ptr(matrix));
}

void FEShader::loadDataToGPU()
{
	for (size_t i = 0; i < params.size(); i++)
	{
		if (params[i].data == nullptr)
			continue;

		switch (params[i].type)
		{
			case FE_INT_SCALAR_UNIFORM:
			{
				loadScalar(params[i].getParamName().c_str(), *(int*)params[i].data);
				break;
			}

			case FE_FLOAT_SCALAR_UNIFORM:
			{
				loadScalar(params[i].getParamName().c_str(), *(float*)params[i].data);
				break;
			}

			case FE_VECTOR2_UNIFORM:
			{
				loadVector(params[i].getParamName().c_str(), *(glm::vec2*)params[i].data);
				break;
			}

			case FE_VECTOR3_UNIFORM:
			{
				loadVector(params[i].getParamName().c_str(), *(glm::vec3*)params[i].data);
				break;
			}

			case FE_VECTOR4_UNIFORM:
			{
				loadVector(params[i].getParamName().c_str(), *(glm::vec4*)params[i].data);
				break;
			}

			case FE_MAT4_UNIFORM:
			{
				loadMatrix(params[i].getParamName().c_str(), *(glm::mat4*)params[i].data);
				break;
			}

			default:
				break;
		}
	}
}

void FEShader::addParams(std::vector<FEShaderParam> Params)
{
	for (size_t i = 0; i < Params.size(); i++)
	{
		params.push_back(Params[i]);
	}
}

void FEShader::addParams(FEShaderParam Params)
{
	params.push_back(Params);
}

FEShaderParam& FEShader::getParam(std::string name)
{
	for (size_t i = 0; i < params.size(); i++)
	{
		if (params[i].getParamName() == name)
		{
			return params[i];
		}
	}
	//do-to: fix warning
}
