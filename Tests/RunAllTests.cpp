#include "RunAllTests.h"
using namespace FocalEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize Google Test
	testing::GTEST_FLAG(output) = "xml:report.xml";
	testing::InitGoogleTest();

	ENGINE.InitWindow(1280, 720);
	THREAD_POOL.SetConcurrentThreadCount(10);
	//LOG.SetFileOutput(true);

	int TestResult = RUN_ALL_TESTS();

	while (ENGINE.IsNotTerminated())
	{
		ENGINE.BeginFrame();
		ENGINE.Render();
		ENGINE.EndFrame();
	}

	return 0;
}