#pragma once

#include "FETexture.h"

namespace FocalEngine
{
	class FEShader
	{
	public:
		FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>&& attributes);
		~FEShader();

		virtual void start();
		virtual void stop();

		void bindAttribute(int& attribute, const char* variableName);

		void loadFloat(GLuint& location, GLfloat& value);
		void loadInt(GLuint& location, GLint value);
		void loadVector(GLuint& location, glm::vec3& vector);
		void loadVector(GLuint& location, glm::vec4& vector);
		void loadVector(GLuint& location, glm::vec2& vector);
		void loadBool(GLuint& location, bool& value);
		void loadMatrix(GLuint& location, glm::mat4& matrix);

		void loadModelViewProjection(glm::mat4& matrix);
		void loadProjectionMatrix(glm::mat4& matrix);

		virtual void loadData();
	private:
		GLuint programID;
		GLuint vertexShaderID;
		GLuint fragmentShaderID;
		std::vector<std::string> attributes;

		// standard uniforms
		GLuint locationProjectionMatrix;

		GLuint loadShader(const char* shaderText, GLuint shaderType);
		void cleanUp();
		void bindAttributes();
		GLuint getUniformLocation(const char* name);
	};
}
