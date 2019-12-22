#pragma once

#include "../Renderer/FEEntity.h"

namespace FocalEngine
{
	class FEResourceManager
	{
	public:
		static FEResourceManager& getInstance()
		{
			if (!_instance)
				_instance = new FEResourceManager();

			return *_instance;
		}
		~FEResourceManager();

		FEMesh* getSimpleMesh(std::string meshName);
	private:
		static FEResourceManager* _instance;
		FEResourceManager();
		SINGLETON_COPY_ASSIGN_PART(FEResourceManager)
		FEMesh* cube;
	};
}