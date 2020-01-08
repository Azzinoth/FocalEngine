#include "FESolidColorMaterial.h"
using namespace FocalEngine;

FESolidColorMaterial::FESolidColorMaterial()//short ID, std::string Name) : FEMaterial(ID, Name)
{
	baseColor = glm::vec3(1.0f, 0.4f, 0.6f);
	
	shader = new FESolidColorShader();

	FocalEngine::FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	addParameter(color);
	
	//setParam("baseColor", baseColor);
}

FESolidColorMaterial::~FESolidColorMaterial()
{
}