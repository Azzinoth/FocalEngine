#include "FEShader.h"
using namespace FocalEngine;

FEShader::FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>& attributes)
{
	vertexShaderID = loadShader(vertexText, GL_VERTEX_SHADER);
	fragmentShaderID = loadShader(fragmentText, GL_FRAGMENT_SHADER);
	this->attributes = attributes;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	bindAttributes();
	glLinkProgram(programID);
	glValidateProgram(programID); // too slow ?

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	locationWorldMatrix = getUniformLocation("FEWorldMatrix");
	locationViewMatrix = getUniformLocation("FEViewMatrix");
	locationProjectionMatrix = getUniformLocation("FEProjectionMatrix");
	//if (locationProjectionMatrix == GLint(-1))
	//	FEError("shader didn't containe minimum needed uniforms.");
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

void FEShader::bindAttribute(int& attribute, const char* variableName)
{
	glBindAttribLocation(programID, attribute, variableName);
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
	int index = 0;
	for (std::string attribute : attributes)
	{
		bindAttribute(index, attribute.c_str());
		index++;
	}
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
	std::string parsedShaderText = shaderText;

	size_t index = parsedShaderText.find(FE_WORLD_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_WORLD_MATRIX_MACRO), "uniform mat4 FEWorldMatrix;");
		macroWorldMatrix = true;
	}

	index = parsedShaderText.find(FE_VIEW_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_VIEW_MATRIX_MACRO), "uniform mat4 FEViewMatrix;");
		macroViewMatrix = true;
	}

	index = parsedShaderText.find(FE_PROJECTION_MATRIX_MACRO);
	if (index != size_t(-1))
	{
		parsedShaderText.replace(index, strlen(FE_PROJECTION_MATRIX_MACRO), "uniform mat4 FEProjectionMatrix;");
		macroProjectionMatrix = true;
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

void FEShader::loadVector(const char* uniformName, glm::vec3& vector)
{
	glUniform3f(getUniformLocation(uniformName), vector.x, vector.y, vector.z);
}

void FEShader::loadVector(const char* uniformName, glm::vec4& vector)
{
	glUniform4f(getUniformLocation(uniformName), vector.x, vector.y, vector.z, vector.w);
}

void FEShader::loadVector(const char* uniformName, glm::vec2& vector)
{
	glUniform2f(getUniformLocation(uniformName), vector.x, vector.y);
}

void FEShader::loadMatrix(const char* uniformName, glm::mat4& matrix)
{
	glUniformMatrix4fv(getUniformLocation(uniformName), 1, false, glm::value_ptr(matrix));
}

void FEShader::loadWorldMatrix(glm::mat4& matrix)
{
	glUniformMatrix4fv(locationWorldMatrix, 1, false, glm::value_ptr(matrix));
}

void FEShader::loadViewMatrix(glm::mat4& matrix)
{
	glUniformMatrix4fv(locationViewMatrix, 1, false, glm::value_ptr(matrix));
}

void FEShader::loadProjectionMatrix(glm::mat4& matrix)
{
	glUniformMatrix4fv(locationProjectionMatrix, 1, false, glm::value_ptr(matrix));
}

void FEShader::loadModelViewProjection(glm::mat4& matrix)
{
	//glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
}

void FEShader::loadData()
{

}