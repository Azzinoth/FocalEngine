#include "FEStandardMaterial.h"
using namespace FocalEngine;

FEStandardMaterial::FEStandardMaterial()
{
	baseColor = glm::vec3(1.0f, 0.4f, 0.6f);
	shaders.push_back(new FEStandardShader());

	std::vector<FEShaderInputData> dataForShader;
	dataForShader.push_back(FEShaderInputData(FE_VECTOR3_UNIFORM, new glm::vec3(baseColor), std::string("baseColor")));
	dataForShader.push_back(FEShaderInputData(FE_VECTOR3_UNIFORM, new glm::vec3(glm::vec3(1.0f, 1.0f, 1.6f)), std::string("secondBaseColor")));
	
	shaders[0]->consumeData(dataForShader);
}

FEStandardMaterial::~FEStandardMaterial()
{
}

glm::vec3 FEStandardMaterial::getBaseColor()
{
	return baseColor;
}

void FEStandardMaterial::setBaseColor(glm::vec3 newColor)
{
	baseColor = newColor;

	// memory leak
	std::vector<FEShaderInputData> dataForShader;
	dataForShader.push_back(FEShaderInputData(FE_VECTOR3_UNIFORM, new glm::vec3(baseColor), std::string("baseColor")));
	shaders[0]->consumeData(dataForShader);
}