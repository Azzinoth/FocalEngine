#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

FETexture* FEResourceManager::createTexture(std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedTexture";

	FETexture* newTexture = new FETexture(Name);
	if (forceObjectID != "")
		newTexture->setID(forceObjectID);
	textures[newTexture->getObjectID()] = newTexture;

	return newTexture;
}

bool FEResourceManager::makeTextureStandard(FETexture* texture)
{
	if (texture == nullptr)
	{
		LOG.add("texture is nullptr in function FEResourceManager::makeTextureStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardTextures.find(texture->getObjectID()) == standardTextures.end())
	{
		if (textures.find(texture->getObjectID()) != textures.end())
			textures.erase(texture->getObjectID());
		standardTextures[texture->getObjectID()] = texture;

		return true;
	}

	return false;
}

FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name)
{
	if (Name.size() == 0)
		Name = "unnamedMesh";

	FEMesh* newMesh = new FEMesh(VaoID, VertexCount, VertexBuffersTypes, AABB, Name);
	newMesh->setName(Name);
	meshes[newMesh->getObjectID()] = newMesh;

	return newMesh;
}

bool FEResourceManager::makeMeshStandard(FEMesh* mesh)
{
	if (mesh == nullptr)
	{
		LOG.add("mesh is nullptr in function FEResourceManager::makeMeshStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardMeshes.find(mesh->getObjectID()) == standardMeshes.end())
	{
		if (meshes.find(mesh->getObjectID()) != meshes.end())
			meshes.erase(mesh->getObjectID());
		standardMeshes[mesh->getObjectID()] = mesh;

		return true;
	}

	return false;
}

FETexture* FEResourceManager::LoadPNGTexture(const char* fileName, bool usingAlpha, std::string Name)
{
	FETexture* newTexture = createTexture(Name);
	std::vector<unsigned char> rawData;
	unsigned uWidth, uHeight;

	lodepng::decode(rawData, uWidth, uHeight, fileName);
	if (rawData.size() == 0)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadPNGTexture.", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	int internalFormat = usingAlpha ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());
	newTexture->internalFormat = internalFormat;

	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	newTexture->fileName = fileName;

	if (Name.size() == 0)
	{
		std::string filePath = newTexture->fileName;
		std::size_t index = filePath.find_last_of("/\\");
		std::string newFileName = filePath.substr(index + 1);
		index = newFileName.find_last_of(".");
		std::string fileNameWithOutExtention = newFileName.substr(0, index);
		newTexture->setName(fileNameWithOutExtention);
	}

	return newTexture;
}

FETexture* FEResourceManager::LoadPNGTextureWithTransparencyMask(const char* mainfileName, const char* maskFileName, std::string Name)
{
	FETexture* newTexture = createTexture(Name);
	std::vector<unsigned char> rawData;
	unsigned uWidth, uHeight;

	lodepng::decode(rawData, uWidth, uHeight, mainfileName);
	if (rawData.size() == 0)
	{
		LOG.add(std::string("can't load file: ") + mainfileName + " in function FEResourceManager::LoadPNGTextureWithTransparencyMask.", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	// Transparency mask part
	std::vector<unsigned char> maskRawData;
	lodepng::decode(maskRawData, uWidth, uHeight, maskFileName);
	if (maskRawData.size() == 0)
	{
		LOG.add(std::string("can't load file: ") + maskFileName + " in function FEResourceManager::LoadPNGTextureWithTransparencyMask.", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	for (size_t i = 4; i < uWidth * uHeight * 4; i+=4)
	{
		rawData[-1 + i] = maskRawData[-4 + i];
	}

	int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());
	newTexture->internalFormat = internalFormat;

	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	newTexture->fileName = mainfileName;

	if (Name.size() == 0)
	{
		std::string filePath = newTexture->fileName;
		std::size_t index = filePath.find_last_of("/\\");
		std::string newFileName = filePath.substr(index + 1);
		index = newFileName.find_last_of(".");
		std::string fileNameWithOutExtention = newFileName.substr(0, index);
		newTexture->setName(fileNameWithOutExtention);
	}

	return newTexture;
}

void FEResourceManager::saveFETexture(FETexture* texture, const char* fileName)
{
	// Height map
	if (texture->internalFormat == GL_R16)
	{
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture->textureID));

		GLint imgSize = 0;
		std::fstream file;

		file.open(fileName, std::ios::out | std::ios::binary);
		// version of FETexture file type
		float version = FE_TEXTURE_VERSION;
		file.write((char*)&version, sizeof(float));

		int objectIDSize = texture->getObjectID().size() + 1;
		file.write((char*)&objectIDSize, sizeof(int));
		file.write((char*)texture->getObjectID().c_str(), sizeof(char) * objectIDSize);

		file.write((char*)&texture->width, sizeof(int));
		file.write((char*)&texture->height, sizeof(int));
		file.write((char*)&texture->internalFormat, sizeof(int));

		int nameSize = texture->getName().size() + 1;
		file.write((char*)&nameSize, sizeof(int));

		char* textureName = new char[nameSize];
		strcpy_s(textureName, nameSize, texture->getName().c_str());
		file.write((char*)textureName, sizeof(char) * nameSize);

		int fileSize = texture->width * texture->height * 2;
		char* pixels = new char[fileSize];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, pixels));

		file.write((char*)&fileSize, sizeof(int));
		file.write((char*)pixels, sizeof(char) * fileSize);
		file.close();

		delete[] pixels;

		return;
	}

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture->textureID));
	int maxDimention = std::max(texture->width, texture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	char** pixelData = new char*[mipCount];
	GLint imgSize = 0;
	std::fstream file;

	file.open(fileName, std::ios::out | std::ios::binary);
	// version of FETexture file type
	float version = FE_TEXTURE_VERSION;
	file.write((char*)&version, sizeof(float));

	int objectIDSize = texture->getObjectID().size() + 1;
	file.write((char*)&objectIDSize, sizeof(int));
	file.write((char*)texture->getObjectID().c_str(), sizeof(char) * objectIDSize);

	file.write((char*)&texture->width, sizeof(int));
	file.write((char*)&texture->height, sizeof(int));
	file.write((char*)&texture->internalFormat, sizeof(int));

	int nameSize = texture->getName().size() + 1;
	file.write((char*)&nameSize, sizeof(int));

	char* textureName = new char[nameSize];
	strcpy_s(textureName, nameSize, texture->getName().c_str());
	file.write((char*)textureName, sizeof(char) * nameSize);

	for (size_t i = 0; i < mipCount; i++)
	{
		FE_GL_ERROR(glGetTexLevelParameteriv(GL_TEXTURE_2D, GLint(i), GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imgSize));

		char* pixels = new char[imgSize * 2];
		for (size_t i = 0; i < size_t(imgSize * 2); i++)
		{
			pixels[i] = ' ';
		}

		char* additionalTestPixels = new char[imgSize * 2];
		for (size_t i = 0; i < size_t(imgSize * 2); i++)
		{
			additionalTestPixels[i] = '1';
		}

		FE_GL_ERROR(glGetCompressedTexImage(GL_TEXTURE_2D, GLint(i), pixels));
		FE_GL_ERROR(glGetCompressedTexImage(GL_TEXTURE_2D, GLint(i), additionalTestPixels));

		int realSize = 0;
		for (size_t i = imgSize * 2 - 1; i > 0 ; i--)
		{
			if (pixels[i] != ' ')
			{
				realSize = i + 1;
				break;
			}
		}

		int additionalRealSize = 0;
		for (size_t i = imgSize * 2 - 1; i > 0; i--)
		{
			if (additionalTestPixels[i] != '1')
			{
				additionalRealSize = i + 1;
				break;
			}
		}

		realSize = std::max(realSize, additionalRealSize);

		pixelData[i] = new char[realSize];
		memcpy(pixelData[i], pixels, realSize);
		delete[] pixels;
		delete[] additionalTestPixels;

		file.write((char*)&realSize, sizeof(int));
		file.write((char*)pixelData[i], sizeof(char) * realSize);
	}

	file.close();

	for (size_t i = 0; i < mipCount; i++)
	{
		delete[] pixelData[i];
	}
	delete[] pixelData;
}

FETexture* FEResourceManager::rawDataToFETexture(char* textureData, int width, int height, bool isAlphaUsed)
{
	FETexture* newTexture = createTexture();
	newTexture->width = width;
	newTexture->height = height;
	newTexture->internalFormat = isAlphaUsed ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	
	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));// to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	return newTexture;
}

FETexture* FEResourceManager::LoadFETextureAsync(const char* fileName, std::string Name, FETexture* existingTexture, std::string forceObjectID)
{
	FETexture* newTexture = createTexture(Name, forceObjectID);
	FE_GL_ERROR(glDeleteTextures(1, &newTexture->textureID));
	newTexture->textureID = noTexture->textureID;
	newTexture->width = noTexture->width;
	newTexture->height = noTexture->height;
	newTexture->internalFormat = noTexture->internalFormat;
	newTexture->fileName = noTexture->fileName;

	JOB_MANAGER.loadTextureAsync(fileName, newTexture);

	return newTexture;
}

FETexture* FEResourceManager::LoadFETexture(const char* fileName, std::string Name, FETexture* existingTexture)
{
	auto start = std::chrono::system_clock::now();

	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture.", FE_LOG_ERROR, FE_LOG_LOADING);
		return this->noTexture;
	}
	
	file.seekg(0, std::ios::beg);
	char* fileData = new char[int(fileSize)];
	file.read(fileData, fileSize);
	file.close();

	int currentShift = 0;
	// version of FETexture file type
	float version = *(float*)(&fileData[currentShift]);
	currentShift += 4;
	if (version != FE_TEXTURE_VERSION)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture. File was created in different version of engine!", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	int objectIDSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* objectID = new char[objectIDSize];
	strcpy_s(objectID, objectIDSize, (char*)(&fileData[currentShift]));
	currentShift += objectIDSize;

	int width = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int height = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int internalFormat = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	int nameSize = 0;
	nameSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* textureName = new char[nameSize];
	strcpy_s(textureName, nameSize, (char*)(&fileData[currentShift]));
	currentShift += nameSize;

	FETexture* newTexture = nullptr;
	if (existingTexture != nullptr)
	{
		newTexture = existingTexture;
		newTexture->setName(textureName);
	}
	else
	{
		newTexture = createTexture(textureName);
	}
	
	newTexture->width = width;
	newTexture->height = height;
	newTexture->internalFormat = internalFormat;
	newTexture->fileName = fileName;

	auto end = std::chrono::system_clock::now();
	totalTimeDisk += std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count() * 1000.0f;

	start = std::chrono::system_clock::now();

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	// Height map should not be loaded by this function
	if (newTexture->internalFormat == GL_R16)
		return nullptr;

	int maxDimention = std::max(newTexture->width, newTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, mipCount, newTexture->internalFormat, newTexture->width, newTexture->height));

	end = std::chrono::system_clock::now();
	TimeOpenGL += std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count() * 1000.0f;
	start = std::chrono::system_clock::now();

	if (newTexture->mipEnabled)
	{
		//FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	end = std::chrono::system_clock::now();
	TimeOpenGLmip += std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count() * 1000.0f;
	start = std::chrono::system_clock::now();

	int mipW = newTexture->width / 2;
	int mipH = newTexture->height / 2;
	for (size_t i = 0; i < mipCount; i++)
	{
		int size = *(int*)(&fileData[currentShift]);
		currentShift += 4;

		if (i == 0)
		{
			FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newTexture->width, newTexture->height, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));
		}
		else
		{
			FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipW, mipH, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));
			
			mipW = mipW / 2;
			mipH = mipH / 2;

			if (mipW <= 0 || mipH <= 0)
				break;
		}

		currentShift += size;
	}

	// overwrite objectID with objectID from file.
	if (objectID != nullptr)
	{
		std::string oldID = newTexture->getObjectID();
		newTexture->setID(objectID);

		if (textures.find(oldID) != textures.end())
		{
			textures.erase(oldID);
			textures[newTexture->getObjectID()] = newTexture;
		}
	}

	end = std::chrono::system_clock::now();
	TimeOpenGLmipload += std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count() * 1000.0f;

	delete[] objectID;
	delete[] fileData;
	delete[] textureName;

	totalTimeDisk;
	TimeOpenGL;
	TimeOpenGLmip;
	TimeOpenGLmipload;

	return newTexture;
}

FETexture* FEResourceManager::LoadFETexture(char* fileData, std::string Name, FETexture* existingTexture)
{
	int currentShift = 0;
	// version of FETexture file type
	float version = *(float*)(&fileData[currentShift]);
	currentShift += 4;
	if (version != FE_TEXTURE_VERSION)
	{
		LOG.add(std::string("can't load fileData: in function FEResourceManager::LoadFETexture. FileData was created in different version of engine!"), FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	int objectIDSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* objectID = new char[objectIDSize];
	strcpy_s(objectID, objectIDSize, (char*)(&fileData[currentShift]));
	currentShift += objectIDSize;

	int width = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int height = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int internalFormat = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	int nameSize = 0;
	nameSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* textureName = new char[nameSize];
	strcpy_s(textureName, nameSize, (char*)(&fileData[currentShift]));
	currentShift += nameSize;

	FETexture* newTexture = nullptr;
	if (existingTexture != nullptr)
	{
		newTexture = existingTexture;
		newTexture->setName(textureName);
	}
	else
	{
		newTexture = createTexture(textureName);
	}

	newTexture->width = width;
	newTexture->height = height;
	newTexture->internalFormat = internalFormat;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	// Height map should not be loaded by this function
	if (newTexture->internalFormat == GL_R16)
		return nullptr;

	int maxDimention = std::max(newTexture->width, newTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, mipCount, newTexture->internalFormat, newTexture->width, newTexture->height));

	if (newTexture->mipEnabled)
	{
		//FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	int mipW = newTexture->width / 2;
	int mipH = newTexture->height / 2;
	for (size_t i = 0; i < mipCount; i++)
	{
		int size = *(int*)(&fileData[currentShift]);
		currentShift += 4;

		if (i == 0)
		{
			FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newTexture->width, newTexture->height, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));
		}
		else
		{
			FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipW, mipH, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));

			mipW = mipW / 2;
			mipH = mipH / 2;

			if (mipW <= 0 || mipH <= 0)
				break;
		}

		currentShift += size;
	}

	// overwrite objectID with objectID from file.
	if (objectID != nullptr)
	{
		std::string oldID = newTexture->getObjectID();
		newTexture->setID(objectID);

		if (textures.find(oldID) != textures.end())
		{
			textures.erase(oldID);
			textures[newTexture->getObjectID()] = newTexture;
		}
	}

	delete[] objectID;
	delete[] textureName;

	return newTexture;
}

FETexture* FEResourceManager::LoadFEHeightmap(const char* fileName, FETerrain* terrain, std::string Name)
{
	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture.", FE_LOG_ERROR, FE_LOG_LOADING);
		terrain->heightMap = this->noTexture;
		return this->noTexture;
	}

	file.seekg(0, std::ios::beg);
	char* fileData = new char[int(fileSize)];
	file.read(fileData, fileSize);
	file.close();

	int currentShift = 0;
	// version of FETexture file type
	float version = *(float*)(&fileData[currentShift]);
	currentShift += 4;
	if (version != FE_TEXTURE_VERSION)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture. File was created in different version of engine!", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	int objectIDSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* objectID = new char[objectIDSize];
	strcpy_s(objectID, objectIDSize, (char*)(&fileData[currentShift]));
	currentShift += objectIDSize;

	int width = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int height = *(int*)(&fileData[currentShift]);
	currentShift += 4;
	int internalFormat = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	int nameSize = 0;
	nameSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* textureName = new char[nameSize];
	strcpy_s(textureName, nameSize, (char*)(&fileData[currentShift]));
	currentShift += nameSize;

	FETexture* newTexture = Name.size() != 0 ? createTexture(Name.c_str()) : createTexture(textureName);
	newTexture->width = width;
	newTexture->height = height;
	newTexture->internalFormat = internalFormat;
	newTexture->fileName = fileName;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (newTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, 1, newTexture->internalFormat, newTexture->width, newTexture->height));

	int size = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	terrain->heightMapArray.resize(size / sizeof(unsigned short));
	float max = FLT_MIN;
	float min = FLT_MAX;
	for (size_t i = 0; i < size / sizeof(unsigned short); i++)
	{
		unsigned short temp = *(unsigned short*)(&fileData[currentShift]);
		terrain->heightMapArray[i] = temp / float(0xFFFF);
		currentShift += sizeof(unsigned short);

		if (max < terrain->heightMapArray[i])
			max = terrain->heightMapArray[i];

		if (min > terrain->heightMapArray[i])
			min = terrain->heightMapArray[i];
	}

	currentShift -= size;
	glm::vec3 minPoint = glm::vec3(-1.0f, min, -1.0f);
	glm::vec3 maxPoint = glm::vec3(1.0f, max, 1.0f);
	terrain->AABB = FEAABB(minPoint, maxPoint);

	FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newTexture->width, newTexture->height, GL_RED, GL_UNSIGNED_SHORT, (void*)(&fileData[currentShift])));

	// overwrite objectID with objectID from file.
	if (objectID != nullptr)
	{
		std::string oldID = newTexture->getObjectID();
		newTexture->setID(objectID);

		if (textures.find(oldID) != textures.end())
		{
			textures.erase(oldID);
			textures[newTexture->getObjectID()] = newTexture;
		}
	}

	delete[] objectID;
	delete[] fileData;
	delete[] textureName;

	terrain->heightMap = newTexture;
	initTerrainEditTools(terrain);
	return newTexture;
}

FETexture* FEResourceManager::LoadFETextureUnmanaged(const char* fileName, std::string Name)
{
	FETexture* newTexture = LoadFETexture(fileName, Name);
	textures.erase(newTexture->getObjectID());
	return newTexture;
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index, std::string Name)
{
	return rawDataToMesh(positions.data(), positions.size(), UV.data(), UV.size(), normals.data(), normals.size(), tangents.data(), tangents.size(), index.data(), index.size(), nullptr, 0, 0, Name);
}

FEMesh* FEResourceManager::rawDataToMesh(float* positions, int posSize,
										 float* UV, int UVSize,
										 float* normals, int normSize,
										 float* tangents, int tanSize,
										 int* indices, int indexSize,
										 float* matIndexs, int matIndexsSize, int matCount,
										 std::string Name)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint indicesBufferID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &indicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	GLuint positionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &positionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, positionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint normalsBufferID;
	// normals
	FE_GL_ERROR(glGenBuffers(1, &normalsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, normalsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint tangentsBufferID;
	// tangents
	FE_GL_ERROR(glGenBuffers(1, &tangentsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, tangentsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint UVBufferID;
	// UV
	FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint materialsIndicesBufferID = -1;
	if (matIndexs != nullptr && matIndexsSize > 1)
	{
		// Material ID
		FE_GL_ERROR(glGenBuffers(1, &materialsIndicesBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, materialsIndicesBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * matIndexsSize, matIndexs, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(5/*FE_MATINDEX*/, 1, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	int vertexType = FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX;
	if (matCount > 1)
	{
		vertexType |= FE_MATINDEX;
	}

	FEMesh* newMesh = createMesh(vaoID, indexSize, vertexType, FEAABB(positions, posSize), Name);
	newMesh->indicesCount = indexSize;
	newMesh->indicesBufferID = indicesBufferID;

	newMesh->positionsCount = posSize;
	newMesh->positionsBufferID = positionsBufferID;

	newMesh->normalsCount = normSize;
	newMesh->normalsBufferID = normalsBufferID;

	newMesh->tangentsCount = tanSize;
	newMesh->tangentsBufferID = tangentsBufferID;

	newMesh->UVCount = UVSize;
	newMesh->UVBufferID = UVBufferID;

	newMesh->materialsIndicesCount = matIndexsSize;
	newMesh->materialsIndicesBufferID = materialsIndicesBufferID;

	newMesh->materialsCount = matCount;

	return newMesh;
}

void FEResourceManager::loadStandardMeshes()
{
	if (meshes.find("84251E6E0D0801363579317R"/*"cube"*/) != meshes.end())
		return;

	std::vector<int> cubeIndices = {
		4, 2, 0, 9, 7, 3, 6, 5,	20,	21,	15,
		22,	10,	12,	18,	8, 1, 19, 4, 17, 2,
		9, 23, 7, 6, 13, 5, 24, 16, 15, 10,
		14, 12, 8, 11, 1
	};

	std::vector<float> cubePositions = {
		1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};

	std::vector<float> cubeNormals = {
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f, 0.0f
	};

	std::vector<float> cubeTangents = {
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
	};

	std::vector<float> cubeUV = {
		0.375f, 1.0f, 0.625f, 0.25f, 0.375f, 0.75f,
		0.375f, 0.75f, 0.625f, 1.0f, 0.375f, 0.25f,
		0.625f, 0.5f, 0.375f, 0.5f, 0.875f, 0.5f,
		0.625f, 0.75f, 0.375f, 0.5f, 0.875f, 0.25f,
		0.125f, 0.25f, 0.625f, 0.25f, 0.375f, 0.25f,
		0.375f, 0.0f, 0.625f, 0.0f, 0.625f, 0.75f,
		0.125f, 0.5f, 0.625f, 0.5f, 0.375f, 0.5f,
		0.625f, 0.25f, 0.375f, 0.25f, 0.625f, 0.5f,
		0.625f, 0.25f
	};

	standardMeshes["84251E6E0D0801363579317R"] = rawDataToMesh(cubePositions, cubeNormals, cubeTangents, cubeUV, cubeIndices, "cube");
	meshes.erase(standardMeshes["84251E6E0D0801363579317R"/*"cube"*/]->getObjectID());
	standardMeshes["84251E6E0D0801363579317R"/*"cube"*/]->setID("84251E6E0D0801363579317R"/*"cube"*/);

	std::vector<int> planeIndices = {
		0, 1, 2, 3, 0, 2
	};

	std::vector<float> planePositions = {
		-1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	std::vector<float> planeNormals = {
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	std::vector<float> planeTangents = {
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f
	};

	std::vector<float> planeUV = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	standardMeshes["1Y251E6E6T78013635793156"] = rawDataToMesh(planePositions, planeNormals, planeTangents, planeUV, planeIndices, "plane");
	meshes.erase(standardMeshes["1Y251E6E6T78013635793156"/*"plane"*/]->getObjectID());
	standardMeshes["1Y251E6E6T78013635793156"/*"plane"*/]->setID("1Y251E6E6T78013635793156"/*"plane"*/);

	standardMeshes["7F251E3E0D08013E3579315F"] = LoadFEMesh((defaultResourcesFolder + "7F251E3E0D08013E3579315F.model").c_str(), "sphere");
	meshes.erase(standardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->getObjectID());
	standardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->setID("7F251E3E0D08013E3579315F"/*"sphere"*/);
}

FEResourceManager::FEResourceManager()
{
	//// checking if we need to change default resource folder because we are in cmake generated project
	//DWORD dwAttrib = GetFileAttributesA(defaultResourcesFolder.c_str());
	//if (!(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)))
	//{
	//	defaultResourcesFolder = "..//Resources//";
	//}

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	noTexture = LoadFETexture((defaultResourcesFolder + "48271F005A73241F5D7E7134.texture").c_str(), "noTexture");
	makeTextureStandard(noTexture);

	loadStandardMaterial();
	loadStandardMeshes();
	loadStandardGameModels();
}

FEResourceManager::~FEResourceManager()
{
	clear();
}

FEMesh* FEResourceManager::LoadOBJMesh(const char* fileName, std::string Name)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.readFile(fileName);

	FEMesh* newMesh = rawDataToMesh(objLoader.fVerC.data(), objLoader.fVerC.size(),
									objLoader.fTexC.data(), objLoader.fTexC.size(),
									objLoader.fNorC.data(), objLoader.fNorC.size(),
									objLoader.fTanC.data(), objLoader.fTanC.size(),
									objLoader.fInd.data(), objLoader.fInd.size(),
									objLoader.matIDs.data(), objLoader.matIDs.size(), objLoader.materialRecords.size(), Name);

	if (Name.size() == 0)
		Name = getFileNameFromFilePath(fileName);

	if (meshes.find(Name) != meshes.end())
		Name = "mesh_" + std::to_string(meshes.size());
	
	// in rawDataToMesh() hidden FEMesh allocation and it will go to hash table so we need to use setMeshName() not setName.
	newMesh->setName(Name);
	meshes[newMesh->getObjectID()] = newMesh;

	return newMesh;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* fileName, std::string Name)
{
	std::fstream file;

	file.open(fileName, std::ios::in | std::ios::binary);
	std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFEMesh.", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardMeshes.size() > 0)
		{
			return getMesh("84251E6E0D0801363579317R"/*"cube"*/);
		}
		else
		{
			return nullptr;
		}
	}

	char* buffer = new char[4];

	// version of FEMesh file type
	file.read(buffer, 4);
	float version = *(float*)buffer;
	if (version != FE_MESH_VERSION)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFEMesh. File was created in different version of engine!", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardMeshes.size() > 0)
		{
			return getMesh("84251E6E0D0801363579317R"/*"cube"*/);
		}
		else
		{
			return nullptr;
		}
	}

	int objectIDSize = 0;
	file.read(buffer, 4);
	objectIDSize = *(int*)buffer;

	char* objectID = new char[objectIDSize + 1];
	file.read(objectID, objectIDSize);
	objectID[objectIDSize] = '\0';

	int meshNameSize = 0;
	file.read(buffer, 4);
	meshNameSize = *(int*)buffer;
	char* meshName = new char[meshNameSize + 1];
	file.read(meshName, meshNameSize);
	meshName[meshNameSize] = '\0';
	
	file.read(buffer, 4);
	int vertexCout = *(int*)buffer;
	char* vertexBuffer = new char[vertexCout * 4];
	file.read(vertexBuffer, vertexCout * 4);

	file.read(buffer, 4);
	int texCout = *(int*)buffer;
	char* texBuffer = new char[texCout * 4];
	file.read(texBuffer, texCout * 4);

	file.read(buffer, 4);
	int normCout = *(int*)buffer;
	char* normBuffer = new char[normCout * 4];
	file.read(normBuffer, normCout * 4);

	file.read(buffer, 4);
	int tangCout = *(int*)buffer;
	char* tangBuffer = new char[tangCout * 4];
	file.read(tangBuffer, tangCout * 4);

	file.read(buffer, 4);
	int indexCout = *(int*)buffer;
	char* indexBuffer = new char[indexCout * 4];
	file.read(indexBuffer, indexCout * 4);

	int matIndexCout = 0;
	char* matIndexBuffer = nullptr;

	file.read(buffer, 4);
	int matCount = *(int*)buffer;

	if (matCount > 1)
	{
		file.read(buffer, 4);
		matIndexCout = *(int*)buffer;
		matIndexBuffer = new char[matIndexCout * 4];
		file.read(matIndexBuffer, matIndexCout * 4);
	}

	FEAABB meshAABB;
	for (size_t i = 0; i <= 2; i++)
	{
		file.read(buffer, 4);
		meshAABB.min[i] = *(float*)buffer;
	}

	for (size_t i = 0; i <= 2; i++)
	{
		file.read(buffer, 4);
		meshAABB.max[i] = *(float*)buffer;
	}

	file.close();

	FEMesh* newMesh = rawDataToMesh((float*)vertexBuffer, vertexCout,
									(float*)texBuffer, texCout,
									(float*)normBuffer, normCout,
									(float*)tangBuffer, tangCout,
									(int*)indexBuffer, indexCout,
									(float*)matIndexBuffer, matIndexCout, matCount,
									Name);

	std::string oldID = newMesh->ID;
	// overwrite objectID with objectID from file.
	if (objectID != nullptr)
	{
		newMesh->setID(objectID);
	}

	delete[] objectID;

	delete[] buffer;
	delete[] vertexBuffer;
	delete[] texBuffer;
	delete[] normBuffer;
	delete[] tangBuffer;
	delete[] indexBuffer;

	newMesh->AABB = meshAABB;
	newMesh->setName(Name);
	
	meshes.erase(oldID);
	meshes[newMesh->getObjectID()] = newMesh;

	return newMesh;
}

FEMaterial* FEResourceManager::createMaterial(std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedMaterial";

	FEMaterial* newMaterial = new FEMaterial(Name);
	if (forceObjectID != "")
		newMaterial->setID(forceObjectID);
	materials[newMaterial->getObjectID()] = newMaterial;
	
	return materials[newMaterial->getObjectID()];
}

FEEntity* FEResourceManager::createEntity(FEGameModel* gameModel, std::string Name, std::string forceObjectID)
{
	if (gameModel == nullptr)
		gameModel = standardGameModels["67251E393508013ZV579315F"];

	FEEntity* newEntity = new FEEntity(gameModel, Name);
	if (forceObjectID != "")
		newEntity->setID(forceObjectID);
	return newEntity;
}

std::vector<std::string> FEResourceManager::getMaterialList()
{
	FE_MAP_TO_STR_VECTOR(materials)
}

std::vector<std::string> FEResourceManager::getStandardMaterialList()
{
	FE_MAP_TO_STR_VECTOR(standardMaterials)
}

FEMaterial* FEResourceManager::getMaterial(std::string ID)
{
	if (materials.find(ID) == materials.end())
	{
		if (standardMaterials.find(ID) != standardMaterials.end())
		{
			return standardMaterials[ID];
		}

		return nullptr;
	}

	return materials[ID];
}

std::string FEResourceManager::getFileNameFromFilePath(std::string filePath)
{
	for (size_t i = filePath.size() - 1; i > 0; i--)
	{
		if (filePath[i] == '\\' || filePath[i] == '/')
			return filePath.substr(i + 1, filePath.size() - 1 - i);
	}

	return std::string("");
}

std::vector<std::string> FEResourceManager::getMeshList()
{
	FE_MAP_TO_STR_VECTOR(meshes)
}

std::vector<std::string> FEResourceManager::getStandardMeshList()
{
	FE_MAP_TO_STR_VECTOR(standardMeshes)
}

FEMesh* FEResourceManager::getMesh(std::string ID)
{
	if (meshes.find(ID) == meshes.end())
	{
		if (standardMeshes.find(ID) != standardMeshes.end())
		{
			return standardMeshes[ID];
		}

		return nullptr;
	}
	else
	{
		return meshes[ID];
	}
}

bool FEResourceManager::makeMaterialStandard(FEMaterial* material)
{
	if (material == nullptr)
	{
		LOG.add("material is nullptr in function FEResourceManager::makeMaterialStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardMaterials.find(material->getObjectID()) == standardMaterials.end())
	{
		if (materials.find(material->getObjectID()) != materials.end())
			materials.erase(material->getObjectID());
		standardMaterials[material->getObjectID()] = material;

		return true;
	}

	return false;
}

void FEResourceManager::loadStandardMaterial()
{
	FEMaterial* newMaterial = createMaterial("SolidColorMaterial");
	materials.erase(newMaterial->getObjectID());
	newMaterial->setID("18251A5E0F08013Z3939317U");
	newMaterial->shader = createShader("FESolidColorShader", loadGLSL("CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_VS.glsl").c_str(),
															 loadGLSL("CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_FS.glsl").c_str());
	newMaterial->shader->setID("6917497A5E0C05454876186F");

	makeShaderStandard(newMaterial->shader);
	FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	newMaterial->addParameter(color);
	makeMaterialStandard(newMaterial);

	FEShader* FEPhongShader = createShader("FEPhongShader", loadGLSL("CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_VS.glsl").c_str(),
															loadGLSL("CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_FS.glsl").c_str());
	shaders.erase(FEPhongShader->getObjectID());
	FEPhongShader->setID("4C41665B5E125C2A07456E44"/*"FEPhongShader"*/);
	shaders[FEPhongShader->getObjectID()] = FEPhongShader;

	makeShaderStandard(getShader("4C41665B5E125C2A07456E44"/*"FEPhongShader"*/));

	// ****************************** PBR SHADER ******************************
#ifdef USE_DEFERRED_RENDERER
	FEShader* FEPBRShader = createShader("FEPBRShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str(),
														loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str());
#else
	FEShader* FEPBRShader = createShader("FEPBRShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str(),
														loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str());
#endif // USE_DEFERRED_RENDERER
	shaders.erase(FEPBRShader->getObjectID());
	FEPBRShader->setID("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	shaders[FEPBRShader->getObjectID()] = FEPBRShader;

	makeShaderStandard(getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/));

#ifdef USE_DEFERRED_RENDERER
	FEShader* FEPBRInstancedShader = createShader("FEPBRInstancedShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str(),
																		  loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str());
#else
	FEShader* FEPBRInstancedShader = createShader("FEPBRInstancedShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str(),
																		  loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str());
#endif // USE_DEFERRED_RENDERER
	shaders.erase(FEPBRInstancedShader->getObjectID());
	FEPBRInstancedShader->setID("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
	shaders[FEPBRInstancedShader->getObjectID()] = FEPBRInstancedShader;

	makeShaderStandard(getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/));

	newMaterial = createMaterial("FEPBRBaseMaterial");
	materials.erase(newMaterial->getObjectID());
	newMaterial->setID("61649B9E0F08013Q3939316C"/*"FEPBRBaseMaterial"*/);
	newMaterial->shader = getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	newMaterial->setAlbedoMap(noTexture);
	makeMaterialStandard(newMaterial);
	// ****************************** PBR SHADER END ******************************

#ifdef USE_DEFERRED_RENDERER
	FEShader* FETerrainShader = createShader("FETerrainShader", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_VS.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_FS_DEFERRED.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TCS.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TES.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_GS.glsl").c_str());
#else
	FEShader* FETerrainShader = createShader("FETerrainShader", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_VS.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_FS.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TCS.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TES.glsl").c_str(),
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_GS.glsl").c_str());
#endif // USE_DEFERRED_RENDERER
	shaders.erase(FETerrainShader->getObjectID());
	FETerrainShader->setID("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	shaders[FETerrainShader->getObjectID()] = FETerrainShader;

	makeShaderStandard(getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/));

	FEShader* FESMTerrainShader = createShader("FESMTerrainShader", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_VS.glsl").c_str(),
																	loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_FS.glsl").c_str(),
																	loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TCS.glsl").c_str(),
																	loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TES.glsl").c_str());
	shaders.erase(FESMTerrainShader->getObjectID());
	FESMTerrainShader->setID("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
	shaders[FESMTerrainShader->getObjectID()] = FESMTerrainShader;

	FEShaderParam colorParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor");
	getShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/)->addParameter(colorParam);

	makeShaderStandard(getShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/));

	FEMaterial* skyDomeMaterial = createMaterial("skyDomeMaterial");
	materials.erase(skyDomeMaterial->getObjectID());
	skyDomeMaterial->setID("5A649B9E0F36073D4939313H");
	skyDomeMaterial->shader = createShader("FESkyDome", loadGLSL("CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_VS.glsl").c_str(),
														loadGLSL("CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_FS.glsl").c_str());
	shaders.erase(skyDomeMaterial->shader->getObjectID());
	skyDomeMaterial->shader->setID("3A69744E831A574E4857361B");
	shaders[skyDomeMaterial->shader->getObjectID()] = skyDomeMaterial->shader;

	makeShaderStandard(skyDomeMaterial->shader);
	makeMaterialStandard(skyDomeMaterial);

	FEShader* terrainBrushOutput = createShader("terrainBrushOutput", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_VS.glsl").c_str(),
																	  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_FS.glsl").c_str());
	shaders.erase(terrainBrushOutput->getObjectID());
	terrainBrushOutput->setID("49654A4A10604C2A1221426B"/*"terrainBrushOutput"*/);
	shaders[terrainBrushOutput->getObjectID()] = terrainBrushOutput;


	makeShaderStandard(getShader("49654A4A10604C2A1221426B"/*"terrainBrushOutput"*/));

	FEShader* terrainBrushVisual = createShader("terrainBrushVisual", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_VS.glsl").c_str(),
																	  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_FS.glsl").c_str());
	shaders.erase(terrainBrushVisual->getObjectID());
	terrainBrushVisual->setID("40064B7B4287805B296E526E"/*"terrainBrushVisual"*/);
	shaders[terrainBrushVisual->getObjectID()] = terrainBrushVisual;

	makeShaderStandard(getShader("40064B7B4287805B296E526E"/*"terrainBrushVisual"*/));

	// same as FERenderer::updateFogInShaders()
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);

	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);

	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);
}

void FEResourceManager::loadStandardGameModels()
{
	FEGameModel* newGameModel = new FEGameModel(getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/), getMaterial("18251A5E0F08013Z3939317U"/*"FESolidColorShader"*/), "standardGameModel");
	newGameModel->setID("67251E393508013ZV579315F");
	makeGameModelStandard(newGameModel);

	newGameModel = new FEGameModel(getMesh("7F251E3E0D08013E3579315F"), getMaterial("5A649B9E0F36073D4939313H"/*"skyDomeMaterial"*/), "skyDomeGameModel");
	newGameModel->setID("17271E603508013IO77931TY");
	makeGameModelStandard(newGameModel);
}

void FEResourceManager::clear()
{
	auto materialIt = materials.begin();
	while (materialIt != materials.end())
	{
		delete materialIt->second;
		materialIt++;
	}
	materials.clear();

	auto meshIt = meshes.begin();
	while (meshIt != meshes.end())
	{
		delete meshIt->second;
		meshIt++;
	}
	meshes.clear();

	auto textureIt = textures.begin();
	while (textureIt != textures.end())
	{
		delete textureIt->second;
		textureIt++;
	}
	textures.clear();

	auto gameModelIt = gameModels.begin();
	while (gameModelIt != gameModels.end())
	{
		delete gameModelIt->second;
		gameModelIt++;
	}
	gameModels.clear();
}

void FEResourceManager::saveFEMesh(FEMesh* Mesh, const char* fileName)
{
	std::fstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	// version of FEMesh file type
	float version = FE_MESH_VERSION;
	file.write((char*)&version, sizeof(float));

	int objectIDSize = Mesh->getObjectID().size();
	file.write((char*)&objectIDSize, sizeof(int));
	file.write((char*)Mesh->getObjectID().c_str(), sizeof(char) * objectIDSize);

	int nameSize = Mesh->getName().size();
	file.write((char*)&nameSize, sizeof(int));
	file.write((char*)Mesh->getName().c_str(), sizeof(char) * nameSize);

	int count = Mesh->getPositionsCount();
	float* positions = new float[count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getPositionsBufferID(), 0, sizeof(float) * count, positions));
	file.write((char*)&count, sizeof(int));
	file.write((char*)positions, sizeof(float) * count);

	count = Mesh->getUVCount();
	float* UV = new float[count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getUVBufferID(), 0, sizeof(float) * count, UV));
	file.write((char*)&count, sizeof(int));
	file.write((char*)UV, sizeof(float) * count);

	count = Mesh->getNormalsCount();
	float* normals = new float[count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getNormalsBufferID(), 0, sizeof(float) * count, normals));
	file.write((char*)&count, sizeof(int));
	file.write((char*)normals, sizeof(float) * count);
	
	count = Mesh->getTangentsCount();
	float* tangents = new float[count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getTangentsBufferID(), 0, sizeof(float) * count, tangents));
	file.write((char*)&count, sizeof(int));
	file.write((char*)tangents, sizeof(float) * count);

	count = Mesh->getIndicesCount();
	int* indices = new int[count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getIndicesBufferID(), 0, sizeof(int) * count, indices));
	file.write((char*)&count, sizeof(int));
	file.write((char*)indices, sizeof(int) * count);

	int materialCount = Mesh->materialsCount;
	file.write((char*)&materialCount, sizeof(int));
	
	if (materialCount > 1)
	{
		count = Mesh->getMaterialsIndicesCount();
		float* matIndices = new float[count];
		FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getMaterialsIndicesBufferID(), 0, sizeof(float) * count, matIndices));
		file.write((char*)&count, sizeof(int));
		file.write((char*)matIndices, sizeof(float) * count);
	}

	FEAABB tempAABB(positions, Mesh->getPositionsCount());
	file.write((char*)&tempAABB.min[0], sizeof(float));
	file.write((char*)&tempAABB.min[1], sizeof(float));
	file.write((char*)&tempAABB.min[2], sizeof(float));

	file.write((char*)&tempAABB.max[0], sizeof(float));
	file.write((char*)&tempAABB.max[1], sizeof(float));
	file.write((char*)&tempAABB.max[2], sizeof(float));

	file.close();

	delete[] positions;
	delete[] UV;
	delete[] normals;
	delete[] tangents;
	delete[] indices;
}

std::vector<std::string> FEResourceManager::getTextureList()
{
	FE_MAP_TO_STR_VECTOR(textures)
}

FETexture* FEResourceManager::getTexture(std::string ID)
{
	if (textures.find(ID) == textures.end())
		return nullptr;

	return textures[ID];
}

void FEResourceManager::deleteFETexture(FETexture* texture)
{
	// looking if this texture is used in some materials
	// to-do: should be done through list of pointers to materials that uses this texture.
	auto materialIterator = materials.begin();
	while (materialIterator != materials.end())
	{
		for (size_t i = 0; i < materialIterator->second->textures.size(); i++)
		{
			if (materialIterator->second->textures[i] == texture)
			{
				materialIterator->second->textures[i] = noTexture;
				break;
			}
		}

		materialIterator++;
	}

	// after we make sure that texture is no more referenced by any material, we can delete it
	textures.erase(texture->getObjectID());

	delete texture;
}

void FEResourceManager::deleteFEMesh(FEMesh* mesh)
{
	// looking if this mesh is used in some gameModels
	// to-do: should be done through list of pointers to gameModels that uses this mesh.
	auto gameModelIterator = gameModels.begin();
	while (gameModelIterator != gameModels.end())
	{
		if (gameModelIterator->second->mesh == mesh)
		{
			gameModelIterator->second->mesh = getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);
		}

		gameModelIterator++;
	}

	meshes.erase(mesh->getObjectID());
	delete mesh;
}

std::vector<std::string> FEResourceManager::getGameModelList()
{
	FE_MAP_TO_STR_VECTOR(gameModels)
}

std::vector<std::string> FEResourceManager::getStandardGameModelList()
{
	FE_MAP_TO_STR_VECTOR(standardGameModels)
}

FEGameModel* FEResourceManager::getGameModel(std::string ID)
{
	if (gameModels.find(ID) == gameModels.end())
	{
		if (standardGameModels.find(ID) != standardGameModels.end())
		{
			return standardGameModels[ID];
		}

		return nullptr;
	}

	return gameModels[ID];
}

FEGameModel* FEResourceManager::createGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedGameModel";

	if (Mesh == nullptr)
		Mesh = getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);

	if (Material == nullptr)
		Material = getMaterial("18251A5E0F08013Z3939317U"/*"FESolidColorShader"*/);

	FEGameModel* newGameModel = new FEGameModel(Mesh, Material, Name);
	if (forceObjectID != "")
	{
		gameModels[forceObjectID] = newGameModel;
		gameModels[forceObjectID]->setID(forceObjectID);
	}
	else
	{
		gameModels[newGameModel->ID] = newGameModel;
	}
	
	gameModels[newGameModel->ID]->setName(Name);
	return gameModels[newGameModel->ID];
}

void FEResourceManager::deleteGameModel(FEGameModel* gameModel)
{
	gameModels.erase(gameModel->getObjectID());
	delete gameModel;
}

bool FEResourceManager::makeGameModelStandard(FEGameModel* gameModel)
{
	if (gameModel == nullptr)
	{
		LOG.add("gameModel is nullptr in function FEResourceManager::makeGameModelStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardGameModels.find(gameModel->getObjectID()) == standardGameModels.end())
	{
		if (gameModels.find(gameModel->getObjectID()) != gameModels.end())
			gameModels.erase(gameModel->getObjectID());
		standardGameModels[gameModel->getObjectID()] = gameModel;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::createShader(std::string shaderName, const char* vertexText, const char* fragmentText,
										  const char* tessControlText, const char* tessEvalText,
										  const char* geometryText, const char* computeText, std::string forceObjectID)
{
	if (shaderName.size() == 0)
		shaderName = "unnamedShader";

	FEShader* newShader = new FEShader(shaderName, vertexText, fragmentText, tessControlText, tessEvalText, geometryText, computeText);
	if (forceObjectID != "")
		newShader->setID(forceObjectID);
	shaders[newShader->getObjectID()] = newShader;
	return newShader;
}

bool FEResourceManager::makeShaderStandard(FEShader* shader)
{
	if (shader == nullptr)
	{
		LOG.add("shader is nullptr in function FEResourceManager::makeShaderStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardShaders.find(shader->getObjectID()) == standardShaders.end())
	{
		if (shaders.find(shader->getObjectID()) != shaders.end())
			shaders.erase(shader->getObjectID());
		standardShaders[shader->getObjectID()] = shader;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::getShader(std::string shaderID)
{
	if (shaders.find(shaderID) == shaders.end())
	{
		if (standardShaders.find(shaderID) != standardShaders.end())
		{
			return standardShaders[shaderID];
		}

		return nullptr;
	}
	else
	{
		return shaders[shaderID];
	}
}

std::vector<std::string> FEResourceManager::getShadersList()
{
	FE_MAP_TO_STR_VECTOR(shaders)
}

std::vector<std::string> FEResourceManager::getStandardShadersList()
{
	FE_MAP_TO_STR_VECTOR(standardShaders)
}

void FEResourceManager::deleteShader(FEShader* shader)
{
	if (shader == nullptr)
		return;

	auto it = materials.begin();
	while (it != materials.end())
	{
		if (it->second->shader->getNameHash() == shader->getNameHash())
			it->second->shader = getShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
		
		it++;
	}

	it = standardMaterials.begin();
	while (it != standardMaterials.end())
	{
		if (it->second->shader->getNameHash() == shader->getNameHash())
			it->second->shader = getShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);

		it++;
	}

	shaders.erase(shader->getObjectID());
	standardShaders.erase(shader->getObjectID());
	delete shader;
}

bool FEResourceManager::replaceShader(std::string oldShaderID, FEShader* newShader)
{
	FEShader* shaderToReplace = getShader(oldShaderID);
	if (shaderToReplace == nullptr)
		return false;

	if (newShader->getName().size() == 0)
		return false;

	if (shaders.find(oldShaderID) != shaders.end())
	{
		*(shaders[oldShaderID]) = *newShader;
		newShader->setID(oldShaderID);
	}
	else if (standardShaders.find(oldShaderID) != standardShaders.end())
	{
		*(standardShaders[oldShaderID]) = *newShader;
		newShader->setID(oldShaderID);
	}

	return true;
}

FETerrain* FEResourceManager::createTerrain(bool createHeightMap, std::string name, std::string forceObjectID)
{
	FETerrain* newTerrain = new FETerrain(name);
	if (forceObjectID != "")
		newTerrain->setID(forceObjectID);

	newTerrain->shader = getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	newTerrain->layer0 = getMaterial("18251A5E0F08013Z3939317U"/*"FESolidColorShader"*/);

	if (createHeightMap)
	{
		//creating blank heightMap
		FETexture* newTexture = createTexture(name + "_heightMap");
		std::vector<unsigned char> rawData;
		rawData.resize(defaultHeighttMapResolution * defaultHeighttMapResolution * sizeof(unsigned short));
		for (size_t i = 0; i < defaultHeighttMapResolution * defaultHeighttMapResolution * sizeof(unsigned short); i++)
		{
			rawData[i] = 0;
		}

		newTexture->width = defaultHeighttMapResolution;
		newTexture->height = defaultHeighttMapResolution;
		newTexture->internalFormat = GL_R16;
		newTexture->magFilter = FE_LINEAR;
		newTexture->fileName = "NULL";

		FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
		//FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, TRUE));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData.data());
		//FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE));

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		newTerrain->heightMapArray.resize(rawData.size() / sizeof(unsigned short));
		for (size_t i = 0; i < newTerrain->heightMapArray.size(); i++)
		{
			newTerrain->heightMapArray[i] = 0.0f;
		}

		glm::vec3 minPoint = glm::vec3(-1.0f, 0.0f, -1.0f);
		glm::vec3 maxPoint = glm::vec3(1.0f, 0.0f, 1.0f);
		newTerrain->AABB = FEAABB(minPoint, maxPoint);
		newTerrain->heightMap = newTexture;

		initTerrainEditTools(newTerrain);
	}

	return newTerrain;
}

void FEResourceManager::initTerrainEditTools(FETerrain* terrain)
{
	if (terrain == nullptr)
	{
		LOG.add("called FEResourceManager::initTerrainEditTools with nullptr terrain", FE_LOG_ERROR, FE_LOG_RENDERING);
		return;
	}

	if (terrain->brushOutputFB != nullptr)
	{
		delete terrain->brushOutputFB;
		terrain->brushOutputFB = nullptr;
	}

	if (terrain->brushVisualFB != nullptr)
	{
		delete terrain->brushVisualFB;
		terrain->brushVisualFB = nullptr;
	}
	
	terrain->brushOutputFB = createFramebuffer(FE_COLOR_ATTACHMENT, 32, 32);
	delete terrain->brushOutputFB->getColorAttachment();
	//terrain->brushOutputFB->setColorAttachment(new FETexture(GL_R16, GL_RED, terrain->heightMap->getWidth(), terrain->heightMap->getHeight()));
	terrain->brushOutputFB->setColorAttachment(terrain->heightMap);

	terrain->brushVisualFB = createFramebuffer(FE_COLOR_ATTACHMENT, terrain->heightMap->getWidth(), terrain->heightMap->getHeight());
	terrain->projectedMap = terrain->brushVisualFB->getColorAttachment();

	terrain->brushOutputShader = getShader("49654A4A10604C2A1221426B"/*"terrainBrushOutput"*/);
	terrain->brushVisualShader = getShader("40064B7B4287805B296E526E"/*"terrainBrushVisual"*/);

	terrain->planeMesh = getMesh("1Y251E6E6T78013635793156"/*"plane"*/);
}

FETexture* FEResourceManager::LoadPNGHeightmap(const char* fileName, FETerrain* terrain, std::string Name)
{
	FETexture* newTexture = createTexture(Name);
	std::vector<unsigned char> rawData;
	unsigned uWidth, uHeight;
	lodepng::decode(rawData, uWidth, uHeight, fileName, LCT_GREY, 16);
	if (rawData.size() == 0)
	{
		delete newTexture;
		LOG.add(std::string("can't read file: ") + fileName + " in function FEResourceManager::LoadHeightmap.", FE_LOG_ERROR, FE_LOG_LOADING);
		return this->noTexture;
	}

	if ((uWidth != 0 && (uWidth & (uWidth - 1)) == 0) && (uWidth != 0 && (uWidth & (uWidth - 1)) == 0))
	{
		// it is power of 2
	}
	else
	{
		delete newTexture;
		LOG.add(std::string("texture has dementions not power of two! file: ") + fileName + " in function FEResourceManager::LoadHeightmap.", FE_LOG_ERROR, FE_LOG_LOADING);
		return this->noTexture;
	}

	newTexture->width = uWidth;
	newTexture->height = uHeight;
	newTexture->internalFormat = GL_R16;
	newTexture->magFilter = FE_LINEAR;
	newTexture->fileName = fileName;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	// lodepng returns data with different bytes order that openGL expects.
	FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, TRUE));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData.data());
	FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE));
	
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	terrain->heightMapArray.resize(rawData.size() / sizeof(unsigned short));
	float max = FLT_MIN;
	float min = FLT_MAX;
	int iterator = 0;
	for (size_t i = 0; i < rawData.size(); i+=2)
	{
		unsigned short temp = *(unsigned short*)(&rawData[i]);
		terrain->heightMapArray[iterator] = temp / float(0xFFFF);

		if (max < terrain->heightMapArray[iterator])
			max = terrain->heightMapArray[iterator];

		if (min > terrain->heightMapArray[iterator])
			min = terrain->heightMapArray[iterator];

		iterator++;
	}

	glm::vec3 minPoint = glm::vec3(-1.0f, min, -1.0f);
	glm::vec3 maxPoint = glm::vec3(1.0f, max, 1.0f);
	terrain->AABB = FEAABB(minPoint, maxPoint);

	if (Name.size() == 0)
	{
		std::string filePath = newTexture->fileName;
		std::size_t index = filePath.find_last_of("/\\");
		std::string newFileName = filePath.substr(index + 1);
		index = newFileName.find_last_of(".");
		std::string fileNameWithOutExtention = newFileName.substr(0, index);
		newTexture->setName(fileNameWithOutExtention);
	}

	if (terrain->heightMap != nullptr)
		deleteFETexture(terrain->heightMap);
	
	terrain->heightMap = newTexture;
	initTerrainEditTools(terrain);

	return newTexture;
}

std::string FEResourceManager::loadGLSL(const char* fileName)
{
	std::string shaderData;
	std::ifstream file(fileName);
	std::string line;
	
	if (file.is_open())
	{
		while (getline(file, line))
		{
			shaderData += line;
			shaderData += "\n";
		}
		file.close();
	}
	else
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::loadGLSL.", FE_LOG_ERROR, FE_LOG_LOADING);
	}

	return shaderData;
}

FETexture* FEResourceManager::createTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool unManaged, std::string Name)
{
	if (Name.size() == 0)
		Name = "unnamedTexture";

	FETexture* newTexture = new FETexture(InternalFormat, Format, Width, Height, Name);
	if (!unManaged)
		textures[newTexture->getObjectID()] = newTexture;

	return newTexture;
}

FEFramebuffer* FEResourceManager::createFramebuffer(int attachments, int Width, int Height, bool HDR)
{
	FEFramebuffer* newFramebuffer = new FEFramebuffer();
	newFramebuffer->colorAttachments.resize(maxColorAttachments);
	for (size_t i = 0; i < size_t(maxColorAttachments); i++)
	{
		newFramebuffer->colorAttachments[i] = nullptr;
	}
	
	newFramebuffer->width = Width;
	newFramebuffer->height = Height;

	FE_GL_ERROR(glGenFramebuffers(1, &newFramebuffer->fbo));
	newFramebuffer->bind();

	if (attachments & FE_COLOR_ATTACHMENT)
	{
		HDR ? newFramebuffer->colorAttachments[0] = createTexture(GL_RGBA16F, GL_RGBA, Width, Height) : newFramebuffer->colorAttachments[0] = new FETexture(Width, Height, freeObjectName(FE_TEXTURE));
		// Allocate the mipmaps
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		newFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newFramebuffer->colorAttachments[0]);
	}

	if (attachments & FE_DEPTH_ATTACHMENT)
	{
		newFramebuffer->depthAttachment = createTexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, Width, Height);
		newFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newFramebuffer->depthAttachment);

		// if only DEPTH_ATTACHMENT
		if (!(attachments & FE_COLOR_ATTACHMENT))
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		newFramebuffer->depthAttachment->bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		newFramebuffer->depthAttachment->unBind();
	}

	if (attachments & FE_STENCIL_ATTACHMENT)
	{
		//to-do: make it correct
		newFramebuffer->stencilAttachment = new FETexture(Width, Height, freeObjectName(FE_TEXTURE));
		newFramebuffer->attachTexture(GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, newFramebuffer->stencilAttachment);
	}

	newFramebuffer->unBind();

	return newFramebuffer;
}

FEPostProcess* FEResourceManager::createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name)
{
	FEPostProcess* newPostProcess = new FEPostProcess(Name);

	newPostProcess->screenWidth = ScreenWidth;
	newPostProcess->screenHeight = ScreenHeight;
	newPostProcess->screenQuad = getMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	newPostProcess->screenQuadShader = getShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	newPostProcess->intermediateFramebuffer = createFramebuffer(FocalEngine::FE_COLOR_ATTACHMENT, ScreenWidth, ScreenHeight);

	// currently postProcess is not using intermediateFramebuffer colorAttachment directly.
	delete newPostProcess->intermediateFramebuffer->colorAttachments[0];
	newPostProcess->intermediateFramebuffer->colorAttachments[0] = nullptr;

	return newPostProcess;
}

std::string FEResourceManager::freeObjectName(FEObjectType objectType)
{
	std::string result = "NULL";
	switch (objectType)
	{
		case FocalEngine::FE_NULL:
		{
			return result;
		}
		case FocalEngine::FE_SHADER:
		{
			size_t nextID = shaders.size() > standardShaders.size() ? shaders.size() : standardShaders.size();
			size_t index = 0;
			result = "shader_" + std::to_string(nextID + index);
			while (shaders.find(result) != shaders.end() || standardShaders.find(result) != standardShaders.end())
			{
				index++;
				result = "shader_" + std::to_string(nextID + index);
			}

			return result;
		}
		case FocalEngine::FE_TEXTURE:
		{
			size_t nextID = textures.size() > standardTextures.size() ? textures.size() : standardTextures.size();
			size_t index = 0;
			result = "texture_" + std::to_string(nextID + index);
			while (textures.find(result) != textures.end() || standardTextures.find(result) != standardTextures.end())
			{
				index++;
				result = "texture_" + std::to_string(nextID + index);
			}
			
			return result;
		}
		case FocalEngine::FE_MESH:
		{
			size_t nextID = meshes.size() > standardMeshes.size() ? meshes.size() : standardMeshes.size();
			size_t index = 0;
			result = "mesh_" + std::to_string(nextID + index);
			while (meshes.find(result) != meshes.end() || standardMeshes.find(result) != standardMeshes.end())
			{
				index++;
				result = "mesh_" + std::to_string(nextID + index);
			}

			return result;
		}
		case FocalEngine::FE_MATERIAL:
		{
			size_t nextID = materials.size() > standardMaterials.size() ? materials.size() : standardMaterials.size();
			size_t index = 0;
			result = "material_" + std::to_string(nextID + index);
			while (materials.find(result) != materials.end() || standardMaterials.find(result) != standardMaterials.end())
			{
				index++;
				result = "material_" + std::to_string(nextID + index);
			}

			return result;
		}
		case FocalEngine::FE_GAMEMODEL:
		{
			size_t nextID = gameModels.size() > standardGameModels.size() ? gameModels.size() : standardGameModels.size();
			size_t index = 0;
			result = "gameModel_" + std::to_string(nextID + index);
			while (gameModels.find(result) != gameModels.end() || standardGameModels.find(result) != standardGameModels.end())
			{
				index++;
				result = "gameModel_" + std::to_string(nextID + index);
			}

			return result;
		}
		case FocalEngine::FE_ENTITY:
		{
			return result;
		}
		case FocalEngine::FE_TERRAIN:
		{
			/*size_t nextID = terrains.size();
			size_t index = 0;
			result = "terrain_" + std::to_string(nextID + index);
			while (terrains.find(result) != terrains.end())
			{
				index++;
				result = "terrain_" + std::to_string(nextID + index);
			}*/

			return result;
		}
		case FocalEngine::FE_ENTITY_INSTANCED:
		{
			return result;
		}
		default:
		{
			return result;
		}	
	}

	return result;
}

FETexture* FEResourceManager::createSameFormatTexture(FETexture* exampleTexture, int differentW, int differentH, bool unManaged, std::string Name)
{
	if (exampleTexture == nullptr)
	{
		LOG.add("FEResourceManager::createSameFormatTexture called with nullptr pointer as exampleTexture", FE_LOG_ERROR, FE_LOG_RENDERING);
		return nullptr;
	}

	if (differentW == 0 && differentH == 0)
		return createTexture(exampleTexture->internalFormat, exampleTexture->format, exampleTexture->width, exampleTexture->height, unManaged, Name);
	
	if (differentW != 0 && differentH == 0)
		return createTexture(exampleTexture->internalFormat, exampleTexture->format, differentW, exampleTexture->height, unManaged, Name);
	
	if (differentW == 0 && differentH != 0)
		return createTexture(exampleTexture->internalFormat, exampleTexture->format, exampleTexture->width, differentH, unManaged, Name);
	
	return createTexture(exampleTexture->internalFormat, exampleTexture->format, differentW, differentH, unManaged, Name);
}

void FEResourceManager::reSaveStandardMeshes()
{
	std::vector<std::string> standardMeshes = getStandardMeshList();
	for (size_t i = 0; i < standardMeshes.size(); i++)
	{
		FEMesh* currentMesh = getMesh(standardMeshes[i]);
		saveFEMesh(currentMesh, (defaultResourcesFolder + currentMesh->getObjectID() + std::string(".model")).c_str());
	}
}

void FEResourceManager::reSaveStandardTextures()
{
	auto it = standardTextures.begin();
	while (it != standardTextures.end())
	{
		saveFETexture(it->second, (defaultResourcesFolder + it->second->getObjectID() + std::string(".texture")).c_str());
		it++;
	}
}

void FEResourceManager::deleteMaterial(FEMaterial* Material)
{
	// looking if this mesh is used in some gameModels
	// to-do: should be done through list of pointers to gameModels that uses this mesh.
	auto gameModelIterator = gameModels.begin();
	while (gameModelIterator != gameModels.end())
	{
		if (gameModelIterator->second->material == Material)
			gameModelIterator->second->material = getMaterial("18251A5E0F08013Z3939317U"/*"FESolidColorShader"*/);

		gameModelIterator++;
	}

	materials.erase(Material->getObjectID());
	delete Material;
}

std::string FEResourceManager::getDefaultResourcesFolder()
{
	return defaultResourcesFolder;
}