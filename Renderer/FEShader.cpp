#include "FEShader.h"

FocalEngine::FEShader::FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>&& attributes)
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

	locationProjectionMatrix = getUniformLocation("ProjectionMatrix");
	//if (locationProjectionMatrix == GLint(-1))
	//	FEError("shader didn't containe minimum needed uniforms.");
}

FocalEngine::FEShader::~FEShader()
{
	cleanUp();
}

GLuint FocalEngine::FEShader::loadShader(const char* shaderText, GLuint shaderType)
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

void FocalEngine::FEShader::bindAttribute(int& attribute, const char* variableName)
{
	glBindAttribLocation(programID, attribute, variableName);
}

void FocalEngine::FEShader::cleanUp()
{
	stop();
	//GL_ERROR(glDetachShader(programID, vertexShaderID));
	//GL_ERROR(glDetachShader(programID, fragmentShaderID));
	//GL_ERROR(glDeleteShader(vertexShaderID));
	//GL_ERROR(glDeleteShader(fragmentShaderID));
	glDeleteProgram(programID);
}

void FocalEngine::FEShader::bindAttributes()
{
	int index = 0;
	for (std::string attribute : attributes)
	{
		bindAttribute(index, attribute.c_str());
		index++;
	}
}

void FocalEngine::FEShader::start()
{
	glUseProgram(programID);
}

void FocalEngine::FEShader::stop()
{
	glUseProgram(0);
}

GLuint FocalEngine::FEShader::getUniformLocation(const char* name)
{
	return glGetUniformLocation(programID, name);
}

void FocalEngine::FEShader::loadFloat(GLuint& location, GLfloat& value)
{
	glUniform1f(location, value);
}

void FocalEngine::FEShader::loadInt(GLuint& location, GLint value)
{
	glUniform1i(location, value);
}

void FocalEngine::FEShader::loadVector(GLuint& location, glm::vec3& vector)
{
	glUniform3f(location, vector.x, vector.y, vector.z);
}

void FocalEngine::FEShader::loadVector(GLuint& location, glm::vec4& vector)
{
	glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
}

void FocalEngine::FEShader::loadVector(GLuint& location, glm::vec2& vector)
{
	glUniform2f(location, vector.x, vector.y);
}

void FocalEngine::FEShader::loadBool(GLuint& location, bool& value)
{
	GLfloat fValue = 0.0f;
	if (value) fValue = 1.0f;
	glUniform1f(location, fValue);
}

void FocalEngine::FEShader::loadMatrix(GLuint& location, glm::mat4& matrix)
{
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
}

void FocalEngine::FEShader::loadProjectionMatrix(glm::mat4& matrix)
{
	glUniformMatrix4fv(locationProjectionMatrix, 1, false, glm::value_ptr(matrix));
}

void FocalEngine::FEShader::loadModelViewProjection(glm::mat4& matrix)
{
	//glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
}

void FocalEngine::FEShader::loadData()
{

}