#pragma once

#include "../SubSystems/FEFileSystem.h"

#ifdef OLD_LOADING

namespace FocalEngine
{
	class TextureLoadJob
	{
		std::thread ThreadHandler;
		std::atomic<bool> LoadingDone;
		std::atomic<bool> NewJobsReady;
		std::vector<std::pair<std::string, void*>> TexturesToLoad;
		std::vector<std::pair<char*, void*>> TexturesData;

		void LoadTextureFunc();
	public:
		TextureLoadJob();
		~TextureLoadJob();

		bool IsThreadReadyForJob();
		bool BeginJobsUpdate();
		void AddTextureToLoad(std::pair<std::string, void*> TextureFilePath);
		int GetReadyJobCount();
		std::pair<char**, void*> GetJobByIndex(size_t Index);
		void EndJobsUpdate();
		bool ClearJobs();
	};

	class FEResourceManager;

	class FEJobManager
	{
		friend FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEJobManager)

		void LoadTextureAsync(std::string TexturePath, void* Texture);
		int GetFreeTextureThreadCount();
	private:
		SINGLETON_PRIVATE_PART(FEJobManager)

		std::vector<TextureLoadJob*> TextureLoadJobs;
		std::vector<std::pair<std::string, void*>> TextureListToLoad;
	};

	#define JOB_MANAGER FEJobManager::getInstance()
}

#endif // OLD_LOADING