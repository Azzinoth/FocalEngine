#pragma once

#include "FETexture.h"

#define FE_VERTEX_ATTRIBUTE_POSITION "@In_Position@"
#define FE_VERTEX_ATTRIBUTE_COLOR "@In_Color@"
#define FE_VERTEX_ATTRIBUTE_NORMAL "@In_Normal@"
#define FE_VERTEX_ATTRIBUTE_TANGENT "@In_Tangent@"
#define FE_VERTEX_ATTRIBUTE_UV "@In_UV@"

#define FE_WORLD_MATRIX_MACRO "@WorldMatrix@"
#define FE_VIEW_MATRIX_MACRO "@ViewMatrix@"
#define FE_PROJECTION_MATRIX_MACRO "@ProjectionMatrix@"
#define FE_CAMERA_POSITION_MACRO "@CameraPosition@"

#define FE_LIGHT_POSITION_MACRO "@LightPosition@"
#define FE_LIGHT_COLOR_MACRO "@LightColor@"

#define FE_TEXTURE_MACRO "@Texture@"

namespace FocalEngine
{
	enum FEShaderParamType
	{
		FE_INT_SCALAR_UNIFORM = 0,
		FE_FLOAT_SCALAR_UNIFORM = 1,
		FE_VECTOR2_UNIFORM = 2,
		FE_VECTOR3_UNIFORM = 3,
		FE_VECTOR4_UNIFORM = 4,
		FE_MAT4_UNIFORM = 5
	};

	enum FEVertexAttributes
	{
		FE_POSITION = 1,
		FE_COLOR = 2,
		FE_NORMAL = 4,
		FE_TANGENTS = 8,
		FE_UV = 16,
		FE_INDEX = 32,
	};

	struct FEShaderParam
	{
		FEShaderParam() {};
		FEShaderParam(int Data, std::string Name);
		FEShaderParam(float Data, std::string Name);
		FEShaderParam(glm::vec2 Data, std::string Name);
		FEShaderParam(glm::vec3 Data, std::string Name);
		FEShaderParam(glm::vec4 Data, std::string Name);
		FEShaderParam(glm::mat4 Data, std::string Name);

		void copyCode(const FEShaderParam& copy);
		FEShaderParam(const FEShaderParam& copy);
		void operator=(const FEShaderParam& assign);

		~FEShaderParam();

		void updateData(int Data);
		void updateData(float Data);
		void updateData(glm::vec2 Data);
		void updateData(glm::vec3 Data);
		void updateData(glm::vec4 Data);
		void updateData(glm::mat4 Data);

		std::string getParamName();
		void setParamName(std::string newName);

		void* data;
		FEShaderParamType type;
		std::string paramName;
	};

	class FEMaterial;
	class FERenderer;
	class FEPostProcess;

	class FEShader
	{
		friend FEMaterial;
		friend FERenderer;
		friend FEPostProcess;
	public:
		FEShader(const char* vertexText, const char* fragmentText);
		~FEShader();

		virtual void start();
		virtual void stop();

		void loadScalar(const char* uniformName, GLfloat& value);
		void loadScalar(const char* uniformName, GLint& value);
		void loadVector(const char* uniformName, glm::vec2& vector);
		void loadVector(const char* uniformName, glm::vec3& vector);
		void loadVector(const char* uniformName, glm::vec4& vector);
		void loadMatrix(const char* uniformName, glm::mat4& matrix);

		std::string parseShaderForMacro(const char* shaderText);
		void registerUniforms();

		virtual void loadDataToGPU();
		virtual void addParams(std::vector<FEShaderParam> Params);
		virtual void addParams(FEShaderParam Param);

		virtual FEShaderParam& getParam(std::string name);
	private:
		GLuint programID;
		GLuint vertexShaderID;
		GLuint fragmentShaderID;
		int vertexAttributes = 0;

		std::vector<FEShaderParam> params;

		GLuint loadShader(const char* shaderText, GLuint shaderType);
		void cleanUp();
		void bindAttributes();
		GLuint getUniformLocation(const char* name);
		std::vector<std::string> textureUniforms;
	};
}
