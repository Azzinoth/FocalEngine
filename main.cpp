#include "../Editor/FEEditor.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//srand(time(NULL));

	//int max = 0;
	//int min = 999;

	//std::unordered_map<int, std::string> IDs;
	//
	//for (size_t i = 0; i < 10000000; i++)
	//{
	//	std::string finalHashedString = "";
	//	finalHashedString += char(rand() % 128);
	//	for (size_t j = 0; j < 11; j++)
	//	{
	//		finalHashedString.insert(rand() % finalHashedString.size(), 1, char(rand() % 128));
	//	}
	//	int ID = std::hash<std::string>{}(finalHashedString);

	//	if (IDs.find(ID) == IDs.end())
	//	{
	//		IDs[ID] = finalHashedString;
	//	}
	//	else
	//	{
	//		i;
	//		bool strEqual = finalHashedString == IDs.find(ID)->second;
	//		if (strEqual)
	//		{
	//			int y = 0;
	//			y++;
	//		}
	//		int wasID = IDs.find(ID)->first;
	//		std::string wasString = IDs.find(ID)->second;
	//	}
	//}
	
	ENGINE.createWindow();
	ENGINE.setKeyCallback(keyButtonCallback);
	ENGINE.setMouseButtonCallback(mouseButtonCallback);
	ENGINE.setMouseMoveCallback(mouseMoveCallback);

	loadEditor();

	const int frameCountTillMeasure = 20;
	float cpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	float gpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	int frameCounter = 0;

	float avarageCpuFrameDuration = 0.0f;
	float avarageGpuFrameDuration = 0.0f;

	while (ENGINE.isWindowOpened())
	{
		ENGINE.beginFrame();
		ENGINE.render();

		if (RENDERER.CSM0 != nullptr)
		{
			ImGui::Image((void*)(intptr_t)RENDERER.CSM0->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.CSM1->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.CSM2->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.CSM3->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}

		//ImGui::ShowDemoWindow();

		renderEditor();
		ENGINE.endFrame();

		// CPU and GPU Time
		cpuFrameDurations[frameCounter++] = ENGINE.getCpuTime();
		gpuFrameDurations[frameCounter++] = ENGINE.getGpuTime();

		if (frameCounter > frameCountTillMeasure - 1)
		{
			avarageCpuFrameDuration = 0.0f;
			avarageGpuFrameDuration = 0.0f;
			for (size_t i = 0; i < frameCountTillMeasure; i++)
			{
				avarageCpuFrameDuration += cpuFrameDurations[i];
				avarageGpuFrameDuration += gpuFrameDurations[i];
			}
			avarageCpuFrameDuration /= frameCountTillMeasure;
			avarageGpuFrameDuration /= frameCountTillMeasure;
			
			frameCounter = 0;
		}

		std::string cpuMS = std::to_string(avarageCpuFrameDuration);
		cpuMS.erase(cpuMS.begin() + 4, cpuMS.end());

		std::string gpuMS = std::to_string(avarageGpuFrameDuration);
		gpuMS.erase(gpuMS.begin() + 4, gpuMS.end());

		std::string frameMS = std::to_string(avarageCpuFrameDuration + avarageGpuFrameDuration);
		frameMS.erase(frameMS.begin() + 4, frameMS.end());

		std::string caption = "CPU time : ";
		caption += cpuMS;
		caption += " ms";
		caption += "  GPU time : ";
		caption += gpuMS;
		caption += " ms";
		caption += "  Frame time : ";
		caption += frameMS;
		caption += " ms";
		ENGINE.setWindowCaption(caption.c_str());
	}
	
	return 0;
}