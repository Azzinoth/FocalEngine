#pragma once

#include "../SubSystems/FEFileSystem.h"

namespace FocalEngine
{
	class textureLoadJob
	{
		std::thread threadHandler;
		std::atomic<bool> loadingDone;
		std::atomic<bool> newJobsReady;
		std::vector<std::pair<std::string, void*>> texturesToLoad;
		std::vector<std::pair<char*, void*>> texturesData;

		void loadTextureFunc();
	public:
		textureLoadJob();
		~textureLoadJob();

		bool isThreadReadyForJob();
		bool beginJobsUpdate();
		void addTextureToLoad(std::pair<std::string, void*> textureFilePath);
		int getReadyJobCount();
		std::pair<char**, void*> getJobByIndex(size_t index);
		void endJobsUpdate();
		bool clearJobs();
	};

	class FERenderer;

	class FEJobManager
	{
		friend FERenderer;
	public:
		SINGLETON_PUBLIC_PART(FEJobManager)

		void loadTextureAsync(std::string texturePath, void* texture);
		int getFreeTextureThreadCount();
	private:
		SINGLETON_PRIVATE_PART(FEJobManager)

		std::vector<textureLoadJob*> textureLoadJobs;
		std::vector<std::pair<std::string, void*>> textureListToLoad;
	};

	#define JOB_MANAGER FEJobManager::getInstance()
}

