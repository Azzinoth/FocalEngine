#include "../Editor/FEEditor.h"


static const char* const MyVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const MyFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;
uniform float blurSize;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	depthValue = (1 - depthValue) * 10;
	//gl_FragColor = vec4(vec3((1 - depthValue) * 10), 1.0); // only for perspective projection

	if (depthValue > depthThreshold)
	{
		//gl_FragColor = texture(depthTexture, textureCoords) * vec4(0.5);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords; // * 0.5 + 0.5

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * blurSize);
		}

		gl_FragColor = vec4(0.0);
		gl_FragColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}

}
)";

static const char* const MyFS2 = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;
uniform float blurSize;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	depthValue = (1 - depthValue) * 10;
	//gl_FragColor = vec4(vec3((1 - depthValue) * 10), 1.0); // only for perspective projection

	if (depthValue > depthThreshold)
	{
		//gl_FragColor = texture(depthTexture, textureCoords) * vec4(0.5);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords; // * 0.5 + 0.5

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * blurSize, 0.0);
		}

		gl_FragColor = vec4(0.0);
		gl_FragColor = vec4(0.0);
		gl_FragColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}

}
)";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	engine.createWindow();
	engine.setKeyCallback(keyButtonCallback);
	engine.setMouseButtonCallback(mouseButtonCallback);
	engine.setMouseMoveCallback(mouseMoveCallback);

	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	/*FocalEngine::FEMaterial* testMat = resourceManager.createMaterial("TestMaterial");
	testMat->shader = new FocalEngine::FEShader(FEPhongVS, FEPhongFS);
	FocalEngine::FEShaderParam color(glm::vec3(0.0f, 0.4f, 0.6f), "baseColor");
	testMat->addParameter(color);*/

	loadEditor();

	/*FocalEngine::FEPostProcess* testEffect = engine.createPostProcess("DOF");
	FocalEngine::FEShader* testshader = new FocalEngine::FEShader(MyVS, MyFS);
	FocalEngine::FEShader* testshader2 = new FocalEngine::FEShader(MyVS, MyFS2);
	for (size_t i = 0; i < 1; i++)
	{
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(2.0f);
		testEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(0.1f);
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader2));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(2.0f);
		testEffect->stages.back()->shader->getParameter("depthThreshold")->updateData(0.1f);
	}
	renderer.addPostProcess(testEffect);*/

	while (engine.isWindowOpened())
	{
		engine.beginFrame();
		engine.render();

		if (FERenderer::getInstance().CSM0 != nullptr)
		{
			//ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM0->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM1->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM2->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			//ImGui::Image((void*)(intptr_t)FERenderer::getInstance().CSM3->getTextureID(), ImVec2(256, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}

		//ImGui::ShowDemoWindow();
		renderEditor();
		engine.endFrame();

		// CPU and GPU Time
		std::string cpuMS = std::to_string(engine.getCpuTime());
		cpuMS.erase(cpuMS.begin() + 4, cpuMS.end());

		std::string gpuMS = std::to_string(engine.getGpuTime());
		gpuMS.erase(gpuMS.begin() + 4, gpuMS.end());

		std::string caption = "CPU time : ";
		caption += cpuMS;
		caption += " ms";
		caption += "  Frame time : ";
		caption += gpuMS;
		caption += " ms";
		engine.setWindowCaption(caption.c_str());
	}
	
	return 0;
}