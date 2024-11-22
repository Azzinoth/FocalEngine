#include "FEShader.h"
using namespace FocalEngine;

FEShader::FEShader(const std::string Name, const char* VertexText, const char* FragmentText,
                   const char* TessControlText, const char* TessEvalText,
                   const char* GeometryText, const char* ComputeText, const bool TestCompilation, const int GlslVersion) : FEObject(FE_SHADER, Name)
{
	this->GlslVersion = GlslVersion;
	bTestCompilationMode = TestCompilation;
	SetName(Name);
	size_t TextLenght = 0;

	if (VertexText != nullptr)
	{
		VertexShaderID = LoadShader(VertexText, GL_VERTEX_SHADER);
		TextLenght = strlen(VertexText);
		VertexShaderText = new char[TextLenght + 1];
		strcpy_s(VertexShaderText, TextLenght + 1, VertexText);
	}

	if (TessControlText != nullptr)
	{
		TessControlShaderID = LoadShader(TessControlText, GL_TESS_CONTROL_SHADER);
		TextLenght = strlen(TessControlText);
		TessControlShaderText = new char[TextLenght + 1];
		strcpy_s(TessControlShaderText, TextLenght + 1, TessControlText);
	}
	
	if (TessEvalText != nullptr)
	{
		TessEvalShaderID = LoadShader(TessEvalText, GL_TESS_EVALUATION_SHADER);
		TextLenght = strlen(TessEvalText);
		TessEvalShaderText = new char[TextLenght + 1];
		strcpy_s(TessEvalShaderText, TextLenght + 1, TessEvalText);
	}

	if (GeometryText != nullptr)
	{
		GeometryShaderID = LoadShader(GeometryText, GL_GEOMETRY_SHADER);
		TextLenght = strlen(GeometryText);
		GeometryShaderText = new char[TextLenght + 1];
		strcpy_s(GeometryShaderText, TextLenght + 1, GeometryText);
	}

	if (FragmentText != nullptr)
	{
		FragmentShaderID = LoadShader(FragmentText, GL_FRAGMENT_SHADER);
		TextLenght = strlen(FragmentText);
		FragmentShaderText = new char[TextLenght + 1];
		strcpy_s(FragmentShaderText, TextLenght + 1, FragmentText);
	}

	if (ComputeText != nullptr)
	{
		bTestCompilationMode = TestCompilation;
		ComputeShaderID = LoadShader(ComputeText, GL_COMPUTE_SHADER);
		TextLenght = strlen(ComputeText);
		ComputeShaderText = new char[TextLenght + 1];
		strcpy_s(ComputeShaderText, TextLenght + 1, ComputeText);
	}

	if (bTestCompilationMode && !CompilationErrors.empty() && !LinkErrors.empty() && !ValidateErrors.empty())
		return;

	FE_GL_ERROR(ProgramID = glCreateProgram());

	if (VertexText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, VertexShaderID));
	if (TessControlText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, TessControlShaderID));
	if (TessEvalText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, TessEvalShaderID));
	if (GeometryText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, GeometryShaderID));
	if (FragmentText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, FragmentShaderID));
	if (ComputeText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, ComputeShaderID));

	BindAttributes();
	LinkProgram();

	if (VertexText != nullptr)
		FE_GL_ERROR(glDeleteShader(VertexShaderID));
	if (TessControlText != nullptr)
		FE_GL_ERROR(glDeleteShader(TessControlShaderID));
	if (TessEvalText != nullptr)
		FE_GL_ERROR(glDeleteShader(TessEvalShaderID));
	if (GeometryText != nullptr)
		FE_GL_ERROR(glDeleteShader(GeometryShaderID));
	if (FragmentText != nullptr)
		FE_GL_ERROR(glDeleteShader(FragmentShaderID));
	if (ComputeText != nullptr)
		FE_GL_ERROR(glDeleteShader(ComputeShaderID));

	RegisterUniforms();

#ifdef FE_DEBUG_ENABLED
	CreateSSBO();
#endif
}

void FEShader::CopyCode(const FEShader& Shader)
{
	Name = Shader.Name;
	NameHash = Shader.NameHash;
	CompilationErrors = Shader.CompilationErrors;
	LinkErrors = Shader.LinkErrors;
	ValidateErrors = Shader.ValidateErrors;

	ProgramID = Shader.ProgramID;
	VertexShaderID = Shader.VertexShaderID;
	VertexShaderText = new char[strlen(Shader.VertexShaderText) + 1];
	strcpy_s(VertexShaderText, strlen(Shader.VertexShaderText) + 1, Shader.VertexShaderText);

	TessControlShaderID = Shader.TessControlShaderID;
	if (Shader.TessControlShaderText != nullptr)
	{
		TessControlShaderText = new char[strlen(Shader.TessControlShaderText) + 1];
		strcpy_s(TessControlShaderText, strlen(Shader.TessControlShaderText) + 1, Shader.TessControlShaderText);
	}

	TessEvalShaderID = Shader.TessEvalShaderID;
	if (Shader.TessEvalShaderText != nullptr)
	{
		TessEvalShaderText = new char[strlen(Shader.TessEvalShaderText) + 1];
		strcpy_s(TessEvalShaderText, strlen(Shader.TessEvalShaderText) + 1, Shader.TessEvalShaderText);
	}

	GeometryShaderID = Shader.GeometryShaderID;
	if (Shader.GeometryShaderText != nullptr)
	{
		GeometryShaderText = new char[strlen(Shader.GeometryShaderText) + 1];
		strcpy_s(GeometryShaderText, strlen(Shader.GeometryShaderText) + 1, Shader.GeometryShaderText);
	}

	FragmentShaderID = Shader.FragmentShaderID;
	if (Shader.FragmentShaderText != nullptr)
	{
		FragmentShaderText = new char[strlen(Shader.FragmentShaderText) + 1];
		strcpy_s(FragmentShaderText, strlen(Shader.FragmentShaderText) + 1, Shader.FragmentShaderText);
	}

	ComputeShaderID = Shader.ComputeShaderID;
	if (Shader.ComputeShaderText != nullptr)
	{
		ComputeShaderText = new char[strlen(Shader.ComputeShaderText) + 1];
		strcpy_s(ComputeShaderText, strlen(Shader.ComputeShaderText) + 1, Shader.ComputeShaderText);
	}

	VertexAttributes = Shader.VertexAttributes;

	Uniforms = Shader.Uniforms;
	BlockUniforms = Shader.BlockUniforms;
	TextureUniforms = Shader.TextureUniforms;

	bCSM = Shader.bCSM;
	bTestCompilationMode = Shader.bTestCompilationMode;

#ifdef FE_DEBUG_ENABLED
	bDebugRequest = Shader.bDebugRequest;
	CreateSSBO();
	DebugVariables = Shader.DebugVariables;
	DebugData = Shader.DebugData;
#endif
}

FEShader::FEShader(const FEShader& Shader) : FEObject(FE_SHADER, Shader.Name)
{
	CopyCode(Shader);
}

void FEShader::operator=(const FEShader& Shader)
{
	if (&Shader != this)
		this->CleanUp();

	CopyCode(Shader);
}

FEShader::~FEShader()
{
	CleanUp();
}

void FEShader::RegisterUniforms()
{
	GLint UniformCount;
	GLint UniformSize;
	GLenum UniformType;
	GLsizei ActualUniformNameLength;

	FE_GL_ERROR(glGetProgramiv(ProgramID, GL_ACTIVE_UNIFORMS, &UniformCount));
	for (size_t i = 0; i < static_cast<size_t>(UniformCount); i++)
	{
		const GLsizei MaxUniformNameLength = 512;
		GLchar UniformName[MaxUniformNameLength];
		FE_GL_ERROR(glGetActiveUniform(ProgramID, static_cast<GLuint>(i), MaxUniformNameLength, &ActualUniformNameLength, &UniformSize, &UniformType, UniformName));
		
		std::string UniformNameString = UniformName;
		if (UniformNameString.find("debugFlag") != std::string::npos)
		{
			int y = 0;
			y++;
			//bDebugRequest = true;
			//continue;
		}

		bool bIsArray = ((UniformSize > 1) && (UniformNameString.find('[') != std::string::npos));
		std::string UniformArrayName = "";
		if (bIsArray)
			UniformArrayName = UniformNameString.substr(0, UniformNameString.find('['));

		std::vector<GLuint> Locations;
		if (bIsArray)
		{
			for (size_t i = 0; i < UniformSize; i++)
			{
				std::string CurrentVariableName = UniformArrayName + "[" + std::to_string(i) + "]";
				GLuint Location = -1;
				FE_GL_ERROR(Location = glGetUniformLocation(ProgramID, CurrentVariableName.c_str()));
				Locations.push_back(Location);
			}
		}
		else
		{
			GLuint Location = -1;
			FE_GL_ERROR(Location = glGetUniformLocation(ProgramID, UniformName));
			Locations.push_back(Location);
		}
		
		switch (UniformType)
		{
			case GL_BOOL:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_BOOL_ARRAY, std::vector<bool>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_BOOL, false, UniformName, Locations));
				}

				break;
			}

			case GL_UNSIGNED_INT:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT_ARRAY, std::vector<unsigned int>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT, 0, UniformName, Locations));
				}

				break;
			}

			case GL_INT:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_INT_ARRAY, std::vector<int>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_INT, 0, UniformName, Locations));
				}

				break;
			}

			case GL_FLOAT:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_ARRAY, std::vector<float>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT, 0.0f, UniformName, Locations));
				}

				break;
			}

			case GL_FLOAT_VEC2:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2_ARRAY, std::vector<glm::vec2>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2, glm::vec2(0.0f), UniformName, Locations));
				}

				break;
			}

			case GL_FLOAT_VEC3:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3_ARRAY, std::vector<glm::vec3>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3, glm::vec3(0.0f), UniformName, Locations));
				}

				break;
			}

			case GL_FLOAT_VEC4:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4_ARRAY, std::vector<glm::vec4>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4, glm::vec4(0.0f), UniformName, Locations));
				}

				break;
			}

			case GL_FLOAT_MAT4:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_MAT4_ARRAY, std::vector<glm::mat4>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_MAT4, glm::mat4(1.0f), UniformName, Locations));
				}

				break;
			}

			case GL_SAMPLER_1D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D_ARRAY, Locations, UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D, 0, UniformName, Locations));
				}

				break;
			}

			case GL_SAMPLER_2D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D_ARRAY, Locations, UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D, 0, UniformName, Locations));
				}

				break;
			}

			case GL_SAMPLER_3D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D_ARRAY, Locations, UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D, 0, UniformName, Locations));
				}

				break;
			}

			case GL_SAMPLER_CUBE:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE_ARRAY, Locations, UniformArrayName, Locations));
				}
				else
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE, 0, UniformName, Locations));
				}

				break;
			}

			case GL_IMAGE_1D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D_ARRAY, std::vector<unsigned int>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					// TODO: check if this make sense
					GLint BindingPoint;
					FE_GL_ERROR(glGetUniformiv(ProgramID, Locations[0], &BindingPoint));
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D, BindingPoint, UniformName, Locations));
				}

				break;
			}

			case GL_IMAGE_2D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D_ARRAY, std::vector<unsigned int>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					// TODO: check if this make sense
					GLint BindingPoint;
					FE_GL_ERROR(glGetUniformiv(ProgramID, Locations[0], &BindingPoint));
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D, BindingPoint, UniformName, Locations));
				}

				break;
			}

			case GL_IMAGE_3D:
			{
				if (bIsArray)
				{
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D_ARRAY, std::vector<unsigned int>(UniformSize), UniformArrayName, Locations));
				}
				else
				{
					// TODO: check if this make sense
					GLint BindingPoint;
					FE_GL_ERROR(glGetUniformiv(ProgramID, Locations[0], &BindingPoint));
					AddUniformInternal(FEShaderUniform(FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D, BindingPoint, UniformName, Locations));
				}

				break;
			}

			default:
			{
				LOG.Add("FEShader::RegisterUniforms() unknown uniform type: " + std::to_string(UniformType), "FE_LOG_RENDERING", FE_LOG_ERROR);
				break;
			}	
		}
	}
	
	GLuint UniformBlockIndex = -1;
	FE_GL_ERROR(UniformBlockIndex = glGetUniformBlockIndex(ProgramID, "lightInfo"));
	if (UniformBlockIndex != GL_INVALID_INDEX)
	{
		FE_GL_ERROR(glUniformBlockBinding(ProgramID, UniformBlockIndex, 0));
		BlockUniforms[static_cast<int>(std::hash<std::string>{}("lightInfo"))] = GL_INVALID_INDEX;
	}

	UniformBlockIndex = -1;
	FE_GL_ERROR(UniformBlockIndex = glGetUniformBlockIndex(ProgramID, "directionalLightInfo"));
	if (UniformBlockIndex != GL_INVALID_INDEX)
	{
		FE_GL_ERROR(glUniformBlockBinding(ProgramID, UniformBlockIndex, 1));
		BlockUniforms[static_cast<int>(std::hash<std::string>{}("directionalLightInfo"))] = GL_INVALID_INDEX;
	}

	Start();

	if (bMaterialTexturesList)
	{
		for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
		{
			FE_GL_ERROR(glUniform1i(GetUniform("textures")->Locations[i], static_cast<int>(i)));
		}

		// 16 textures for material + 4 CSM textures at the end. Next available binding is 20. Max is 27.
		for (size_t i = 20; i < 20 + TextureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, TextureUniforms[i - 20].c_str()), static_cast<int>(i)));
		}
	}
	else if (bTerrainLayersTexturesList)
	{
		for (size_t i = 0; i < 24; i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, std::string("textures[" + std::to_string(i) + "]").c_str()), static_cast<int>(i)));
		}

		// 24 textures for terrain layers + 4 CSM textures at the end. next available binding is 24. Max is 27.
		for (size_t i = 24; i < 24 + TextureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, TextureUniforms[i - 24].c_str()), static_cast<int>(i)));
		}
	}
	else
	{
		for (size_t i = 0; i < TextureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, TextureUniforms[i].c_str()), static_cast<int>(i)));
		}
	}

	if (bCSM)
	{
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, "CSM0"), FE_CSM_UNIT));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, "CSM1"), FE_CSM_UNIT + 1));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, "CSM2"), FE_CSM_UNIT + 2));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(ProgramID, "CSM3"), FE_CSM_UNIT + 3));
	}

	Stop();
}

GLuint FEShader::LoadShader(const char* ShaderText, const GLuint ShaderType)
{
	GLuint ShaderID;
	FE_GL_ERROR(ShaderID = glCreateShader(ShaderType));

	const std::string TempString = ParseShaderForMacro(ShaderText);
	const char *ParsedShaderText = TempString.c_str();
	FE_GL_ERROR(glShaderSource(ShaderID, 1, &ParsedShaderText, nullptr));
	FE_GL_ERROR(glCompileShader(ShaderID));
	GLint Status = 0;
	FE_GL_ERROR(glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Status));

	if (Status == GL_FALSE)
	{
		GLint LogSize = 0;
		FE_GL_ERROR(glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &LogSize));
		std::vector<GLchar> ErrorLog(LogSize);

		FE_GL_ERROR(glGetShaderInfoLog(ShaderID, LogSize, &LogSize, &ErrorLog[0]));
		for (size_t i = 0; i < ErrorLog.size(); i++)
		{
			CompilationErrors.push_back(ErrorLog[i]);
		}
		if (!bTestCompilationMode)
			assert(Status);
	}

	return ShaderID;
}

bool FEShader::LinkProgram()
{
	FE_GL_ERROR(glLinkProgram(ProgramID));

	GLint Status = 0;
	FE_GL_ERROR(glGetProgramiv(ProgramID, GL_LINK_STATUS, &Status));
	if (Status == GL_FALSE)
	{
		GLint LogSize = 0;
		FE_GL_ERROR(glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &LogSize));
		if (LogSize > 0)
		{
			std::vector<GLchar> ErrorLog(LogSize);

			FE_GL_ERROR(glGetProgramInfoLog(ProgramID, LogSize, &LogSize, &ErrorLog[0]));
			for (size_t i = 0; i < ErrorLog.size(); i++)
			{
				LinkErrors.push_back(ErrorLog[i]);
			}
		}

		if (!bTestCompilationMode)
			assert(Status);

		return false;
	}

	FE_GL_ERROR(glValidateProgram(ProgramID));

	Status = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Status);
	if (Status == GL_FALSE)
	{
		GLint LogSize = 0;
		FE_GL_ERROR(glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &LogSize));
		if (LogSize > 0)
		{
			std::vector<GLchar> ErrorLog(LogSize);

			FE_GL_ERROR(glGetProgramInfoLog(ProgramID, LogSize, &LogSize, &ErrorLog[0]));
			for (size_t i = 0; i < ErrorLog.size(); i++)
			{
				ValidateErrors.push_back(ErrorLog[i]);
			}
		}

		if (!bTestCompilationMode)
			assert(Status);

		return false;
	}

	return true;
}

void FEShader::CleanUp()
{
	Stop();
	delete[] VertexShaderText;
	VertexShaderText = nullptr;
	delete[] TessControlShaderText;
	TessControlShaderText = nullptr;
	delete[] TessEvalShaderText;
	TessEvalShaderText = nullptr;
	delete[] GeometryShaderText;
	GeometryShaderText = nullptr;
	delete[] FragmentShaderText;
	FragmentShaderText = nullptr;
	delete[] ComputeShaderText;
	ComputeShaderText = nullptr;

	Uniforms.clear();
#ifdef FE_DEBUG_ENABLED
	if (bDebugRequest)
		FE_GL_ERROR(glDeleteBuffers(1, &SSBO));

	DebugData.clear();
	DebugVariables.clear();
	bDebugRequest = false;
#endif
	FE_GL_ERROR(glDeleteProgram(ProgramID));
}

void FEShader::BindAttributes()
{
	if ((VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glBindAttribLocation(ProgramID, 0, "FEPosition"));
	if ((VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glBindAttribLocation(ProgramID, 1, "FEColor"));
	if ((VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glBindAttribLocation(ProgramID, 2, "FENormal"));
	if ((VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glBindAttribLocation(ProgramID, 3, "FETangent"));
	if ((VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glBindAttribLocation(ProgramID, 4, "FETexCoord"));
	if ((VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glBindAttribLocation(ProgramID, 5, "FEMatIndex"));
	if ((VertexAttributes & FE_INSTANCEDATA) == FE_INSTANCEDATA) FE_GL_ERROR(glBindAttribLocation(ProgramID, 6, "FEInstanceData"));
}

void FEShader::Start()
{
	FE_GL_ERROR(glUseProgram(ProgramID));

#ifdef FE_DEBUG_ENABLED
		if (SSBO == static_cast<GLuint>(-1))
			return;

	const unsigned BeginIterator = 0u;
	FE_GL_ERROR(glNamedBufferSubData(SSBO, 0, sizeof(unsigned), &BeginIterator));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOBinding, SSBO));
#endif
}

void FEShader::Stop()
{
#ifdef FE_DEBUG_ENABLED
	if (SSBO == static_cast<GLuint>(-1))
		return;

	if (this->ComputeShaderText != nullptr)
		FE_GL_ERROR(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

	unsigned DebugSize = 0, BufferSize;
	FE_GL_ERROR(glGetNamedBufferSubData(SSBO, 0, sizeof(unsigned), &DebugSize));
	FE_GL_ERROR(glGetNamedBufferParameteriv(SSBO, GL_BUFFER_SIZE, (GLint*)&BufferSize));
	BufferSize /= sizeof(float);

	if (DebugSize > BufferSize)
		DebugSize = BufferSize;

	if (DebugData.size() <= static_cast<size_t>(ThisFrameDebugBind))
		DebugData.push_back(std::vector<float>());

	if (ThisFrameDebugBind >= static_cast<int>(DebugData.size()))
		ThisFrameDebugBind = static_cast<int>(DebugData.size() - 1);

	if (DebugData[ThisFrameDebugBind].size() != DebugSize + 1)
		DebugData[ThisFrameDebugBind].resize(DebugSize + 1);

	DebugData[ThisFrameDebugBind][0] = static_cast<float>(DebugSize);
	FE_GL_ERROR(glGetNamedBufferSubData(SSBO, sizeof(float), static_cast<GLsizei>((DebugData[ThisFrameDebugBind].size() - 1) * sizeof(float)), DebugData[ThisFrameDebugBind].data() + 1));

	ThisFrameDebugBind++;
#endif
	//FE_GL_ERROR(glUseProgram(0));
}

std::string FEShader::ParseShaderForMacro(const char* ShaderText)
{
	size_t Index = -1;
	std::string ParsedShaderText = ShaderText;

	const std::string GlslVersionText = "#version " + std::to_string(GlslVersion) + " core\n";
	ParsedShaderText.insert(0, GlslVersionText);

	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_POSITION);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_POSITION), "layout (location = 0) in vec3 FEPosition;");
		VertexAttributes |= FE_POSITION;
	}
	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_COLOR);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_COLOR), "layout (location = 1) in vec3 FEColor;");
		VertexAttributes |= FE_COLOR;
	}
	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_NORMAL);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_NORMAL), "layout (location = 2) in vec3 FENormal;");
		VertexAttributes |= FE_NORMAL;
	}
	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_TANGENT);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_TANGENT), "layout (location = 3) in vec3 FETangent;");
		VertexAttributes |= FE_TANGENTS;
	}
	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_UV);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_UV), "layout (location = 4) in vec2 FETexCoord;");
		VertexAttributes |= FE_UV;
	}
	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_MATINDEX);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_MATINDEX), "layout (location = 5) in float FEMatIndex;");
		VertexAttributes |= FE_MATINDEX;
	}

	Index = ParsedShaderText.find(FE_VERTEX_ATTRIBUTE_INSTANCEDATA);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VERTEX_ATTRIBUTE_INSTANCEDATA), "layout (location = 6) in mat4 FEInstanceData;");
		VertexAttributes |= FE_INSTANCEDATA;
	}

	Index = ParsedShaderText.find(FE_WORLD_MATRIX_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_WORLD_MATRIX_MACRO), "uniform mat4 FEWorldMatrix;");
	}

	Index = ParsedShaderText.find(FE_VIEW_MATRIX_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_VIEW_MATRIX_MACRO), "uniform mat4 FEViewMatrix;");
	}

	Index = ParsedShaderText.find(FE_PROJECTION_MATRIX_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_PROJECTION_MATRIX_MACRO), "uniform mat4 FEProjectionMatrix;");
	}

	Index = ParsedShaderText.find(FE_PVM_MATRIX_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_PVM_MATRIX_MACRO), "uniform mat4 FEPVMMatrix;");
	}

	Index = ParsedShaderText.find(FE_CAMERA_POSITION_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_CAMERA_POSITION_MACRO), "uniform vec3 FECameraPosition;");
	}

	Index = ParsedShaderText.find(FE_LIGHT_POSITION_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_LIGHT_POSITION_MACRO), "uniform vec3 FELightPosition;");
	}

	Index = ParsedShaderText.find(FE_LIGHT_COLOR_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_LIGHT_COLOR_MACRO), "uniform vec3 FELightColor;");
	}

	Index = ParsedShaderText.find(FE_TEXTURE_MACRO);
	while (Index != std::string::npos)
	{
		const size_t SemicolonPos = ParsedShaderText.find(";", Index);
		std::string TextureName = ParsedShaderText.substr(Index + strlen(FE_TEXTURE_MACRO) + 1, SemicolonPos - (Index + strlen(FE_TEXTURE_MACRO)) - 1);

		ParsedShaderText.replace(Index, strlen(FE_TEXTURE_MACRO), "uniform sampler2D");

		// several shaders could use same texture
		bool WasAlreadyDefined = false;
		for (size_t i = 0; i < TextureUniforms.size(); i++)
		{
			if (TextureName == TextureUniforms[i])
			{
				WasAlreadyDefined = true;
				break;
			}
		}

		if (WasAlreadyDefined)
		{
			Index = ParsedShaderText.find(FE_TEXTURE_MACRO);
			continue;
		}

		// only 16 user textures can be used.
		if (TextureUniforms.size() < 16)
			TextureUniforms.push_back(TextureName);
		Index = ParsedShaderText.find(FE_TEXTURE_MACRO);
	}

	Index = ParsedShaderText.find(FE_CSM_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_CSM_MACRO), "uniform sampler2D CSM0; uniform sampler2D CSM1; uniform sampler2D CSM2; uniform sampler2D CSM3;");
		bCSM = true;
	}

	Index = ParsedShaderText.find(FE_RECEVESHADOWS_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_RECEVESHADOWS_MACRO), "uniform bool FEReceiveShadows;");
	}

	Index = ParsedShaderText.find(FE_MATERIAL_TEXTURES_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_MATERIAL_TEXTURES_MACRO), "uniform int textureBindings[16];\nuniform int textureChannels[16];\nuniform sampler2D textures[16];\n");
		bMaterialTexturesList = true;
	}

	Index = ParsedShaderText.find(FE_TERRAIN_LAYERS_TEXTURES_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_TERRAIN_LAYERS_TEXTURES_MACRO), "uniform sampler2D textures[24];\n");
		bTerrainLayersTexturesList = true;
	}

	Index = ParsedShaderText.find(FE_UNIFORM_LIGHTING_MACRO);
	if (Index != std::string::npos)
	{
		ParsedShaderText.replace(Index, strlen(FE_UNIFORM_LIGHTING_MACRO), "uniform bool FEUniformLighting;");
	}

	// find out if there is any debug requests in shader text.
	int DebugRequestCount = 0;
	int FirstOccurrenceIndex = -1;
	Index = ParsedShaderText.find(FE_DEBUG_MACRO);
	while (Index != std::string::npos)
	{
		int EndIndex = -1;
		std::string VariableName;

		for (size_t i = Index + strlen(FE_DEBUG_MACRO); i < ParsedShaderText.size(); i++)
		{
			if (ParsedShaderText[i] == ')')
			{
				EndIndex = static_cast<int>(i);
				VariableName = ParsedShaderText.substr(Index + strlen(FE_DEBUG_MACRO), EndIndex - (Index + strlen(FE_DEBUG_MACRO)));
				break;
			}
		}
		
		if (EndIndex != -1 && !VariableName.empty())
		{
			ParsedShaderText.erase(Index, EndIndex - Index + 1);
			
			if (DebugRequestCount == 0)
				FirstOccurrenceIndex = static_cast<int>(Index);

#ifdef FE_DEBUG_ENABLED
			DebugVariables.push_back(VariableName);
			// add replacement only in debug mode
			std::string replacement = "debugData[printfIndex++] = ";
			replacement += VariableName;
			replacement += ";";
			ParsedShaderText.insert(ParsedShaderText.begin() + Index, replacement.begin(), replacement.end());
#endif
			
			DebugRequestCount++;
		}
		else
		{
			// we need to delete debug macro anyway
			ParsedShaderText.erase(Index, strlen(FE_DEBUG_MACRO));
		}

		// Find next if it exists
		Index = ParsedShaderText.find(FE_DEBUG_MACRO);
	}

	if (DebugRequestCount != 0)
	{
#ifdef FE_DEBUG_ENABLED
		bDebugRequest = true;

		std::string CounterVariable = "\nuint printfIndex = min(atomicAdd(currentLocation, ";
		CounterVariable += std::to_string(DebugRequestCount);
		CounterVariable += "u), debugData.length() - ";
		CounterVariable += std::to_string(DebugRequestCount);
		CounterVariable += "u);\n";
		ParsedShaderText.insert(ParsedShaderText.begin() + FirstOccurrenceIndex, CounterVariable.begin(), CounterVariable.end());

		size_t version = ParsedShaderText.find("#version");
		size_t extension = ParsedShaderText.rfind("#extension");
		if (extension != std::string::npos)
			version = extension > version ? extension : version;

		size_t LineAfterVersion = 2, BufferInsertOffset = 0;

		if (version != std::string::npos)
		{
			for (size_t i = 0; i < version; ++i)
			{
				if (ParsedShaderText[i] == '\n')
					LineAfterVersion++;
			}

			BufferInsertOffset = version;
			for (size_t i = version; i < ParsedShaderText.length(); ++i)
			{
				BufferInsertOffset += 1;
				if (ParsedShaderText[i] == '\n')
					break;
			}
		}

		ParsedShaderText = ParsedShaderText.substr(0, BufferInsertOffset) + "\nbuffer debugBuffer\n{\nuint currentLocation;\nfloat debugData[];\n};\n#line " + std::to_string(LineAfterVersion) + "\n" + ParsedShaderText.substr(BufferInsertOffset);
#endif //layout (std430)
	}
	
	return ParsedShaderText;
}

void FEShader::LoadUniformsDataToGPU()
{
	auto UniformIterator = Uniforms.begin();
	while (UniformIterator != Uniforms.end())
	{
		UniformIterator->second.LoadUniformToGPU();
		UniformIterator++;
	}
}

bool FEShader::LoadUniformDataToGPU(std::string UniformName)
{
	FEShaderUniform* Uniform = GetUniform(UniformName);
	if (Uniform == nullptr)
		return false;

	Uniform->LoadUniformToGPU();

	return true;
}

void FEShader::AddUniformInternal(FEShaderUniform NewUniform)
{
	Uniforms[NewUniform.GetName()] = NewUniform;
}

std::vector<std::string> FEShader::GetUniformNameList()
{
	FE_MAP_TO_STR_VECTOR(Uniforms)
}

FEShaderUniform* FEShader::GetUniform(const std::string Name)
{
	if (Uniforms.find(Name) == Uniforms.end())
	{
		LOG.Add(std::string("getParameter can't find : ") + Name + " in function FEShader::getParameter", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	return &Uniforms[Name];
}

bool FEShader::GetUniformData(std::string UniformName, FEShaderUniformValue& ReturnedValue)
{
	FEShaderUniform* Uniform = GetUniform(UniformName);
	if (Uniform == nullptr)
		return false;

	ReturnedValue = Uniform->CurrentValue;
	return true;
}

std::vector<std::string> FEShader::GetTextureList()
{
	return TextureUniforms;
}

char* FEShader::GetVertexShaderText()
{
	return VertexShaderText;
}

char* FEShader::GetFragmentShaderText()
{
	return FragmentShaderText;
}

std::string FEShader::GetCompilationErrors()
{
	return CompilationErrors;
}

std::string FEShader::GetLinkErrors()
{
	return LinkErrors;
}

std::string FEShader::GetValidateErrors()
{
	return ValidateErrors;
}

char* FEShader::GetTessControlShaderText()
{
	return TessControlShaderText;
}

char* FEShader::GetTessEvalShaderText()
{
	return TessEvalShaderText;
}

char* FEShader::GetGeometryShaderText()
{
	return GeometryShaderText;
}

char* FEShader::GetComputeShaderText()
{
	return ComputeShaderText;
}

#ifdef FE_DEBUG_ENABLED
inline void FEShader::CreateSSBO()
{
	if (!bDebugRequest)
		return;
	FE_GL_ERROR(glCreateBuffers(1, &SSBO));
	FE_GL_ERROR(glNamedBufferData(SSBO, SSBOSize * 4, nullptr, GL_STREAM_READ));

	SSBOBinding = -1;
	const GLenum Prop = GL_BUFFER_BINDING;
	FE_GL_ERROR(glGetProgramResourceiv(this->ProgramID, GL_SHADER_STORAGE_BLOCK, glGetProgramResourceIndex(this->ProgramID, GL_SHADER_STORAGE_BLOCK, "debugBuffer"), 1, &Prop, sizeof(SSBOBinding), nullptr, &SSBOBinding));

	ThisFrameDebugBind = 0;
}
#endif

bool FEShader::IsDebugRequest()
{
#ifdef FE_DEBUG_ENABLED
	return bDebugRequest;
#endif
	return false;
}

std::vector<std::vector<float>>* FEShader::GetDebugData()
{
#ifdef FE_DEBUG_ENABLED
	return &DebugData;
#endif

	return nullptr;
}

std::vector<std::string> FEShader::GetDebugVariables()
{
#ifdef FE_DEBUG_ENABLED
	return DebugVariables;
#endif

	return std::vector<std::string>();
}

void FEShader::Dispatch(const GLuint GroupXCount, const GLuint GroupYCount, const GLuint GroupZCount)
{
	if (GetComputeShaderText() == nullptr)
		return;

	FE_GL_ERROR(glDispatchCompute(GroupXCount, GroupYCount, GroupZCount));
}

void FEShader::ReCompile(const std::string Name, const char* VertexText, const char* FragmentText,
                         const char* TessControlText, const char* TessEvalText,
                         const char* GeometryText, const char* ComputeText, const bool TestCompilation, const int GlslVersion)
{
	CleanUp();

	this->GlslVersion = GlslVersion;
	bTestCompilationMode = TestCompilation;
	SetName(Name);
	size_t TextLenght;

	if (VertexText != nullptr)
	{
		VertexShaderID = LoadShader(VertexText, GL_VERTEX_SHADER);
		TextLenght = strlen(VertexText);
		VertexShaderText = new char[TextLenght + 1];
		strcpy_s(VertexShaderText, TextLenght + 1, VertexText);
	}

	if (TessControlText != nullptr)
	{
		TessControlShaderID = LoadShader(TessControlText, GL_TESS_CONTROL_SHADER);
		TextLenght = strlen(TessControlText);
		TessControlShaderText = new char[TextLenght + 1];
		strcpy_s(TessControlShaderText, TextLenght + 1, TessControlText);
	}

	if (TessEvalText != nullptr)
	{
		TessEvalShaderID = LoadShader(TessEvalText, GL_TESS_EVALUATION_SHADER);
		TextLenght = strlen(TessEvalText);
		TessEvalShaderText = new char[TextLenght + 1];
		strcpy_s(TessEvalShaderText, TextLenght + 1, TessEvalText);
	}

	if (GeometryText != nullptr)
	{
		GeometryShaderID = LoadShader(GeometryText, GL_GEOMETRY_SHADER);
		TextLenght = strlen(GeometryText);
		GeometryShaderText = new char[TextLenght + 1];
		strcpy_s(GeometryShaderText, TextLenght + 1, GeometryText);
	}

	if (FragmentText != nullptr)
	{
		FragmentShaderID = LoadShader(FragmentText, GL_FRAGMENT_SHADER);
		TextLenght = strlen(FragmentText);
		FragmentShaderText = new char[TextLenght + 1];
		strcpy_s(FragmentShaderText, TextLenght + 1, FragmentText);
	}

	if (ComputeText != nullptr)
	{
		bTestCompilationMode = TestCompilation;
		ComputeShaderID = LoadShader(ComputeText, GL_COMPUTE_SHADER);
		TextLenght = strlen(ComputeText);
		ComputeShaderText = new char[TextLenght + 1];
		strcpy_s(ComputeShaderText, TextLenght + 1, ComputeText);
	}

	if (bTestCompilationMode && !CompilationErrors.empty() && !LinkErrors.empty() && !ValidateErrors.empty())
		return;

	FE_GL_ERROR(ProgramID = glCreateProgram());

	if (VertexText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, VertexShaderID));
	if (TessControlText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, TessControlShaderID));
	if (TessEvalText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, TessEvalShaderID));
	if (GeometryText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, GeometryShaderID));
	if (FragmentText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, FragmentShaderID));
	if (ComputeText != nullptr)
		FE_GL_ERROR(glAttachShader(ProgramID, ComputeShaderID));

	BindAttributes();

	FE_GL_ERROR(glLinkProgram(ProgramID));
	FE_GL_ERROR(glValidateProgram(ProgramID)); // too slow ?

	if (VertexText != nullptr)
		FE_GL_ERROR(glDeleteShader(VertexShaderID));
	if (TessControlText != nullptr)
		FE_GL_ERROR(glDeleteShader(TessControlShaderID));
	if (TessEvalText != nullptr)
		FE_GL_ERROR(glDeleteShader(TessEvalShaderID));
	if (GeometryText != nullptr)
		FE_GL_ERROR(glDeleteShader(GeometryShaderID));
	if (FragmentText != nullptr)
		FE_GL_ERROR(glDeleteShader(FragmentShaderID));
	if (ComputeText != nullptr)
		FE_GL_ERROR(glDeleteShader(ComputeShaderID));

#ifdef FE_DEBUG_ENABLED
	CreateSSBO();
#endif
	RegisterUniforms();
}

void FEShader::AddUniformsFromShader(FEShader* Shader)
{
	if (Shader == nullptr)
		return;

	auto UniformIterator = Shader->Uniforms.begin();
	while (UniformIterator != Shader->Uniforms.end())
	{
		AddUniformInternal(UniformIterator->second);
		UniformIterator++;
	}
}