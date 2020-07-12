#include "FEShadowMapShader.h"
using namespace FocalEngine;

FEShadowMap::FEShadowMap()
	: FEShader(FEShadowMapVS, FEShadowMapFS, "FEShadowMapShader")
{
}

FEShadowMap::~FEShadowMap()
{
}