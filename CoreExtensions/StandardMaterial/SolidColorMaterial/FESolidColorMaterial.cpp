#include "FESolidColorMaterial.h"
using namespace FocalEngine;

FESolidColorMaterial::FESolidColorMaterial()
{
	baseColor = glm::vec3(1.0f, 0.4f, 0.6f);
	
	shaders.push_back(new FESolidColorShader());
	setParam("baseColor", baseColor);
}

FESolidColorMaterial::~FESolidColorMaterial()
{
}