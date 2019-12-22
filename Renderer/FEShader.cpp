#include "FEShader.h"
using namespace FocalEngine;

FEShader::FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>&& attributes)
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

	locationWorldMatrix = getUniformLocation("WorldMatrix");
	locationViewMatrix = getUniformLocation("ViewMatrix");
	locationProjectionMatrix = getUniformLocation("ProjectionMatrix");
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
	glShaderSource(shaderID, 1, &shaderText, nullptr);
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

GLuint FEShader::getUniformLocation(const char* name)
{
	return glGetUniformLocation(programID, name);
}

void FEShader::loadFloat(GLuint& location, GLfloat& value)
{
	glUniform1f(location, value);
}

void FEShader::loadInt(GLuint& location, GLint value)
{
	glUniform1i(location, value);
}

void FEShader::loadVector(GLuint& location, glm::vec3& vector)
{
	glUniform3f(location, vector.x, vector.y, vector.z);
}

void FEShader::loadVector(GLuint& location, glm::vec4& vector)
{
	glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
}

void FEShader::loadVector(GLuint& location, glm::vec2& vector)
{
	glUniform2f(location, vector.x, vector.y);
}

void FEShader::loadBool(GLuint& location, bool& value)
{
	GLfloat fValue = 0.0f;
	if (value) fValue = 1.0f;
	glUniform1f(location, fValue);
}

void FEShader::loadMatrix(GLuint& location, glm::mat4& matrix)
{
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
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