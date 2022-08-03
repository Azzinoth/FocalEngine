#include "FEPostProcess.h"
using namespace FocalEngine;

FEPostProcessStage::FEPostProcessStage(const int InTextureSource, FEShader* Shader)
{
	this->InTextureSource.push_back(InTextureSource);
	this->Shader = Shader;
}

FEPostProcessStage::FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader)
{
	this->InTextureSource = std::move(InTextureSource);
	this->Shader = Shader;
}

FEPostProcess::FEPostProcess(const std::string Name) : FEObject(FE_POST_PROCESS, Name)
{
}

FEPostProcess::~FEPostProcess()
{
	for (size_t i = 0; i < TexturesToDelete.size(); i++)
	{
		delete TexturesToDelete[i];
	}

	delete IntermediateFramebuffer;
}

void FocalEngine::FEPostProcess::RenderResult()
{
	ScreenQuadShader->Start();
	ScreenQuadShader->LoadDataToGPU();
	Stages.back()->OutTexture->Bind(0);

	FE_GL_ERROR(glBindVertexArray(ScreenQuad->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, ScreenQuad->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	Stages.back()->OutTexture->UnBind();
	ScreenQuadShader->Stop();
}

FETexture* FEPostProcess::GetInTexture()
{
	return FinalTexture;
}

void FEPostProcess::AddStage(FEPostProcessStage* NewStage)
{
	if (!NewStage->OutTexture)
		NewStage->OutTexture = FinalTexture;
	Stages.push_back(NewStage);
}

bool FEPostProcess::ReplaceOutTexture(const size_t StageIndex, FETexture* NewTexture, const bool bDeleteOldTexture)
{
	if (StageIndex >= Stages.size())
	{
		LOG.Add("Trying to replace texture in FEPostProcess::replaceOutTexture but stageIndex is out of bound!", FE_LOG_ERROR, FE_LOG_RENDERING);
		return false;
	}

	// Delete old texture from the delete list of FEPostProcess.
	if (Stages[StageIndex]->OutTexture != nullptr)
	{
		for (size_t i = 0; i < TexturesToDelete.size(); i++)
		{
			if (TexturesToDelete[i]->GetObjectID() == Stages[StageIndex]->OutTexture->GetObjectID())
			{
				if (bDeleteOldTexture)
					delete Stages[StageIndex]->OutTexture;

				TexturesToDelete.erase(TexturesToDelete.begin() + i);
				break;
			}
		}
	}
	
	Stages[StageIndex]->OutTexture = NewTexture;
	TexturesToDelete.push_back(NewTexture);

	return true;
}