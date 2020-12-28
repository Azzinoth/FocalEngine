#include "FEJobManager.h"
using namespace FocalEngine;

FEJobManager* FEJobManager::_instance = nullptr;

void textureLoadJob::loadTextureFunc()
{
	while (true)
	{
		newJobsReady = false;

		for (size_t i = 0; i < texturesToLoad.size(); i++)
		{
			std::fstream file;
			file.open(texturesToLoad[i].first.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
			std::streamsize fileSize = file.tellg();
			if (fileSize < 0)
				continue;

			file.seekg(0, std::ios::beg);
			char* fileData = new char[int(fileSize)];
			file.read(fileData, fileSize);
			file.close();

			texturesData.push_back(std::make_pair(fileData, texturesToLoad[i].second));
		}

		loadingDone = true;
		while (true)
		{
			Sleep(5);
			if (newJobsReady.load())
				break;
		}
	}
}

textureLoadJob::textureLoadJob()
{
	loadingDone = false;
	newJobsReady = false;
	threadHandler = std::thread(&textureLoadJob::loadTextureFunc, this);
	threadHandler.detach();
}

textureLoadJob::~textureLoadJob()
{

}

bool textureLoadJob::isThreadReadyForJob()
{
	return loadingDone.load();
}

bool textureLoadJob::beginJobsUpdate()
{
	if (loadingDone.load())
	{
		texturesToLoad.clear();
		return true;
	}

	return false;
}

void textureLoadJob::addTextureToLoad(std::pair<std::string, void*> textureFilePath)
{
	if (!loadingDone.load())
		return;

	if (textureFilePath.first == "")
		return;

	texturesToLoad.push_back(textureFilePath);
}

int textureLoadJob::getReadyJobCount()
{
	if (!loadingDone.load())
		return 0;

	return texturesData.size();
}

std::pair<char**, void*> textureLoadJob::getJobByIndex(size_t index)
{
	if (!loadingDone.load())
		return std::make_pair(nullptr, nullptr);

	if (index >= texturesData.size())
		return std::make_pair(nullptr, nullptr);

	return std::make_pair(&texturesData[index].first, texturesData[index].second);
}

bool textureLoadJob::clearJobs()
{
	if (!loadingDone.load())
		return false;

	for (size_t i = 0; i < texturesData.size(); i++)
	{
		delete[] texturesData[i].first;
	}
	texturesData.clear();

	return true;
}

void textureLoadJob::endJobsUpdate()
{
	if (!loadingDone.load())
		return;

	loadingDone = false;
	newJobsReady = true;
}

FEJobManager::FEJobManager()
{
	textureLoadJobs.resize(4);
	for (size_t i = 0; i < textureLoadJobs.size(); i++)
	{
		textureLoadJobs[i] = new textureLoadJob();
	}
}

FEJobManager::~FEJobManager()
{

}

void FEJobManager::loadTextureAsync(std::string texturePath, void* texture)
{
	textureListToLoad.push_back(std::make_pair(texturePath, texture));
}

int FEJobManager::getFreeTextureThreadCount()
{
	int result = 0;
	for (size_t i = 0; i < textureLoadJobs.size(); i++)
	{
		result += textureLoadJobs[i]->isThreadReadyForJob() ? 1 : 0;
	}

	return result;
}
