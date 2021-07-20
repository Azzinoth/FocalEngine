#include "../Editor/FEEditor.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ENGINE.createWindow();
	EDITOR.initializeResources();

	//GLFWwindow* window = glfwGetCurrentContext();

	//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	//offscreen_context = glfwCreateWindow(640, 480, "", NULL, ENGINE.window);

	//window = glfwGetCurrentContext();
	//glfwMakeContextCurrent(ENGINE.window);

	//window = glfwGetCurrentContext();



	/*FETransformComponent test = FETransformComponent();
	test.setPosition(glm::vec3(10.0f, 20.0f, 30.0f));
	test.setScale(glm::vec3(1.0f, 2.0f, 3.0f));
	test.setRotation(glm::vec3(45.0f, 24.0f, 67.0f));
	glm::mat4 testMat = test.getTransformMatrix();

	glm::vec3 extractedPosition = glm::vec3(testMat[3][0], testMat[3][1], testMat[3][2]);
	float extractedScaleX = glm::length(testMat[0]);
	float extractedScaleY = glm::length(testMat[1]);
	float extractedScaleZ = glm::length(testMat[2]);

	testMat[3][0] = 0.0f;
	testMat[3][1] = 0.0f;
	testMat[3][2] = 0.0f;

	testMat[0] /= extractedScaleX;
	testMat[1] /= extractedScaleY;
	testMat[2] /= extractedScaleZ;

	auto q = glm::quat_cast(testMat);
	glm::vec3 angles = glm::eulerAngles(q) * 180.0f / glm::pi<float>();*/


	const int frameCountTillMeasure = 20;
	float cpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	float gpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	int frameCounter = 0;

	float avarageCpuFrameDuration = 0.0f;
	float avarageGpuFrameDuration = 0.0f;

	int countOfFalse = 0;

	while (ENGINE.isWindowOpened())
	{
		ENGINE.beginFrame();
		ENGINE.render();

		if (RENDERER.CSM0 != nullptr)
		{
			//ImGui::Image((void*)(intptr_t)RENDERER.CSM0->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)RENDERER.CSM1->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)RENDERER.CSM2->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)RENDERER.CSM3->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
#ifdef USE_DEFERRED_RENDERER
			//ImGui::Image((void*)(intptr_t)RENDERER.positionsGBufferLastFrame->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)RENDERER.SSAOLastFrame->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			ImGui::Image((void*)(intptr_t)RENDERER.GBuffer->positions->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.GBuffer->normals->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.GBuffer->albedo->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)RENDERER.GBuffer->materialProperties->getTextureID(), ImVec2(256 * 1, 256 * 1), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
#endif
			std::string totalTimeDisk = "totalTimeDisk: " + std::to_string(RESOURCE_MANAGER.totalTimeDisk);
			ImGui::Text(totalTimeDisk.c_str());

			std::string TimeOpenGL = "TimeOpenGL: " + std::to_string(RESOURCE_MANAGER.TimeOpenGL);
			ImGui::Text(TimeOpenGL.c_str());

			std::string TimeOpenGLmip = "TimeOpenGLmip: " + std::to_string(RESOURCE_MANAGER.TimeOpenGLmip);
			ImGui::Text(TimeOpenGLmip.c_str());

			std::string TimeOpenGLmipload = "TimeOpenGLmipload: " + std::to_string(RESOURCE_MANAGER.TimeOpenGLmipload);
			ImGui::Text(TimeOpenGLmipload.c_str());

#ifdef EDITOR_SELECTION_DEBUG_MODE
			std::string objectsUnderMouse = "objectsUnderMouse: " + std::to_string(SELECTED.objectsUnderMouse.size());
			ImGui::Text(objectsUnderMouse.c_str());

			std::string colorIndex = "colorIndex: " + std::to_string(SELECTED.colorIndex);
			ImGui::Text(colorIndex.c_str());

			ImGui::Image((void*)(intptr_t)SELECTED.pixelAccurateSelectionFB->getColorAttachment()->getTextureID(), ImVec2(256 * 4, 256 * 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
#endif
		}

		//ImGui::ShowDemoWindow();
		EDITOR.render();
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