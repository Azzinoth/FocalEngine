#include "FEAsset.h"
using namespace FocalEngine;

FEAsset::FEAsset(FEAssetType assetType, std::string assetName)
{
	ID = getUniqueId();
	std::string IDinHex = "";
	for (size_t i = 0; i < ID.size(); i++)
	{
		IDinHex.push_back("0123456789ABCDEF"[(ID[i] >> 4) & 15]);
		IDinHex.push_back("0123456789ABCDEF"[ID[i] & 15]);
	}

	ID = IDinHex;
	type = assetType;
}

FEAsset::~FEAsset()
{
}

std::string FEAsset::getAssetID() const
{
	return ID;
}

FEAssetType FEAsset::getType() const
{
	return type;
}

bool FEAsset::getDirtyFlag()
{
	return dirtyFlag;
}

void FEAsset::setDirtyFlag(bool newDirtyFlag)
{
	dirtyFlag = newDirtyFlag;
}