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
#define FE_TERRAIN_LAYERS_TEXTURES_MACRO "@TerrainLayersTextures@"

#define FE_MAX_TEXTURES_PER_MATERIAL 16
#define FE_MAX_SUBMATERIALS_PER_MATERIAL 2

namespace FocalEngine
{
	enum FE_SHADER_PARAM_TYPE
	{
		FE_INT_SCALAR_UNIFORM = 0,
		FE_FLOAT_SCALAR_UNIFORM = 1,
		FE_VECTOR2_UNIFORM = 2,
		FE_VECTOR3_UNIFORM = 3,
		FE_VECTOR4_UNIFORM = 4,
		FE_MAT4_UNIFORM = 5,
		FE_NULL_UNIFORM = 6,
		FE_BOOL_UNIFORM = 7
	};

	enum FE_VERTEX_ATTRIBUTES
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

	struct FEShaderParam
	{
		FEShaderParam();
		FEShaderParam(bool Data, std::string Name);
		FEShaderParam(int Data, std::string Name);
		FEShaderParam(float Data, std::string Name);
		FEShaderParam(glm::vec2 Data, std::string Name);
		FEShaderParam(glm::vec3 Data, std::string Name);
		FEShaderParam(glm::vec4 Data, std::string Name);
		FEShaderParam(glm::mat4 Data, std::string Name);

		void CopyCode(const FEShaderParam& Copy);
		FEShaderParam(const FEShaderParam& Copy);
		void operator=(const FEShaderParam& Assign);

		~FEShaderParam();

		int NameHash = 0;
		std::string GetName();
		void SetName(std::string NewName);

		void* Data;
		FE_SHADER_PARAM_TYPE Type;
		std::string Name;
	};

	class FEMaterial;
	class FERenderer;
	class FEPostProcess;
	class FEngine;
	class FEResourceManager;

	class FEShader : public FEObject
	{
		friend FEMaterial;
		friend FERenderer;
		friend FEPostProcess;
		friend FEngine;
		friend FEResourceManager;
	public:
		FEShader(std::string Name, const char* VertexText, const char* FragmentText,
			const char* TessControlText = nullptr, const char* TessEvalText = nullptr,
			const char* GeometryText = nullptr, const char* ComputeText = nullptr, bool TestCompilation = false, int GlslVersion = 450);
		~FEShader();

		FEShader(const FEShader& Shader);
		void operator= (const FEShader& Shader);

		virtual void Start();
		virtual void Stop();

		void LoadScalar(const int& UniformNameHash, const GLboolean& Value);
		void LoadScalar(const int& UniformNameHash, const GLfloat& Value);
		void LoadScalar(const int& UniformNameHash, const GLint& Value);
		void LoadVector(const int& UniformNameHash, const glm::vec2& Vector);
		void LoadVector(const int& UniformNameHash, const glm::vec3& Vector);
		void LoadVector(const int& UniformNameHash, const glm::vec4& Vector);
		void LoadMatrix(const int& UniformNameHash, glm::mat4& Matrix);
		void LoadIntArray(const int& UniformNameHash, const GLint* Array, size_t ArraySize);
		void LoadIntArray(GLuint UniformLocation, const GLint* Array, size_t ArraySize);
		void LoadFloatArray(const int& UniformNameHash, const GLfloat* Array, size_t ArraySize);

		virtual void LoadDataToGPU();
		virtual void AddParameter(FEShaderParam Parameter);

		std::vector<std::string> GetParameterList();
		void* GetParameterData(std::string Name);
		std::vector<std::string> GetTextureList();

		template<typename T>
		void UpdateParameterData(std::string Name, const T& Data);

		template<>
		void UpdateParameterData<void*>(std::string Name, void* const& Data);

		template<>
		void UpdateParameterData<bool>(std::string Name, const bool& Data);

		template<>
		void UpdateParameterData<int>(std::string Name, const int& Data);

		template<>
		void UpdateParameterData<float>(std::string Name, const float& Data);

		template<>
		void UpdateParameterData<glm::vec2>(std::string Name, const glm::vec2& Data);

		template<>
		void UpdateParameterData<glm::vec3>(std::string Name, const glm::vec3& Data);

		template<>
		void UpdateParameterData<glm::vec4>(std::string Name, const glm::vec4& Data);

		template<>
		void UpdateParameterData<glm::mat4>(std::string Name, const glm::mat4& Data);
		
		char* GetVertexShaderText();
		char* GetTessControlShaderText();
		char* GetTessEvalShaderText();
		char* GetGeometryShaderText();
		char* GetFragmentShaderText();
		char* GetComputeShaderText();

		std::string GetCompilationErrors();
		std::string GetLinkErrors();
		std::string GetValidateErrors();

		bool IsDebugRequest();
		std::vector<std::vector<float>>* GetDebugData();
		std::vector<std::string> GetDebugVariables();

		void Dispatch(GLuint GroupXCount, GLuint GroupYCount, GLuint GroupZCount);

		void AddParametersFromShader(FEShader* Shader);
	private:
		void CopyCode(const FEShader& Shader);

		std::string CompilationErrors;
		std::string LinkErrors;
		std::string ValidateErrors;

		GLuint ProgramID;
		GLuint VertexShaderID;
		char* VertexShaderText = nullptr;
		GLuint TessControlShaderID;
		char* TessControlShaderText = nullptr;
		GLuint TessEvalShaderID;
		char* TessEvalShaderText = nullptr;
		GLuint GeometryShaderID;
		char* GeometryShaderText = nullptr;
		GLuint FragmentShaderID;
		char* FragmentShaderText = nullptr;
		GLuint ComputeShaderID;
		char* ComputeShaderText = nullptr;

		int VertexAttributes = 0;

		std::unordered_map<std::string, FEShaderParam> Parameters;
		FEShaderParam* GetParameter(std::string Name);
		std::unordered_map<int, GLuint> BlockUniforms;

		GLuint LoadShader(const char* ShaderText, GLuint ShaderType);
		bool LinkProgram();
		void CleanUp();
		void BindAttributes();
		std::unordered_map<int, GLuint> UniformLocations;
		GLuint GetUniformLocation(const int& UniformNameHash);
		std::vector<std::string> TextureUniforms;

		std::string ParseShaderForMacro(const char* ShaderText);
		void RegisterUniforms();

		bool bCSM = false;
		bool bTestCompilationMode = false;
		bool bMaterialTexturesList = false;
		bool bTerrainLayersTexturesList = false;
		int GlslVersion;

#ifdef FE_DEBUG_ENABLED
		bool bDebugRequest = false;
		GLuint SSBO = -1;
		GLint SSBOBinding;
		unsigned int SSBOSize = 1024 * 1024 * sizeof(float);
		inline void CreateSSBO();
		int ThisFrameDebugBind = 0;
		std::vector<std::string> DebugVariables;
		std::vector<std::vector<float>> DebugData;
#endif
		void ReCompile(std::string Name, const char* VertexText, const char* FragmentText,
					   const char* TessControlText = nullptr, const char* TessEvalText = nullptr,
					   const char* GeometryText = nullptr, const char* ComputeText = nullptr, bool TestCompilation = false, int GlslVersion = 450);
	};

#include "FEShader.inl"
}