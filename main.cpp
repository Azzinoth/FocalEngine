#include "../Editor/FEEditor.h"


static const char* const MyVS = R"(
#version 450 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const MyFS = R"(
#version 450 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;
uniform float depthThresholdFar;
uniform float blurSize;
uniform float intMult;
uniform float zNear;
uniform float zFar;
//uniform float maxGap;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	float depthValueWorld = 2.0 * depthValue - 1.0;
	depthValueWorld = 2.0 * zNear * zFar / (zFar + zNear - depthValueWorld * (zFar - zNear));


	float A = -(zFar + zNear) / (zFar - zNear);
    float B = (-2 * zFar * zNear) / (zFar - zNear);
	float thresholdInZbuffer = 0.5 * (-A * depthThreshold + B) / depthThreshold + 0.5;
	float thresholdInZbufferFar = 0.5 * (-A * depthThresholdFar + B) / depthThresholdFar + 0.5;

	vec4 finalColor = vec4(0.0);
	if (depthValueWorld < depthThreshold)
	{
		float intensity = (thresholdInZbuffer - depthValue) * (intMult / 10.0);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * blurSize * intensity);
		}

		//float maxGap = 0.01;
		//vec4 gapColor = vec4(10.0, 10.0, 10.0, 0.3);
		//vec4 gapColor = texture(sceneTexture, textureCoords);
		
		//float sampleDepth = texture(depthTexture, blurTextureCoords[0]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.0093;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		//}

		//sampleDepth = texture(depthTexture, blurTextureCoords[1]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.028002;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		//}

		//sampleDepth = texture(depthTexture, blurTextureCoords[2]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.065984;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		//}
		
		//sampleDepth = texture(depthTexture, blurTextureCoords[3]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.121703;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		//}
		
		//sampleDepth = texture(depthTexture, blurTextureCoords[4]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.175713;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		//}

		//sampleDepth = texture(depthTexture, blurTextureCoords[5]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.198596;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		//}

		//sampleDepth = texture(depthTexture, blurTextureCoords[6]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.175713;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		//}
		
		//sampleDepth = texture(depthTexture, blurTextureCoords[7]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.121703;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		//}

		//sampleDepth = texture(depthTexture, blurTextureCoords[8]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.065984;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		//}
		
		//sampleDepth = texture(depthTexture, blurTextureCoords[9]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.028002;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		//}
		
		//sampleDepth = texture(depthTexture, blurTextureCoords[10]).r;
		//if (abs(depthValue - sampleDepth) > maxGap)
		//{
		//	finalColor += gapColor * 0.0093;
		//}
		//else
		//{
		//	finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
		//}

		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else if (depthValueWorld > depthThresholdFar)
	{
		float intensity = (thresholdInZbufferFar - depthValue) * (intMult * 2.0);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * blurSize * intensity);
		}

		//gl_FragColor = vec4(0.0);
		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		finalColor = texture(sceneTexture, textureCoords);
	}

	gl_FragColor = finalColor;
}
)";

static const char* const MyFS2 = R"(
#version 450 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;
uniform float depthThresholdFar;
uniform float blurSize;
uniform float intMult;
uniform float zNear;
uniform float zFar;
//uniform float maxGap;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	float depthValueWorld = 2.0 * depthValue - 1.0;
	depthValueWorld = 2.0 * zNear * zFar / (zFar + zNear - depthValueWorld * (zFar - zNear));


	float A = -(zFar + zNear) / (zFar - zNear);
    float B = (-2 * zFar * zNear) / (zFar - zNear);
	float thresholdInZbuffer = 0.5 * (-A * depthThreshold + B) / depthThreshold + 0.5;
	float thresholdInZbufferFar = 0.5 * (-A * depthThresholdFar + B) / depthThresholdFar + 0.5;

	vec4 finalColor = vec4(0.0);
	if (depthValueWorld < depthThreshold)
	{
		float intensity = (thresholdInZbuffer - depthValue) * (intMult / 10.0);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * blurSize * intensity, 0.0);
		}

		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else if (depthValueWorld > depthThresholdFar)
	{
		float intensity = (thresholdInZbufferFar - depthValue) * (intMult * 2.0);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * blurSize * intensity, 0.0);
		}

		//gl_FragColor = vec4(0.0);
		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		finalColor = texture(sceneTexture, textureCoords);
	}

	gl_FragColor = finalColor;
}
)";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	ENGINE.createWindow();
	ENGINE.setKeyCallback(keyButtonCallback);
	ENGINE.setMouseButtonCallback(mouseButtonCallback);
	ENGINE.setMouseMoveCallback(mouseMoveCallback);

	//FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	/*FocalEngine::FEMaterial* testMat = resourceManager.createMaterial("TestMaterial");
	testMat->shader = new FocalEngine::FEShader(FEPhongVS, FEPhongFS);
	FocalEngine::FEShaderParam color(glm::vec3(0.0f, 0.4f, 0.6f), "baseColor");
	testMat->addParameter(color);*/

	loadEditor();

	FocalEngine::FEPostProcess* testEffect = ENGINE.createPostProcess("DOF");
	FocalEngine::FEShader* testshader = RESOURCE_MANAGER.createShader("DOF0", MyVS, MyFS);
	FocalEngine::FEShader* testshader2 = RESOURCE_MANAGER.createShader("DOF1", MyVS, MyFS2);
	for (size_t i = 0; i < 1; i++)
	{
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(2.0f);
		testEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(15.0f);
		testEffect->stages.back()->shader->getParameter("depthThresholdFar")->updateData(9000.0f);
		testEffect->stages.back()->shader->getParameter("zNear")->updateData(0.1f);
		testEffect->stages.back()->shader->getParameter("zFar")->updateData(5000.0f);
		testEffect->stages.back()->shader->getParameter("intMult")->updateData(100.0f);
		//testEffect->stages.back()->shader->getParameter("maxGap")->updateData(0.01f);
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader2));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(0.0f);
		testEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(15.0f);
		testEffect->stages.back()->shader->getParameter("depthThresholdFar")->updateData(9000.0f);
		testEffect->stages.back()->shader->getParameter("zNear")->updateData(0.1f);
		testEffect->stages.back()->shader->getParameter("zFar")->updateData(5000.0f);
		testEffect->stages.back()->shader->getParameter("intMult")->updateData(100.0f);
		//testEffect->stages.back()->shader->getParameter("maxGap")->updateData(0.01f);
	}
	renderer.addPostProcess(testEffect);

	/*FocalEngine::FEPostProcess* chromaticAberrationEffect = ENGINE.createPostProcess("chromaticAberration");
	FocalEngine::FEShader* chromaticAberrationShader = RESOURCE_MANAGER.createShader("chromaticAberrationShader", chromaticAberrationVS, chromaticAberrationFS);
	chromaticAberrationEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0 }, chromaticAberrationShader));
	chromaticAberrationEffect->stages.back()->shader->getParameter("intensity")->updateData(1.0f);
	renderer.addPostProcess(chromaticAberrationEffect);*/

	const int frameCountTillMeasure = 20;
	float cpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	float gpuFrameDurations[frameCountTillMeasure] = { 0.0f };
	int frameCounter = 0;

	float avarageCpuFrameDuration = 0.0f;
	float avarageGpuFrameDuration = 0.0f;

	while (ENGINE.isWindowOpened())
	{
		//Sleep(10);
		ENGINE.beginFrame();
		ENGINE.render();

		if (FERenderer::getInstance().CSM0 != nullptr)
		{
			ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM0->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM1->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM2->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM3->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
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