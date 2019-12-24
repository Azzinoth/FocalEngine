#pragma once

#include "FETexture.h"

#define FE_WORLD_MATRIX_MACRO "@WorldMatrix@"
#define FE_VIEW_MATRIX_MACRO "@ViewMatrix@"
#define FE_PROJECTION_MATRIX_MACRO "@ProjectionMatrix@"

namespace FocalEngine
{
	class FERenderer;
	class FEShader
	{
		friend FERenderer;
	public:
		FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>& attributes);
		~FEShader();

		virtual void start();
		virtual void stop();

		void bindAttribute(int& attribute, const char* variableName);

		void loadScalar(const char* uniformName, GLfloat& value);
		void loadScalar(const char* uniformName, GLint& value);
		void loadVector(const char* uniformName, glm::vec3& vector);
		void loadVector(const char* uniformName, glm::vec4& vector);
		void loadVector(const char* uniformName, glm::vec2& vector);
		void loadMatrix(const char* uniformName, glm::mat4& matrix);

		std::string parseShaderForMacro(const char* shaderText);

		void loadModelViewProjection(glm::mat4& matrix);
		void loadWorldMatrix(glm::mat4& matrix);
		void loadViewMatrix(glm::mat4& matrix);
		void loadProjectionMatrix(glm::mat4& matrix);

		virtual void loadData();
	private:
		GLuint programID;
		GLuint vertexShaderID;
		GLuint fragmentShaderID;
		std::vector<std::string> attributes;

		// standard uniforms
		bool macroWorldMatrix = false;
		GLuint locationWorldMatrix = -1;
		bool macroViewMatrix = false;
		GLuint locationViewMatrix = -1;
		bool macroProjectionMatrix = false;
		GLuint locationProjectionMatrix = -1;

		GLuint loadShader(const char* shaderText, GLuint shaderType);
		void cleanUp();
		void bindAttributes();
		GLuint getUniformLocation(const char* name);
	};
}
