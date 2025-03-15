#include "FEPLYParser.h"
using namespace FocalEngine;

FEPLYParser::FEPLYParser()
{
	
}

FEPLYParser::~FEPLYParser()
{

}

void FEPLYParser::ParseFile(std::string FilePath)
{
	
	for (size_t i = 0; i < LoadedObjects.size(); i++)
	{
		delete LoadedObjects[i];
	}
	LoadedObjects.clear();
	LoadedObjects.push_back(new FERawPLYData());

	if (FilePath.empty())
	{
		LOG.Add(std::string("Empty file path in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::ifstream File(FilePath, std::ios::binary);
	const auto begin = File.tellg();
	File.seekg(0, std::ios::end);
	const auto end = File.tellg();
	const auto fsize = static_cast<size_t>(end - begin);

	File.seekg(0, 0);

	std::string CurrentLine;
	for (size_t i = 0; i < fsize; i++)
	{
		char NewChar;
		File.read(&NewChar, 1);
		CurrentLine += NewChar;
			
		if (NewChar == '\n')
		{
			CurrentLine.erase(CurrentLine.end() - 1, CurrentLine.end());
			//ReadLine(std::stringstream(CurrentLine), LoadedObjects.back());

			CurrentLine = "";
		}
	}

}