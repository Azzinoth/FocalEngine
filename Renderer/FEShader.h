#pragma once

#include "FETexture.h"

#define FE_WORLD_MATRIX_MACRO "@WorldMatrix@"
#define FE_VIEW_MATRIX_MACRO "@ViewMatrix@"
#define FE_PROJECTION_MATRIX_MACRO "@ProjectionMatrix@"

enum FEShaderInputDataType
{
	FE_INT_SCALAR_UNIFORM = 0,
	FE_FLOAT_SCALAR_UNIFORM = 1,
	FE_VECTOR2_UNIFORM = 2,
	FE_VECTOR3_UNIFORM = 3,
	FE_VECTOR4_UNIFORM = 4,
	FE_MAT4_UNIFORM = 5
};

namespace FocalEngine
{
	struct FEShaderInputData
	{
		FEShaderInputData();
		FEShaderInputData(FEShaderInputDataType Type, void* RawData, std::string& UniformName);
		~FEShaderInputData();
		FEShaderInputDataType type;
		void* rawData;
		std::string uniformName;
	};

	class FEMaterial;
	class FERenderer;

	class FEShader
	{
		friend FEMaterial;
		friend FERenderer;
	public:
		FEShader(const char* vertexText, const char* fragmentText, std::vector<std::string>& attributes);
		~FEShader();

		virtual void start();
		virtual void stop();

		void bindAttribute(int& attribute, const char* variableName);

		void loadScalar(const char* uniformName, GLfloat& value);
		void loadScalar(const char* uniformName, GLint& value);
		void loadVector(const char* uniformName, glm::vec2& vector);
		void loadVector(const char* uniformName, glm::vec3& vector);
		void loadVector(const char* uniformName, glm::vec4& vector);
		void loadMatrix(const char* uniformName, glm::mat4& matrix);

		std::string parseShaderForMacro(const char* shaderText);

		virtual void loadDataToGPU();
		virtual inline void consumeData(std::vector<FEShaderInputData> Data);
	private:
		GLuint programID;
		GLuint vertexShaderID;
		GLuint fragmentShaderID;
		std::vector<std::string> attributes;

		// standard uniforms
		std::vector<FEShaderInputData> standardDataRequest;
		std::vector<FEShaderInputData> data;

		GLuint loadShader(const char* shaderText, GLuint shaderType);
		void cleanConsumeData();
		void cleanUp();
		void bindAttributes();
		GLuint getUniformLocation(const char* name);
	};
}
