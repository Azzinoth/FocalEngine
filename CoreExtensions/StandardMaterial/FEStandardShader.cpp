#include "FEStandardShader.h"
using namespace FocalEngine;

FEStandardShader::FEStandardShader()
	: FEShader(FEStandardVS, FEStandardFS, std::vector<std::string> {"vPos"})
{

}

FEStandardShader::~FEStandardShader()
{

}