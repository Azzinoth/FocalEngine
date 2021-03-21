#include "FEPostProcess.h"
using namespace FocalEngine;

FEPostProcessStage::FEPostProcessStage(int InTextureSource, FEShader* Shader)
{
	inTextureSource.push_back(InTextureSource);
	shader = Shader;
}

FEPostProcessStage::FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader)
{
	inTextureSource = std::move(InTextureSource);
	shader = Shader;
}

FEPostProcess::FEPostProcess(std::string Name)
{
	name = Name;
}

FEPostProcess::~FEPostProcess()
{
	for (size_t i = 0; i < texturesToDelete.size(); i++)
	{
		delete texturesToDelete[i];
	}

	delete intermediateFramebuffer;
}

void FocalEngine::FEPostProcess::renderResult()
{
	screenQuadShader->start();
	screenQuadShader->loadDataToGPU();
	stages.back()->outTexture->bind(0);

	FE_GL_ERROR(glBindVertexArray(screenQuad->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	stages.back()->outTexture->unBind();
	screenQuadShader->stop();
}

FETexture* FEPostProcess::getInTexture()
{
	return finalTexture;
}

void FEPostProcess::addStage(FEPostProcessStage* newStage)
{
	if (!newStage->outTexture)
		newStage->outTexture = finalTexture;
	stages.push_back(newStage);
}

std::string FEPostProcess::getName()
{
	return name;
}

void FEPostProcess::setName(std::string newName)
{
	name = newName;
}

bool FEPostProcess::replaceOutTexture(size_t stageIndex, FETexture* newTexture, bool deleteOldTexture)
{
	if (stageIndex >= stages.size())
	{
		return false;
		LOG.logError("Trying to replace texture in FEPostProcess::replaceOutTexture but stageIndex is out of bound!");
	}

	// Delete old texture from the delete list of FEPostProcess.
	if (stages[stageIndex]->outTexture != nullptr)
	{
		for (size_t i = 0; i < texturesToDelete.size(); i++)
		{
			if (texturesToDelete[i]->getAssetID() == stages[stageIndex]->outTexture->getAssetID())
			{
				if (deleteOldTexture)
					delete stages[stageIndex]->outTexture;

				texturesToDelete.erase(texturesToDelete.begin() + i);
				break;
			}
		}
	}
	
	stages[stageIndex]->outTexture = newTexture;
	texturesToDelete.push_back(newTexture);

	return true;
}