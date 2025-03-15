#pragma once

#include "../SubSystems/FileSystem/FEFileSystem.h"
#include "../Core/FEGeometricTools.h"

namespace FocalEngine
{
	struct FERawPLYData
	{
		
	};

	class FEPLYParser
	{
		friend class FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEPLYParser)

		void ParseFile(std::string FilePath);

		// Use to get raw data from the loaded file.
		// Recommenede only if you know what you are doing.
		std::vector<FERawPLYData*>* GetLoadedObjects();

		//bool SaveToPLY(std::string FilePath, FERawPLYData* Data);
	private:
		SINGLETON_PRIVATE_PART(FEPLYParser)
			
		std::vector<FERawPLYData*> LoadedObjects;

		
	};
}