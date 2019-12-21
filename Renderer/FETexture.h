#pragma once

#include "..\SubSystems\FECoreIncludes.h"
#include "..\ResourceManager\PNGTextureLoader.h"

namespace FocalEngine
{
	class FETexture
	{
	public:
		FETexture(char* fileName, std::string Name = "DefaultName");
		FETexture(std::string Name = "DefaultName");
		~FETexture();

		GLuint getTextureID();

		std::string getName();
		void setName(std::string newName);
	private:
		GLuint textureID = -1;
		std::string name = "DefaultName";

		/*float shineDamper = 1;
		float reflectivity = 0;

		bool hasTransparency = false;
		bool useFakeLighting = false;

		int numberOfRows = 1;*/
	};
}