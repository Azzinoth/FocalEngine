#include "FETime.h"
using namespace FocalEngine;

FETime::FETime()
{
	QueryPerformanceFrequency(&Frequency);
}

void FETime::beginTimeStamp()
{
	QueryPerformanceCounter(&StartingTime);
}

float FETime::endTimeStamp()
{
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	//float msEscaped = ElapsedMicroseconds.QuadPart / 1000.0f;
	return ElapsedMicroseconds.QuadPart / 1000.0f;
}