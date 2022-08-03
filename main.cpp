#include "../Editor/FEEditor.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ENGINE.InitWindow();
	EDITOR.InitializeResources();
	THREAD_POOL.SetConcurrentThreadCount(10);

	const int FrameCountTillMeasure = 20;
	double CPUFrameDurations[FrameCountTillMeasure] = { 0.0f };
	double GPUFrameDurations[FrameCountTillMeasure] = { 0.0f };
	int FrameCounter = 0;

	double AvarageCpuFrameDuration = 0.0;
	double AvarageGpuFrameDuration = 0.0;

	while (ENGINE.IsWindowOpened())
	{
		ENGINE.BeginFrame();
		ENGINE.Render();

#ifdef EDITOR_SELECTION_DEBUG_MODE
		std::string objectsUnderMouse = "objectsUnderMouse: " + std::to_string(SELECTED.objectsUnderMouse.size());
		ImGui::Text(objectsUnderMouse.c_str());

		std::string colorIndex = "colorIndex: " + std::to_string(SELECTED.colorIndex);
		ImGui::Text(colorIndex.c_str());

		ImGui::Image((void*)(intptr_t)SELECTED.pixelAccurateSelectionFB->getColorAttachment()->getTextureID(), ImVec2(256 * 4, 256 * 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
#endif

		//ImGui::ShowDemoWindow();
		EDITOR.Render();
		ENGINE.EndFrame();

		// CPU and GPU Time
		CPUFrameDurations[FrameCounter++] = ENGINE.GetCpuTime();
		GPUFrameDurations[FrameCounter++] = ENGINE.GetGpuTime();

		if (FrameCounter > FrameCountTillMeasure - 1)
		{
			AvarageCpuFrameDuration = 0.0f;
			AvarageGpuFrameDuration = 0.0f;
			for (size_t i = 0; i < FrameCountTillMeasure; i++)
			{
				AvarageCpuFrameDuration += CPUFrameDurations[i];
				AvarageGpuFrameDuration += GPUFrameDurations[i];
			}
			AvarageCpuFrameDuration /= FrameCountTillMeasure;
			AvarageGpuFrameDuration /= FrameCountTillMeasure;
			
			FrameCounter = 0;
		}

		std::string CPUMs = std::to_string(AvarageCpuFrameDuration);
		CPUMs.erase(CPUMs.begin() + 4, CPUMs.end());

		std::string GPUMs = std::to_string(AvarageGpuFrameDuration);
		GPUMs.erase(GPUMs.begin() + 4, GPUMs.end());

		std::string FrameMs = std::to_string(AvarageCpuFrameDuration + AvarageGpuFrameDuration);
		FrameMs.erase(FrameMs.begin() + 4, FrameMs.end());

		std::string caption = "CPU time : ";
		caption += CPUMs;
		caption += " ms";
		caption += "  GPU time : ";
		caption += GPUMs;
		caption += " ms";
		caption += "  Frame time : ";
		caption += FrameMs;
		caption += " ms";

		ENGINE.SetWindowCaption(caption.c_str());
	}
	
	return 0;
}