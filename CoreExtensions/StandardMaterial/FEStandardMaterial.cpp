#include "FEStandardMaterial.h"
using namespace FocalEngine;

FEStandardMaterial::FEStandardMaterial()
{
	baseColor = glm::vec3(1.0f, 0.4f, 0.6f);
	shaders.push_back(new FEStandardShader());

	shaders[0]->addParams(FEShaderParam(baseColor, std::string("baseColor")));
	shaders[0]->addParams(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.6f), std::string("secondBaseColor")));
}

FEStandardMaterial::~FEStandardMaterial()
{
}