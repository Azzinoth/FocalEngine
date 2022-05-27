#include "FEShader.h"
using namespace FocalEngine;

FEShaderParam::FEShaderParam()
{
	data = nullptr;
}

std::string FEShaderParam::getName()
{
	return name;
}

void FEShaderParam::setName(std::string newName)
{
	name = newName;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(int Data, std::string Name)
{
	data = new int(Data);
	type = FE_INT_SCALAR_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(float Data, std::string Name)
{
	data = new float(Data);
	type = FE_FLOAT_SCALAR_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(glm::vec2 Data, std::string Name)
{
	data = new glm::vec2(Data);
	type = FE_VECTOR2_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(glm::vec3 Data, std::string Name)
{
	data = new glm::vec3(Data);
	type = FE_VECTOR3_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(glm::vec4 Data, std::string Name)
{
	data = new glm::vec4(Data);
	type = FE_VECTOR4_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

FEShaderParam::FEShaderParam(glm::mat4 Data, std::string Name)
{
	data = new glm::mat4(Data);
	type = FE_MAT4_UNIFORM;
	name = Name;
	nameHash = int(std::hash<std::string>{}(name));
}

void FEShaderParam::updateData(void* Data)
{
	switch (type)
	{
		case FE_INT_SCALAR_UNIFORM:
		{
			*(int*)data = *((int*)Data);
			break;
		}

		case FE_FLOAT_SCALAR_UNIFORM:
		{
			*(float*)data = *((float*)Data);
			break;
		}

		case FE_VECTOR2_UNIFORM:
		{
			*(glm::vec2*)data = *((glm::vec2*)Data);
			break;
		}

		case FE_VECTOR3_UNIFORM:
		{
			*(glm::vec3*)data = *((glm::vec3*)Data);
			break;
		}

		case FE_VECTOR4_UNIFORM:
		{
			*(glm::vec4*)data = *((glm::vec4*)Data);
			break;
		}

		case FE_MAT4_UNIFORM:
		{
			*(glm::mat4*)data = *((glm::mat4*)Data);
			break;
		}

		default:
			break;
	}
}

void FEShaderParam::updateData(int Data)
{
	if (type != FE_INT_SCALAR_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}
		
	*(int*)data = Data;
}

void FEShaderParam::updateData(float Data)
{
	if (type != FE_FLOAT_SCALAR_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}

	*(float*)data = Data;
}

void FEShaderParam::updateData(glm::vec2 Data)
{
	if (type != FE_VECTOR2_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}

	*(glm::vec2*)data = Data;
}

void FEShaderParam::updateData(glm::vec3 Data)
{
	if (type != FE_VECTOR3_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}

	*(glm::vec3*)data = Data;
}

void FEShaderParam::updateData(glm::vec4 Data)
{
	if (type != FE_VECTOR4_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}

	*(glm::vec4*)data = Data;
}

void FEShaderParam::updateData(glm::mat4 Data)
{
	if (type != FE_MAT4_UNIFORM)
	{
		LOG.add(std::string("updateData() incorrect type", FE_LOG_ERROR, FE_LOG_RENDERING));
		return;
	}

	*(glm::mat4*)data = Data;
}

void FEShaderParam::copyCode(const FEShaderParam& copy)
{
	switch (copy.type)
	{
		case FE_INT_SCALAR_UNIFORM:
		{
			data = new int;
			*(int*)data = *((int*)copy.data);
			break;
		}

		case FE_FLOAT_SCALAR_UNIFORM:
		{
			data = new float;
			*(float*)data = *((float*)copy.data);
			break;
		}

		case FE_VECTOR2_UNIFORM:
		{
			data = new glm::vec2;
			*(glm::vec2*)data = *((glm::vec2*)copy.data);
			break;
		}

		case FE_VECTOR3_UNIFORM:
		{
			data = new glm::vec3;
			*(glm::vec3*)data = *((glm::vec3*)copy.data);
			break;
		}

		case FE_VECTOR4_UNIFORM:
		{
			data = new glm::vec4;
			*(glm::vec4*)data = *((glm::vec4*)copy.data);
			break;
		}

		case FE_MAT4_UNIFORM:
		{
			data = new glm::mat4;
			*(glm::mat4*)data = *((glm::mat4*)copy.data);
			break;
		}

		default:
			break;
	}
}

FEShaderParam::FEShaderParam(const FEShaderParam& copy)
{
	this->type = copy.type;
	this->name = copy.name;
	this->nameHash = copy.nameHash;

	copyCode(copy);
}

void FEShaderParam::operator=(const FEShaderParam& assign)
{
	if (&assign != this)
		this->~FEShaderParam();

	this->type = assign.type;
	this->name = assign.name;
	this->nameHash = assign.nameHash;

	copyCode(assign);
}

FEShaderParam::~FEShaderParam()
{
	if (data == nullptr)
		return;

	switch (type)
	{
		case FE_INT_SCALAR_UNIFORM:
		{
			delete (int*)data;
			break;
		}
	
		case FE_FLOAT_SCALAR_UNIFORM:
		{
			delete (float*)data;
			break;
		}
	
		case FE_VECTOR2_UNIFORM:
		{
			delete (glm::vec2*)data;
			break;
		}
	
		case FE_VECTOR3_UNIFORM:
		{
			delete (glm::vec3*)data;
			break;
		}
	
		case FE_VECTOR4_UNIFORM:
		{
			delete (glm::vec4*)data;
			break;
		}
	
		case FE_MAT4_UNIFORM:
		{
			delete (glm::mat4*)data;
			break;
		}
	
		default:
			break;
	}
}

FEShader::FEShader(std::string name, const char* vertexText, const char* fragmentText,
				   const char* tessControlText, const char* tessEvalText,
				   const char* geometryText, const char* computeText, bool testCompilation, int glslVersion) : FEObject(FE_SHADER, name)
{
	this->glslVersion = glslVersion;
	testCompilationMode = testCompilation;
	setName(name);
	size_t textLenght = 0;

	if (vertexText != nullptr)
	{
		vertexShaderID = loadShader(vertexText, GL_VERTEX_SHADER);
		textLenght = strlen(vertexText);
		vertexShaderText = new char[textLenght + 1];
		strcpy_s(vertexShaderText, textLenght + 1, vertexText);
	}

	if (tessControlText != nullptr)
	{
		tessControlShaderID = loadShader(tessControlText, GL_TESS_CONTROL_SHADER);
		size_t textLenght = strlen(tessControlText);
		tessControlShaderText = new char[textLenght + 1];
		strcpy_s(tessControlShaderText, textLenght + 1, tessControlText);
	}
	
	if (tessEvalText != nullptr)
	{
		tessEvalShaderID = loadShader(tessEvalText, GL_TESS_EVALUATION_SHADER);
		size_t textLenght = strlen(tessEvalText);
		tessEvalShaderText = new char[textLenght + 1];
		strcpy_s(tessEvalShaderText, textLenght + 1, tessEvalText);
	}

	if (geometryText != nullptr)
	{
		geometryShaderID = loadShader(geometryText, GL_GEOMETRY_SHADER);
		size_t textLenght = strlen(geometryText);
		geometryShaderText = new char[textLenght + 1];
		strcpy_s(geometryShaderText, textLenght + 1, geometryText);
	}

	if (fragmentText != nullptr)
	{
		fragmentShaderID = loadShader(fragmentText, GL_FRAGMENT_SHADER);
		textLenght = strlen(fragmentText);
		fragmentShaderText = new char[textLenght + 1];
		strcpy_s(fragmentShaderText, textLenght + 1, fragmentText);
	}

	if (computeText != nullptr)
	{
		testCompilationMode = testCompilation;
		computeShaderID = loadShader(computeText, GL_COMPUTE_SHADER);
		size_t textLenght = strlen(computeText);
		computeShaderText = new char[textLenght + 1];
		strcpy_s(computeShaderText, textLenght + 1, computeText);
	}

	if (testCompilationMode && compilationErrors.size() != 0)
		return;

	FE_GL_ERROR(programID = glCreateProgram());

	if (vertexText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, vertexShaderID));
	if (tessControlText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, tessControlShaderID));
	if (tessEvalText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, tessEvalShaderID));
	if (geometryText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, geometryShaderID));
	if (fragmentText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, fragmentShaderID));
	if (computeText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, computeShaderID));

	bindAttributes();

	FE_GL_ERROR(glLinkProgram(programID));
	FE_GL_ERROR(glValidateProgram(programID)); // too slow ?

	if (vertexText != nullptr)
		FE_GL_ERROR(glDeleteShader(vertexShaderID));
	if (tessControlText != nullptr)
		FE_GL_ERROR(glDeleteShader(tessControlShaderID));
	if (tessEvalText != nullptr)
		FE_GL_ERROR(glDeleteShader(tessEvalShaderID));
	if (geometryText != nullptr)
		FE_GL_ERROR(glDeleteShader(geometryShaderID));
	if (fragmentText != nullptr)
		FE_GL_ERROR(glDeleteShader(fragmentShaderID));
	if (computeText != nullptr)
		FE_GL_ERROR(glDeleteShader(computeShaderID));

	registerUniforms();

#ifdef FE_DEBUG_ENABLED
	createSSBO();
#endif
}

void FEShader::copyCode(const FEShader& shader)
{
	name = shader.name;
	nameHash = shader.nameHash;
	compilationErrors = shader.compilationErrors;

	programID = shader.programID;
	vertexShaderID = shader.vertexShaderID;
	vertexShaderText = new char[strlen(shader.vertexShaderText) + 1];
	strcpy_s(vertexShaderText, strlen(shader.vertexShaderText) + 1, shader.vertexShaderText);

	tessControlShaderID = shader.tessControlShaderID;
	if (shader.tessControlShaderText != nullptr)
	{
		tessControlShaderText = new char[strlen(shader.tessControlShaderText) + 1];
		strcpy_s(tessControlShaderText, strlen(shader.tessControlShaderText) + 1, shader.tessControlShaderText);
	}

	tessEvalShaderID = shader.tessEvalShaderID;
	if (shader.tessEvalShaderText != nullptr)
	{
		tessEvalShaderText = new char[strlen(shader.tessEvalShaderText) + 1];
		strcpy_s(tessEvalShaderText, strlen(shader.tessEvalShaderText) + 1, shader.tessEvalShaderText);
	}

	geometryShaderID = shader.geometryShaderID;
	if (shader.geometryShaderText != nullptr)
	{
		geometryShaderText = new char[strlen(shader.geometryShaderText) + 1];
		strcpy_s(geometryShaderText, strlen(shader.geometryShaderText) + 1, shader.geometryShaderText);
	}

	fragmentShaderID = shader.fragmentShaderID;
	if (shader.fragmentShaderText != nullptr)
	{
		fragmentShaderText = new char[strlen(shader.fragmentShaderText) + 1];
		strcpy_s(fragmentShaderText, strlen(shader.fragmentShaderText) + 1, shader.fragmentShaderText);
	}

	computeShaderID = shader.computeShaderID;
	if (shader.computeShaderText != nullptr)
	{
		computeShaderText = new char[strlen(shader.computeShaderText) + 1];
		strcpy_s(computeShaderText, strlen(shader.computeShaderText) + 1, shader.computeShaderText);
	}

	vertexAttributes = shader.vertexAttributes;

	parameters = shader.parameters;
	blockUniforms = shader.blockUniforms;
	uniformLocations = shader.uniformLocations;
	textureUniforms = shader.textureUniforms;

	CSM = shader.CSM;
	testCompilationMode = shader.testCompilationMode;

#ifdef FE_DEBUG_ENABLED
	debugRequest = shader.debugRequest;
	createSSBO();
	debugVariables = shader.debugVariables;
	debugData = shader.debugData;
#endif
}

FEShader::FEShader(const FEShader& shader) : FEObject(FE_SHADER, shader.name)
{
	copyCode(shader);
}

void FEShader::operator=(const FEShader& shader)
{
	if (&shader != this)
		this->cleanUp();

	copyCode(shader);
}

FEShader::~FEShader()
{
	cleanUp();
}

void FEShader::registerUniforms()
{
	GLint count;
	GLint size;
	GLenum type;

	const GLsizei bufSize = 64;
	GLchar name[bufSize];
	GLsizei length;

	FE_GL_ERROR(glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count));
	for (size_t i = 0; i < size_t(count); i++)
	{
		FE_GL_ERROR(glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name));
		// arrays are not currently part of params
		if (std::string(name).find("[") != std::string::npos)
			continue;
		
		switch (type)
		{
			case GL_INT:
			{
				addParameter(FEShaderParam(0, name));
				break;
			}

			case GL_FLOAT:
			{
				addParameter(FEShaderParam(0.0f, name));
				break;
			}

			case GL_FLOAT_VEC2:
			{
				addParameter(FEShaderParam(glm::vec2(0.0f), name));
				break;
			}

			case GL_FLOAT_VEC3:
			{
				addParameter(FEShaderParam(glm::vec3(0.0f), name));
				break;
			}

			case GL_FLOAT_VEC4:
			{
				addParameter(FEShaderParam(glm::vec4(0.0f), name));
				break;
			}

			case GL_FLOAT_MAT4:
			{
				addParameter(FEShaderParam(glm::mat4(1.0f), name));
				break;
			}

			default:
				break;
		}
	}
	
	GLuint uniformBlockIndex = -1;
	FE_GL_ERROR(uniformBlockIndex = glGetUniformBlockIndex(programID, "lightInfo"));
	if (uniformBlockIndex != GL_INVALID_INDEX)
	{
		FE_GL_ERROR(glUniformBlockBinding(programID, uniformBlockIndex, 0));
		blockUniforms[int(std::hash<std::string>{}("lightInfo"))] = GL_INVALID_INDEX;
	}

	uniformBlockIndex = -1;
	FE_GL_ERROR(uniformBlockIndex = glGetUniformBlockIndex(programID, "directionalLightInfo"));
	if (uniformBlockIndex != GL_INVALID_INDEX)
	{
		FE_GL_ERROR(glUniformBlockBinding(programID, uniformBlockIndex, 1));
		blockUniforms[int(std::hash<std::string>{}("directionalLightInfo"))] = GL_INVALID_INDEX;
	}

	start();

	if (materialTexturesList)
	{
		for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
		{
			std::string temp = "textures[" + std::to_string(i) + "]";
			std::string secondTemp = "textureBindings[" + std::to_string(i) + "]";
			std::string thirdTemp = "textureChannels[" + std::to_string(i) + "]";
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, temp.c_str()), int(i)));
			uniformLocations[int(std::hash<std::string>{}(secondTemp))] = glGetUniformLocation(programID, secondTemp.c_str());
			uniformLocations[int(std::hash<std::string>{}(thirdTemp))] = glGetUniformLocation(programID, thirdTemp.c_str());
		}

		// 16 textures for material + 4 CSM textures at the end. Next available binding is 20. Max is 27.
		for (size_t i = 20; i < 20 + textureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, textureUniforms[i - 20].c_str()), int(i)));
		}
	}
	else if (terrainLayersTexturesList)
	{
		for (size_t i = 0; i < 24; i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, std::string("textures[" + std::to_string(i) + "]").c_str()), int(i)));
		}

		// 24 textures for terrain layers + 4 CSM textures at the end. next available binding is 24. Max is 27.
		for (size_t i = 24; i < 24 + textureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, textureUniforms[i - 24].c_str()), int(i)));
		}
	}
	else
	{
		for (size_t i = 0; i < textureUniforms.size(); i++)
		{
			FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, textureUniforms[i].c_str()), int(i)));
		}
	}

	if (CSM)
	{
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, "CSM0"), FE_CSM_UNIT));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, "CSM1"), FE_CSM_UNIT + 1));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, "CSM2"), FE_CSM_UNIT + 2));
		FE_GL_ERROR(glUniform1i(glGetUniformLocation(programID, "CSM3"), FE_CSM_UNIT + 3));
	}
	stop();
}

GLuint FEShader::loadShader(const char* shaderText, GLuint shaderType)
{
	GLuint shaderID;
	FE_GL_ERROR(shaderID = glCreateShader(shaderType));

	std::string tempString = parseShaderForMacro(shaderText);
	const char *parsedShaderText = tempString.c_str();
	FE_GL_ERROR(glShaderSource(shaderID, 1, &parsedShaderText, nullptr));
	FE_GL_ERROR(glCompileShader(shaderID));
	GLint status = 0;
	FE_GL_ERROR(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status));

	if (status == GL_FALSE) {
		GLint logSize = 0;
		FE_GL_ERROR(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize));
		std::vector<GLchar> errorLog(logSize);

		FE_GL_ERROR(glGetShaderInfoLog(shaderID, logSize, &logSize, &errorLog[0]));
		for (size_t i = 0; i < errorLog.size(); i++)
		{
			compilationErrors.push_back(errorLog[i]);
		}
		if (!testCompilationMode)
			assert(status);
	}

	return shaderID;
}

void FEShader::cleanUp()
{
	stop();
	delete[] vertexShaderText;
	vertexShaderText = nullptr;
	delete[] tessControlShaderText;
	tessControlShaderText = nullptr;
	delete[] tessEvalShaderText;
	tessEvalShaderText = nullptr;
	delete[] geometryShaderText;
	geometryShaderText = nullptr;
	delete[] fragmentShaderText;
	fragmentShaderText = nullptr;
	delete[] computeShaderText;
	computeShaderText = nullptr;

	parameters.clear();
#ifdef FE_DEBUG_ENABLED
	if (debugRequest)
		FE_GL_ERROR(glDeleteBuffers(1, &SSBO));

	debugData.clear();
	debugVariables.clear();
	debugRequest = false;
#endif
	FE_GL_ERROR(glDeleteProgram(programID));
}

void FEShader::bindAttributes()
{
	if ((vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glBindAttribLocation(programID, 0, "FEPosition"));
	if ((vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glBindAttribLocation(programID, 1, "FEColor"));
	if ((vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glBindAttribLocation(programID, 2, "FENormal"));
	if ((vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glBindAttribLocation(programID, 3, "FETangent"));
	if ((vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glBindAttribLocation(programID, 4, "FETexCoord"));
	if ((vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glBindAttribLocation(programID, 5, "FEMatIndex"));
	if ((vertexAttributes & FE_INSTANCEDATA) == FE_INSTANCEDATA) FE_GL_ERROR(glBindAttribLocation(programID, 6, "FEInstanceData"));
}

void FEShader::start()
{
	FE_GL_ERROR(glUseProgram(programID));
#ifdef FE_DEBUG_ENABLED
		if (SSBO == GLuint(-1))
			return;

		unsigned beginIterator = 0u;
		FE_GL_ERROR(glNamedBufferSubData(SSBO, 0, sizeof(unsigned), &beginIterator));
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOBinding, SSBO));
#endif
}

void FEShader::stop()
{
#ifdef FE_DEBUG_ENABLED
	if (SSBO == GLuint(-1))
		return;

	unsigned debugSize = 0, bufferSize;
	FE_GL_ERROR(glGetNamedBufferSubData(SSBO, 0, sizeof(unsigned), &debugSize));
	FE_GL_ERROR(glGetNamedBufferParameteriv(SSBO, GL_BUFFER_SIZE, (GLint*)&bufferSize));
	bufferSize /= sizeof(float);

	if (debugSize > bufferSize)
		debugSize = bufferSize;

	if (debugData.size() <= size_t(thisFrameDebugBind))
		debugData.push_back(std::vector<float>());

	if (thisFrameDebugBind >= int(debugData.size()))
		thisFrameDebugBind = int(debugData.size() - 1);

	if (debugData[thisFrameDebugBind].size() != debugSize + 1)
		debugData[thisFrameDebugBind].resize(debugSize + 1);

	debugData[thisFrameDebugBind][0] = float(debugSize);
	FE_GL_ERROR(glGetNamedBufferSubData(SSBO, sizeof(float), GLsizei((debugData[thisFrameDebugBind].size() - 1) * sizeof(float)), debugData[thisFrameDebugBind].data() + 1));

	thisFrameDebugBind++;
#endif
	//FE_GL_ERROR(glUseProgram(0));
}

std::string FEShader::parseShaderForMacro(const char* shaderText)
{
	size_t index = -1;
	std::string parsedShaderText = shaderText;

	std::string glslVersionText = "#version " + std::to_string(glslVersion) + " core\n";
	parsedShaderText.insert(0, glslVersionText);

	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_POSITION);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_POSITION), "layout (location = 0) in vec3 FEPosition;");
		vertexAttributes |= FE_POSITION;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_COLOR);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_COLOR), "layout (location = 1) in vec3 FEColor;");
		vertexAttributes |= FE_COLOR;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_NORMAL);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_NORMAL), "layout (location = 2) in vec3 FENormal;");
		vertexAttributes |= FE_NORMAL;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_TANGENT);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_TANGENT), "layout (location = 3) in vec3 FETangent;");
		vertexAttributes |= FE_TANGENTS;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_UV);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_UV), "layout (location = 4) in vec2 FETexCoord;");
		vertexAttributes |= FE_UV;
	}
	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_MATINDEX);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_MATINDEX), "layout (location = 5) in float FEMatIndex;");
		vertexAttributes |= FE_MATINDEX;
	}

	index = parsedShaderText.find(FE_VERTEX_ATTRIBUTE_INSTANCEDATA);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VERTEX_ATTRIBUTE_INSTANCEDATA), "layout (location = 6) in mat4 FEInstanceData;");
		vertexAttributes |= FE_INSTANCEDATA;
	}

	index = parsedShaderText.find(FE_WORLD_MATRIX_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_WORLD_MATRIX_MACRO), "uniform mat4 FEWorldMatrix;");
	}

	index = parsedShaderText.find(FE_VIEW_MATRIX_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_VIEW_MATRIX_MACRO), "uniform mat4 FEViewMatrix;");
	}

	index = parsedShaderText.find(FE_PROJECTION_MATRIX_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_PROJECTION_MATRIX_MACRO), "uniform mat4 FEProjectionMatrix;");
	}

	index = parsedShaderText.find(FE_PVM_MATRIX_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_PVM_MATRIX_MACRO), "uniform mat4 FEPVMMatrix;");
	}

	index = parsedShaderText.find(FE_CAMERA_POSITION_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_CAMERA_POSITION_MACRO), "uniform vec3 FECameraPosition;");
	}

	index = parsedShaderText.find(FE_LIGHT_POSITION_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_LIGHT_POSITION_MACRO), "uniform vec3 FELightPosition;");
	}

	index = parsedShaderText.find(FE_LIGHT_COLOR_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_LIGHT_COLOR_MACRO), "uniform vec3 FELightColor;");
	}

	index = parsedShaderText.find(FE_TEXTURE_MACRO);
	while (index != std::string::npos)
	{
		size_t semicolonPos = parsedShaderText.find(";", index);
		std::string textureName = parsedShaderText.substr(index + strlen(FE_TEXTURE_MACRO) + 1, semicolonPos - (index + strlen(FE_TEXTURE_MACRO)) - 1);

		parsedShaderText.replace(index, strlen(FE_TEXTURE_MACRO), "uniform sampler2D");

		// several shaders could use same texture
		bool wasAlreadyDefined = false;
		for (size_t i = 0; i < textureUniforms.size(); i++)
		{
			if (textureName == textureUniforms[i])
			{
				wasAlreadyDefined = true;
				break;
			}
		}

		if (wasAlreadyDefined)
		{
			index = parsedShaderText.find(FE_TEXTURE_MACRO);
			continue;
		}

		// only 16 user textures can be used.
		if (textureUniforms.size() < 16)
			textureUniforms.push_back(textureName);
		index = parsedShaderText.find(FE_TEXTURE_MACRO);
	}

	index = parsedShaderText.find(FE_CSM_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_CSM_MACRO), "uniform sampler2D CSM0; uniform sampler2D CSM1; uniform sampler2D CSM2; uniform sampler2D CSM3;");
		CSM = true;
	}

	index = parsedShaderText.find(FE_RECEVESHADOWS_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_RECEVESHADOWS_MACRO), "uniform int FEReceiveShadows;");
	}

	index = parsedShaderText.find(FE_MATERIAL_TEXTURES_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_MATERIAL_TEXTURES_MACRO), "uniform int textureBindings[16];\nuniform int textureChannels[16];\nuniform sampler2D textures[16];\n");
		materialTexturesList = true;
	}

	index = parsedShaderText.find(FE_TERRAIN_LAYERS_TEXTURES_MACRO);
	if (index != std::string::npos)
	{
		parsedShaderText.replace(index, strlen(FE_TERRAIN_LAYERS_TEXTURES_MACRO), "uniform sampler2D textures[24];\n");
		terrainLayersTexturesList = true;
	}

	// find out if there is any debug requests in shader text.
	int debugRequestCount = 0;
	int firstOccurrenceIndex = -1;
	index = parsedShaderText.find(FE_DEBUG_MACRO);
	while (index != std::string::npos)
	{
		int beginIndex = int(index);
		int endIndex = -1;
		std::string variableName = "";

		for (size_t i = index + strlen(FE_DEBUG_MACRO); i < parsedShaderText.size(); i++)
		{
			char text = parsedShaderText[i];
			if (parsedShaderText[i] == ')')
			{
				endIndex = int(i);
				variableName = parsedShaderText.substr(index + strlen(FE_DEBUG_MACRO), endIndex - (index + strlen(FE_DEBUG_MACRO)));
				break;
			}
		}
		
		if (endIndex != -1 && variableName.size() != 0)
		{
			parsedShaderText.erase(index, endIndex - index + 1);
			
			if (debugRequestCount == 0)
				firstOccurrenceIndex = int(index);

#ifdef FE_DEBUG_ENABLED
			debugVariables.push_back(variableName);
			// add replacement only in debug mode
			std::string replacement = "debugData[printfIndex++] = ";
			replacement += variableName;
			replacement += ";";
			parsedShaderText.insert(parsedShaderText.begin() + index, replacement.begin(), replacement.end());
#endif
			
			debugRequestCount++;
		}
		else
		{
			// we need to delete debug macro anyway
			parsedShaderText.erase(index, strlen(FE_DEBUG_MACRO));
		}

		// find next if it is exist
		index = parsedShaderText.find(FE_DEBUG_MACRO);
	}

	if (debugRequestCount != 0)
	{
#ifdef FE_DEBUG_ENABLED
		debugRequest = true;

		std::string counterVariable = "\nuint printfIndex = min(atomicAdd(currentLocation, ";
		counterVariable += std::to_string(debugRequestCount);
		counterVariable += "u), debugData.length() - ";
		counterVariable += std::to_string(debugRequestCount);
		counterVariable += "u);\n";
		parsedShaderText.insert(parsedShaderText.begin() + firstOccurrenceIndex, counterVariable.begin(), counterVariable.end());

		size_t version = parsedShaderText.find("#version");
		size_t extension = parsedShaderText.rfind("#extension");
		if (extension != std::string::npos)
			version = extension > version ? extension : version;

		size_t lineAfterVersion = 2, bufferInsertOffset = 0;

		if (version != std::string::npos)
		{
			for (size_t i = 0; i < version; ++i)
			{
				if (parsedShaderText[i] == '\n')
					lineAfterVersion++;
			}

			bufferInsertOffset = version;
			for (size_t i = version; i < parsedShaderText.length(); ++i)
			{
				bufferInsertOffset += 1;
				if (parsedShaderText[i] == '\n')
					break;
			}
		}

		parsedShaderText = parsedShaderText.substr(0, bufferInsertOffset) + "\nbuffer debugBuffer\n{\nuint currentLocation;\nfloat debugData[];\n};\n#line " + std::to_string(lineAfterVersion) + "\n" + parsedShaderText.substr(bufferInsertOffset);
#endif //layout (std430)
	}
	
	return parsedShaderText;
}

GLuint FEShader::getUniformLocation(int& uniformNameHash)
{
	return uniformLocations[uniformNameHash];
}

void FEShader::loadScalar(int& uniformNameHash, GLfloat& value)
{
	FE_GL_ERROR(glUniform1f(uniformLocations[uniformNameHash], value));
}

void FEShader::loadScalar(int& uniformNameHash, GLint& value)
{
	FE_GL_ERROR(glUniform1i(uniformLocations[uniformNameHash], value));
}

void FEShader::loadVector(int& uniformNameHash, glm::vec2& vector)
{
	FE_GL_ERROR(glUniform2f(uniformLocations[uniformNameHash], vector.x, vector.y));
}

void FEShader::loadVector(int& uniformNameHash, glm::vec3& vector)
{
	FE_GL_ERROR(glUniform3f(uniformLocations[uniformNameHash], vector.x, vector.y, vector.z));
}

void FEShader::loadVector(int& uniformNameHash, glm::vec4& vector)
{
	FE_GL_ERROR(glUniform4f(uniformLocations[uniformNameHash], vector.x, vector.y, vector.z, vector.w));
}

void FEShader::loadMatrix(int& uniformNameHash, glm::mat4& matrix)
{
	FE_GL_ERROR(glUniformMatrix4fv(uniformLocations[uniformNameHash], 1, false, glm::value_ptr(matrix)));
}

void FEShader::loadIntArray(int& uniformNameHash, GLint* array, size_t arraySize)
{
	FE_GL_ERROR(glUniform1iv(uniformLocations[uniformNameHash], int(arraySize), array));
}

void FEShader::loadIntArray(GLuint uniformLocation, GLint* array, size_t arraySize)
{
	FE_GL_ERROR(glUniform1iv(uniformLocation, int(arraySize), array));
}

void FEShader::loadFloatArray(int& uniformNameHash, GLfloat* array, size_t arraySize)
{
	FE_GL_ERROR(glUniform1fv(uniformLocations[uniformNameHash], int(arraySize), array));
}

void FEShader::loadDataToGPU()
{
	auto iterator = parameters.begin();
	while (iterator != parameters.end())
	{
		if (iterator->second.data == nullptr)
			continue;

		switch (iterator->second.type)
		{
			case FE_INT_SCALAR_UNIFORM:
			{
				loadScalar(iterator->second.nameHash, *(int*)iterator->second.data);
				break;
			}

			case FE_FLOAT_SCALAR_UNIFORM:
			{
				loadScalar(iterator->second.nameHash, *(float*)iterator->second.data);
				break;
			}

			case FE_VECTOR2_UNIFORM:
			{
				loadVector(iterator->second.nameHash, *(glm::vec2*)iterator->second.data);
				break;
			}

			case FE_VECTOR3_UNIFORM:
			{
				loadVector(iterator->second.nameHash, *(glm::vec3*)iterator->second.data);
				break;
			}

			case FE_VECTOR4_UNIFORM:
			{
				loadVector(iterator->second.nameHash, *(glm::vec4*)iterator->second.data);
				break;
			}

			case FE_MAT4_UNIFORM:
			{
				loadMatrix(iterator->second.nameHash, *(glm::mat4*)iterator->second.data);
				break;
			}

			default:
				break;
		}
		iterator++;
	}
}

void FEShader::addParameter(FEShaderParam Parameter)
{
	/*bool find = false;
	for (size_t i = 0; i < FEStandardUniforms.size(); i++)
	{
		if (Parameter.getName().find(FEStandardUniforms[i]) != GL_INVALID_INDEX)
			find = true;
	}
	Parameter.loadedFromEngine = find;*/

	parameters[Parameter.getName()] = Parameter;

	parameters[Parameter.getName()].nameHash = int(std::hash<std::string>{}(Parameter.getName()));
	uniformLocations[parameters[Parameter.getName()].nameHash] = glGetUniformLocation(programID, Parameter.getName().c_str());
}

std::vector<std::string> FEShader::getParameterList()
{
	FE_MAP_TO_STR_VECTOR(parameters)
}

FEShaderParam* FEShader::getParameter(std::string name)
{
	if (parameters.find(name) == parameters.end())
	{
		LOG.add(std::string("getParameter can't find : ") + name + " in function FEShader::getParameter", FE_LOG_WARNING, FE_LOG_RENDERING);
		return nullptr;
	}

	return &parameters[name];
}

std::vector<std::string> FEShader::getTextureList()
{
	return textureUniforms;
}

char* FEShader::getVertexShaderText()
{
	return vertexShaderText;
}

char* FEShader::getFragmentShaderText()
{
	return fragmentShaderText;
}

std::string FEShader::getCompilationErrors()
{
	return compilationErrors;
}

char* FEShader::getTessControlShaderText()
{
	return tessControlShaderText;
}

char* FEShader::getTessEvalShaderText()
{
	return tessEvalShaderText;
}

char* FEShader::getGeometryShaderText()
{
	return geometryShaderText;
}

char* FEShader::getComputeShaderText()
{
	return computeShaderText;
}

#ifdef FE_DEBUG_ENABLED
inline void FEShader::createSSBO()
{
	if (!debugRequest)
		return;
	FE_GL_ERROR(glCreateBuffers(1, &SSBO));
	FE_GL_ERROR(glNamedBufferData(SSBO, SSBOSize * 4, nullptr, GL_STREAM_READ));

	SSBOBinding = -1;
	GLenum prop = GL_BUFFER_BINDING;
	FE_GL_ERROR(glGetProgramResourceiv(this->programID, GL_SHADER_STORAGE_BLOCK, glGetProgramResourceIndex(this->programID, GL_SHADER_STORAGE_BLOCK, "debugBuffer"), 1, &prop, sizeof(SSBOBinding), nullptr, &SSBOBinding));

	thisFrameDebugBind = 0;
}
#endif

bool FEShader::isDebugRequest()
{
#ifdef FE_DEBUG_ENABLED
	return debugRequest;
#endif
	return false;
}

std::vector<std::vector<float>>* FEShader::getDebugData()
{
#ifdef FE_DEBUG_ENABLED
	return &debugData;
#endif

	return nullptr;
}

std::vector<std::string> FEShader::getDebugVariables()
{
#ifdef FE_DEBUG_ENABLED
	return debugVariables;
#endif

	return std::vector<std::string>();
}

void FEShader::dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
	if (getComputeShaderText() == nullptr)
		return;

	FE_GL_ERROR(glDispatchCompute(num_groups_x, num_groups_y, num_groups_z));
}

void FEShader::reCompile(std::string name, const char* vertexText, const char* fragmentText,
						 const char* tessControlText, const char* tessEvalText,
						 const char* geometryText, const char* computeText, bool testCompilation, int glslVersion)
{
	cleanUp();

	this->glslVersion = glslVersion;
	testCompilationMode = testCompilation;
	setName(name);
	size_t textLenght = 0;

	if (vertexText != nullptr)
	{
		vertexShaderID = loadShader(vertexText, GL_VERTEX_SHADER);
		textLenght = strlen(vertexText);
		vertexShaderText = new char[textLenght + 1];
		strcpy_s(vertexShaderText, textLenght + 1, vertexText);
	}

	if (tessControlText != nullptr)
	{
		tessControlShaderID = loadShader(tessControlText, GL_TESS_CONTROL_SHADER);
		size_t textLenght = strlen(tessControlText);
		tessControlShaderText = new char[textLenght + 1];
		strcpy_s(tessControlShaderText, textLenght + 1, tessControlText);
	}

	if (tessEvalText != nullptr)
	{
		tessEvalShaderID = loadShader(tessEvalText, GL_TESS_EVALUATION_SHADER);
		size_t textLenght = strlen(tessEvalText);
		tessEvalShaderText = new char[textLenght + 1];
		strcpy_s(tessEvalShaderText, textLenght + 1, tessEvalText);
	}

	if (geometryText != nullptr)
	{
		geometryShaderID = loadShader(geometryText, GL_GEOMETRY_SHADER);
		size_t textLenght = strlen(geometryText);
		geometryShaderText = new char[textLenght + 1];
		strcpy_s(geometryShaderText, textLenght + 1, geometryText);
	}

	if (fragmentText != nullptr)
	{
		fragmentShaderID = loadShader(fragmentText, GL_FRAGMENT_SHADER);
		textLenght = strlen(fragmentText);
		fragmentShaderText = new char[textLenght + 1];
		strcpy_s(fragmentShaderText, textLenght + 1, fragmentText);
	}

	if (computeText != nullptr)
	{
		testCompilationMode = testCompilation;
		computeShaderID = loadShader(computeText, GL_COMPUTE_SHADER);
		size_t textLenght = strlen(computeText);
		computeShaderText = new char[textLenght + 1];
		strcpy_s(computeShaderText, textLenght + 1, computeText);
	}

	if (testCompilationMode && compilationErrors.size() != 0)
		return;

	FE_GL_ERROR(programID = glCreateProgram());

	if (vertexText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, vertexShaderID));
	if (tessControlText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, tessControlShaderID));
	if (tessEvalText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, tessEvalShaderID));
	if (geometryText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, geometryShaderID));
	if (fragmentText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, fragmentShaderID));
	if (computeText != nullptr)
		FE_GL_ERROR(glAttachShader(programID, computeShaderID));

	bindAttributes();

	FE_GL_ERROR(glLinkProgram(programID));
	FE_GL_ERROR(glValidateProgram(programID)); // too slow ?

	if (vertexText != nullptr)
		FE_GL_ERROR(glDeleteShader(vertexShaderID));
	if (tessControlText != nullptr)
		FE_GL_ERROR(glDeleteShader(tessControlShaderID));
	if (tessEvalText != nullptr)
		FE_GL_ERROR(glDeleteShader(tessEvalShaderID));
	if (geometryText != nullptr)
		FE_GL_ERROR(glDeleteShader(geometryShaderID));
	if (fragmentText != nullptr)
		FE_GL_ERROR(glDeleteShader(fragmentShaderID));
	if (computeText != nullptr)
		FE_GL_ERROR(glDeleteShader(computeShaderID));

#ifdef FE_DEBUG_ENABLED
	createSSBO();
#endif
	registerUniforms();
}