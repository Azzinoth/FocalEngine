#pragma once

#include "FEShaderUniform.h"

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
#define FE_UNIFORM_LIGHTING_MACRO "@UNIFORM_LIGHTING@"
#define FE_CSM_MACRO "@CSM@"

#define FE_DEBUG_MACRO "@DEBUG@("

#define FE_MATERIAL_TEXTURES_MACRO "@MaterialTextures@"
#define FE_TERRAIN_LAYERS_TEXTURES_MACRO "@TerrainLayersTextures@"

#define FE_MAX_TEXTURES_PER_MATERIAL 16
#define FE_MAX_SUBMATERIALS_PER_MATERIAL 2

namespace FocalEngine
{
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

	class FEShader : public FEObject
	{
		friend class FEMaterial;
		friend class FERenderer;
		friend class FEPostProcess;
		friend class FEngine;
		friend class FEResourceManager;
	public:
		FEShader(std::string Name, const char* VertexText, const char* FragmentText,
				 const char* TessControlText = nullptr, const char* TessEvalText = nullptr,
				 const char* GeometryText = nullptr, const char* ComputeText = nullptr, bool TestCompilation = false, int GlslVersion = 450);
		~FEShader();

		FEShader(const FEShader& Shader);
		void operator= (const FEShader& Shader);

		void Start();
		void Stop();

		void LoadUniformsDataToGPU();
		bool LoadUniformDataToGPU(std::string UniformName);
		
		std::vector<std::string> GetUniformNameList();
		std::vector<std::string> GetTextureList();

		bool GetUniformData(std::string UniformName, FEShaderUniformValue& ReturnedValue);
		template<typename T>
		bool UpdateUniformData(std::string UniformName, const T& Data);
		
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

		// TODO: It should not be public.
		void AddUniformsFromShader(FEShader* Shader);
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

		std::unordered_map<std::string, FEShaderUniform> Uniforms;
		FEShaderUniform* GetUniform(std::string Name);

		std::unordered_map<int, GLuint> BlockUniforms;

		GLuint LoadShader(const char* ShaderText, GLuint ShaderType);
		bool LinkProgram();
		void CleanUp();
		void BindAttributes();

		void AddUniformInternal(FEShaderUniform NewUniform);
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