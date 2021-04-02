#pragma once

#include "..\SubSystems\FEObject.h"

namespace FocalEngine
{
	class FELOG
	{
	public:
		SINGLETON_PUBLIC_PART(FELOG)
		void logError(std::string text);
		//void log(std::string text);
		std::vector<std::string> log;
	private:
		SINGLETON_PRIVATE_PART(FELOG)
	};

	#define LOG FELOG::getInstance()
}