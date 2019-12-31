#include "FEPhongMaterial.h"
using namespace FocalEngine;

FEPhongMaterial::FEPhongMaterial(FETexture* baseColorTexture)
{
	addTexture(baseColorTexture);
	
	shaders.push_back(new FEPhongShader());
}

FEPhongMaterial::~FEPhongMaterial()
{
}