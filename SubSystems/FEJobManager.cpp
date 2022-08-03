#include "FEJobManager.h"

#ifdef OLD_LOADING
using namespace FocalEngine;

FEJobManager* FEJobManager::Instance = nullptr;

void TextureLoadJob::LoadTextureFunc()
{
	while (true)
	{
		NewJobsReady = false;

		for (size_t i = 0; i < TexturesToLoad.size(); i++)
		{
			std::fstream file;
			file.open(TexturesToLoad[i].first.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
			const std::streamsize FileSize = file.tellg();
			if (FileSize <= 0)
			{
				TexturesData.push_back(std::make_pair(nullptr, TexturesToLoad[i].second));
				continue;
			}

			file.seekg(0, std::ios::beg);
			char* FileData = new char[static_cast<int>(FileSize)];
			file.read(FileData, FileSize);
			file.close();

			TexturesData.push_back(std::make_pair(FileData, TexturesToLoad[i].second));
		}

		LoadingDone = true;
		while (true)
		{
			Sleep(5);
			if (NewJobsReady.load())
				break;
		}
	}
}

TextureLoadJob::TextureLoadJob()
{
	LoadingDone = false;
	NewJobsReady = false;
	ThreadHandler = std::thread(&TextureLoadJob::LoadTextureFunc, this);
	ThreadHandler.detach();
}

TextureLoadJob::~TextureLoadJob()
{

}

bool TextureLoadJob::IsThreadReadyForJob()
{
	return LoadingDone.load();
}

bool TextureLoadJob::BeginJobsUpdate()
{
	if (LoadingDone.load())
	{
		TexturesToLoad.clear();
		return true;
	}

	return false;
}

void TextureLoadJob::AddTextureToLoad(const std::pair<std::string, void*> TextureFilePath)
{
	if (!LoadingDone.load())
		return;

	if (TextureFilePath.first.empty())
		return;

	TexturesToLoad.push_back(TextureFilePath);
}

int TextureLoadJob::GetReadyJobCount()
{
	if (!LoadingDone.load())
		return 0;

	return static_cast<int>(TexturesData.size());
}

std::pair<char**, void*> TextureLoadJob::GetJobByIndex(const size_t Index)
{
	if (!LoadingDone.load())
		return std::make_pair(nullptr, nullptr);

	if (Index >= TexturesData.size())
		return std::make_pair(nullptr, nullptr);

	return std::make_pair(&TexturesData[Index].first, TexturesData[Index].second);
}

bool TextureLoadJob::ClearJobs()
{
	if (!LoadingDone.load())
		return false;

	for (size_t i = 0; i < TexturesData.size(); i++)
	{
		delete[] TexturesData[i].first;
	}
	TexturesData.clear();

	return true;
}

void TextureLoadJob::EndJobsUpdate()
{
	if (!LoadingDone.load())
		return;

	LoadingDone = false;
	NewJobsReady = true;
}

FEJobManager::FEJobManager()
{
	TextureLoadJobs.resize(4);
	for (size_t i = 0; i < TextureLoadJobs.size(); i++)
	{
		TextureLoadJobs[i] = new TextureLoadJob();
	}
}

FEJobManager::~FEJobManager()
{

}

void FEJobManager::LoadTextureAsync(std::string TexturePath, void* Texture)
{
	TextureListToLoad.push_back(std::make_pair(TexturePath, Texture));
}

int FEJobManager::GetFreeTextureThreadCount()
{
	int result = 0;
	for (size_t i = 0; i < TextureLoadJobs.size(); i++)
	{
		result += TextureLoadJobs[i]->IsThreadReadyForJob() ? 1 : 0;
	}

	return result;
}

#endif // OLD_LOADING