#pragma once

#include "windows.h"

namespace FocalEngine
{
	class FETime
	{
		LARGE_INTEGER StartingTime;
		LARGE_INTEGER EndingTime;
		LARGE_INTEGER ElapsedMicroseconds;
		LARGE_INTEGER Frequency;

		FETime();
	public:
		static FETime& getInstance()
		{
			static FETime timer;
			return timer;
		}

		void beginTimeStamp();
		float endTimeStamp();
	};
}
