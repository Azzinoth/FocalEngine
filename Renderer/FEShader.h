#pragma once

#include "FETexture.h"

#define FE_VERTEX_ATTRIBUTE_POSITION "@In_Position@"
#define FE_VERTEX_ATTRIBUTE_COLOR "@In_Color@"
#define FE_VERTEX_ATTRIBUTE_NORMAL "@In_Normal@"
#define FE_VERTEX_ATTRIBUTE_TANGENT "@In_Tangent@"
#define FE_VERTEX_ATTRIBUTE_UV "@In_UV@"
#define FE_VERTEX_ATTRIBUTE_MATINDEX "@In_Material_Index@"
#define FE_VERTEX_ATTRIBUTE_INSTANCEDATA "@In_Instance_Data@"

#define FE_WORLD_MATRIX_MACRO "@WorldMatrix@"
#define FE_VIEW_MATRIX_MACRO "@ViewMatrix@"
#define FE_PROJECTION_MATRIX_MACRO "@ProjectionMatrix@"
#define FE_PVM_MATRIX_MACRO "@PVMMatrix@"
#define FE_CAMERA_POSITION_MACRO "@CameraPosition@"

#define FE_LIGHT_POSITION_MACRO "@LightPosition@"
#define FE_LIGHT_COLOR_MACRO "@LightColor@"

#define FE_TEXTURE_MACRO "@Texture@"
#define FE_RECEVESHADOWS_MACRO "@RECEVESHADOWS@"
#define FE_CSM_MACRO "@CSM@"

#define FE_DEBUG_MACRO "@DEBUG@("

#define FE_MATERIAL_TEXTURES_MACRO "@MaterialTextures@"

namespace FocalEngine
{
	enum FEShaderParamType
	{
		FE_INT_SCALAR_UNIFORM = 0,
		FE_FLOAT_SCALAR_UNIFORM = 1,
		FE_VECTOR2_UNIFORM = 2,
		FE_VECTOR3_UNIFORM = 3,
		FE_VECTOR4_UNIFORM = 4,
		FE_MAT4_UNIFORM = 5,
		FE_NULL_UNIFORM = 6
	};

	enum FEVertexAttributes
	{
		FE_POSITION = 1 << 0,
		FE_COLOR = 1 << 1,
		FE_NORMAL = 1 << 2,
		FE_TANGENTS = 1 << 3,
		FE_UV = 1 << 4,
		FE_INDEX = 1 << 5,
		FE_MATINDEX = 1 << 6,
		FE_INSTANCEDATA = 1 << 7
	};

	const int FE_MAX_TEXTURES_PER_MATERIAL = 16;
	const int FE_MAX_SUBMATERIALS_PER_MATERIAL = 2;

	struct FEShaderParam
	{
		FEShaderParam();
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

		void updateData(void* Data);
		void updateData(int Data);
		void updateData(float Data);
		void updateData(glm::vec2 Data);
		void updateData(glm::vec3 Data);
		void updateData(glm::vec4 Data);
		void updateData(glm::mat4 Data);

		int nameHash = 0;
		std::string getName();
		void setName(std::string newName);

		void* data;
		FEShaderParamType type;
		std::string name;
	};

	class FEMaterial;
	class FERenderer;
	class FEPostProcess;
	class FEngine;

	class FEShader : public FEObject
	{
		friend FEMaterial;
		friend FERenderer;
		friend FEPostProcess;
		friend FEngine;
	public:
		FEShader(std::string name, const char* vertexText, const char* fragmentText,
			const char* tessControlText = nullptr, const char* tessEvalText = nullptr,
			const char* geometryText = nullptr, const char* computeText = nullptr, bool testCompilation = false, int glslVersion = 450);
		~FEShader();

		FEShader(const FEShader& shader);
		void operator= (const FEShader& shader);

		virtual void start();
		virtual void stop();

		void loadScalar(int& uniformNameHash, GLfloat& value);
		void loadScalar(int& uniformNameHash, GLint& value);
		void loadVector(int& uniformNameHash, glm::vec2& vector);
		void loadVector(int& uniformNameHash, glm::vec3& vector);
		void loadVector(int& uniformNameHash, glm::vec4& vector);
		void loadMatrix(int& uniformNameHash, glm::mat4& matrix);
		void loadIntArray(int& uniformNameHash, GLint* array, size_t arraySize);
		void loadIntArray(GLuint uniformLocation, GLint* array, size_t arraySize);
		void loadFloatArray(int& uniformNameHash, GLfloat* array, size_t arraySize);

		virtual void loadDataToGPU();
		virtual void addParameter(FEShaderParam Parameter);

		std::vector<std::string> getParameterList();
		std::vector<std::string> getTextureList();
		FEShaderParam* getParameter(std::string name);

		char* getVertexShaderText();
		char* getTessControlShaderText();
		char* getTessEvalShaderText();
		char* getGeometryShaderText();
		char* getFragmentShaderText();
		char* getComputeShaderText();

		std::string getCompilationErrors();

		bool isDebugRequest();
		std::vector<std::vector<float>>* getDebugData();
		std::vector<std::string> getDebugVariables();

		void dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	private:
		void copyCode(const FEShader& shader);

		std::string compilationErrors;

		GLuint programID;
		GLuint vertexShaderID;
		char* vertexShaderText = nullptr;
		GLuint tessControlShaderID;
		char* tessControlShaderText = nullptr;
		GLuint tessEvalShaderID;
		char* tessEvalShaderText = nullptr;
		GLuint geometryShaderID;
		char* geometryShaderText = nullptr;
		GLuint fragmentShaderID;
		char* fragmentShaderText = nullptr;
		GLuint computeShaderID;
		char* computeShaderText = nullptr;

		int vertexAttributes = 0;

		std::unordered_map<std::string, FEShaderParam> parameters;
		std::unordered_map<int, GLuint> blockUniforms;

		GLuint loadShader(const char* shaderText, GLuint shaderType);
		void cleanUp();
		void bindAttributes();
		std::unordered_map<int, GLuint> uniformLocations;
		GLuint getUniformLocation(int& uniformNameHash);
		std::vector<std::string> textureUniforms;

		std::string parseShaderForMacro(const char* shaderText);
		void registerUniforms();

		bool CSM = false;
		bool testCompilationMode = false;
		bool materialTexturesList = false;
		int glslVersion;

#ifdef FE_DEBUG_ENABLED
		bool debugRequest = false;
		GLuint SSBO = -1;
		GLint SSBOBinding;
		unsigned int SSBOSize = 1024 * 1024 * sizeof(float);
		inline void createSSBO();
		int thisFrameDebugBind = 0;
		std::vector<std::string> debugVariables;
		std::vector<std::vector<float>> debugData;
#endif
	};
}
