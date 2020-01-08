#include "FEPhongMaterial.h"
using namespace FocalEngine;

FEPhongMaterial::FEPhongMaterial(FETexture* baseColorTexture)
{
	addTexture(baseColorTexture);
	shader = new FEPhongShader();
}

FEPhongMaterial::~FEPhongMaterial()
{
}