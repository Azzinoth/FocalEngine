#pragma once

#ifndef FEASSET_H
#define FEASSET_H

#include "..\SubSystems\FELog.h"

namespace FocalEngine
{
	enum FEAssetType
	{
		FE_NULL = 0,
		FE_SHADER = 1,
		FE_TEXTURE = 2,
		FE_MESH = 3,
		FE_MATERIAL = 4,
		FE_GAMEMODEL = 5,
		FE_ENTITY = 6,
		FE_TERRAIN = 7,
		FE_ENTITY_INSTANCED = 8
	};

	static std::string FEAssetTypeToString(FEAssetType type)
	{
		switch (type)
		{
			case FocalEngine::FE_NULL:
			{
				return "FE_NULL";
				break;
			}
			case FocalEngine::FE_SHADER:
			{
				return "FE_SHADER";
				break;
			}
			case FocalEngine::FE_TEXTURE:
			{
				return "FE_TEXTURE";
				break;
			}
			case FocalEngine::FE_MESH:
			{
				return "FE_MESH";
				break;
			}
			case FocalEngine::FE_MATERIAL:
			{
				return "FE_MATERIAL";
				break;
			}
			case FocalEngine::FE_GAMEMODEL:
			{
				return "FE_GAMEMODEL";
				break;
			}
			case FocalEngine::FE_ENTITY:
			{
				return "FE_ENTITY";
				break;
			}
			
			case FocalEngine::FE_TERRAIN:
			{
				return "FE_TERRAIN";
				break;
			}
			case FocalEngine::FE_ENTITY_INSTANCED:
			{
				return "FE_ENTITY_INSTANCED";
				break;
			}
			default:
				break;
		}

		return "FE_NULL";
	}

	class FEResourceManager;

	// This function can produce ID's that are identical but it is extremely rare
	// to be 100% sure I could implement system to prevent it but for the sake of simplicity I choose not to do that, at least for now.
	static std::string getUniqueId()
	{
		static bool firstInitialization = true;
		if (firstInitialization)
		{
			srand(unsigned int(time(NULL)));
			firstInitialization = false;
		}	

		std::string ID = "";
		ID += char(rand() % 128);
		for (size_t j = 0; j < 11; j++)
		{
			ID.insert(rand() % ID.size(), 1, char(rand() % 128));
		}

		return ID;
	}

	class FEShader;
	class FEMesh;
	class FETexture;
	class FEMaterial;
	class FEGameModel;
	class FEEntity;
	class FETerrain;
	class FEEntityInstanced;
	class FEScene;

	class FEAsset
	{
		friend FEShader;
		friend FEMesh;
		friend FETexture;
		friend FEMaterial;
		friend FEGameModel;
		friend FEEntity;
		friend FETerrain;
		friend FEEntityInstanced;
		friend FEResourceManager;
		friend FEScene;
	public:
		FEAsset(FEAssetType assetType, std::string assetName);
		~FEAsset();

		std::string getAssetID() const;
		FEAssetType getType() const;

		bool getDirtyFlag();
		void setDirtyFlag(bool newDirtyFlag);

		std::string getName();
		int getNameHash();
	private:
		std::string ID = "";
		FEAssetType type = FE_NULL;
		bool dirtyFlag = false;

		void setName(std::string newName);
		std::string name;
		int nameHash = 0;
	};
}

#endif FEASSET_H