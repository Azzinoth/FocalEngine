#include "FELog.h"
using namespace FocalEngine;

FELOG* FELOG::_instance = nullptr;

FELOG::FELOG()
{
}

FELOG::~FELOG()
{
}

void FELOG::logError(std::string text)
{
	if (log.size() < 100)
		log.push_back(text);
}