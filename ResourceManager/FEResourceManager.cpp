#define STB_IMAGE_IMPLEMENTATION
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

FETexture* FEResourceManager::LoadPNGTexture(const char* fileName, std::string Name)
{
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

	bool usingAlpha = false;
	for (size_t i = 3; i < rawData.size(); i+=4)
	{
		if (rawData[i] != 255)
		{
			usingAlpha = true;
			break;
		}
	}

	FETexture* newTexture = createTexture(Name);
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	int internalFormat = usingAlpha ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

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

void FEResourceManager::saveFETexture(FETexture* texture, const char* fileName)
{
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture->textureID));

	GLint imgSize = 0;
	std::fstream file;

	file.open(fileName, std::ios::out | std::ios::binary);
	// version of FETexture file type
	float version = FE_TEXTURE_VERSION;
	file.write((char*)&version, sizeof(float));

	int objectIDSize = int(texture->getObjectID().size() + 1);
	file.write((char*)&objectIDSize, sizeof(int));
	file.write((char*)texture->getObjectID().c_str(), sizeof(char) * objectIDSize);

	file.write((char*)&texture->width, sizeof(int));
	file.write((char*)&texture->height, sizeof(int));
	file.write((char*)&texture->internalFormat, sizeof(int));

	int nameSize = int(texture->getName().size() + 1);
	file.write((char*)&nameSize, sizeof(int));

	char* textureName = new char[nameSize];
	strcpy_s(textureName, nameSize, texture->getName().c_str());
	file.write((char*)textureName, sizeof(char) * nameSize);

	if (texture->internalFormat == GL_R16 || texture->internalFormat == GL_RED || texture->internalFormat == GL_RGBA)
	{
		size_t dataSize = 0;
		unsigned char* pixels = getFETextureRawData(texture, &dataSize);

		file.write((char*)&dataSize, sizeof(int));
		file.write((char*)pixels, sizeof(char) * dataSize);
		file.close();

		delete[] pixels;
		return;
	}

	int maxDimention = std::max(texture->width, texture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	char** pixelData = new char*[mipCount];

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
				realSize = int(i + 1);
				break;
			}
		}

		int additionalRealSize = 0;
		for (size_t i = imgSize * 2 - 1; i > 0; i--)
		{
			if (additionalTestPixels[i] != '1')
			{
				additionalRealSize = int(i + 1);
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

FETexture* FEResourceManager::rawDataToFETexture(unsigned char* textureData, int width, int height, GLint internalformat, GLenum format, GLenum type)
{
	FETexture* newTexture = createTexture();
	newTexture->width = width;
	newTexture->height = height;

	if (format == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		newTexture->internalFormat = GL_RED;
	}
	else
	{
		newTexture->internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < size_t(width * height * 4); i += 4)
		{
			if (textureData[i] != 255)
			{
				newTexture->internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, format, GL_UNSIGNED_BYTE, textureData);
	
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

	if (format == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), newTexture->internalFormat, newTexture->width, newTexture->height));

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
			FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, int(i), 0, 0, mipW, mipH, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));
			
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
		FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	}
	else
	{
		newTexture = createTexture(textureName);
	}

	newTexture->width = width;
	newTexture->height = height;
	newTexture->internalFormat = internalFormat;

	// Height map should not be loaded by this function
	if (newTexture->internalFormat == GL_R16)
		return nullptr;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));

	if (newTexture->internalFormat == GL_RED || newTexture->internalFormat == GL_RGBA)
	{
		int size = *(int*)(&fileData[currentShift]);
		currentShift += 4;
		
		updateFETextureRawData(newTexture, (unsigned char*)(&fileData[currentShift]));
	}
	else
	{
		
		int maxDimention = std::max(newTexture->width, newTexture->height);
		size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), newTexture->internalFormat, newTexture->width, newTexture->height));

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
				FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, int(i), 0, 0, mipW, mipH, newTexture->internalFormat, size, (void*)(&fileData[currentShift])));

				mipW = mipW / 2;
				mipH = mipH / 2;

				if (mipW <= 0 || mipH <= 0)
					break;
			}

			currentShift += size;
		}
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

	// Height map should not be loaded by this function
	if (newTexture->internalFormat == GL_R16)
		return nullptr;

	if (newTexture->mipEnabled)
	{
		//FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
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

	int size = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	// Reformating terrain from old saves.
	/*for (size_t i = currentShift; i < currentShift + width * height * 2; i += 2)
	{
		*(unsigned short*)(&fileData[i]) += 0xffff * 0.5;
	}*/

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

	updateFETextureRawData(newTexture, (unsigned char*)(&fileData[currentShift]));
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

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

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
	terrain->updateCPUHeightInfo();
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
	return rawDataToMesh(positions.data(), int(positions.size()), UV.data(), int(UV.size()), normals.data(), int(normals.size()), tangents.data(), int(tangents.size()), index.data(), int(index.size()), nullptr, 0, 0, Name);
}

FEMesh* FEResourceManager::rawDataToMesh(float* positions, int posSize,
										 float* UV, int UVSize,
										 float* normals, int normSize,
										 float* tangents, int tanSize,
										 int* indices, int indexSize,
										 float* matIndexs, int matIndexsSize, int matCount,
										 std::string Name)
{
	int vertexType = FE_POSITION | FE_INDEX;

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

	GLuint normalsBufferID = 0;
	if (normals != nullptr)
	{
		vertexType |= FE_NORMAL;
		// normals
		FE_GL_ERROR(glGenBuffers(1, &normalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, normalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint tangentsBufferID = 0;
	if (tangents != nullptr)
	{
		vertexType |= FE_TANGENTS;
		// tangents
		FE_GL_ERROR(glGenBuffers(1, &tangentsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, tangentsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr)
	{
		vertexType |= FE_UV;
		// UV
		FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

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

	//int vertexType = FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX;
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

	standardMeshes["7F251E3E0D08013E3579315F"] = loadFEMesh((defaultResourcesFolder + "7F251E3E0D08013E3579315F.model").c_str(), "sphere");
	meshes.erase(standardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->getObjectID());
	standardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->setID("7F251E3E0D08013E3579315F"/*"sphere"*/);
}

FEResourceManager::FEResourceManager()
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	noTexture = LoadFETexture((defaultResourcesFolder + "48271F005A73241F5D7E7134.texture").c_str(), "noTexture");
	makeTextureStandard(noTexture);
	FETexture::addToNoDeletingList(noTexture->getTextureID());

	loadStandardMaterial();
	loadStandardMeshes();
	loadStandardGameModels();
	loadStandardPrefabs();
}

FEResourceManager::~FEResourceManager()
{
	clear();
}

std::vector<FEObject*> FEResourceManager::importOBJ(const char* fileName, bool forceOneMesh)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.forceOneMesh = forceOneMesh;
	objLoader.readFile(fileName);

	std::vector<FEObject*> result;
	for (size_t i = 0; i < objLoader.loadedObjects.size(); i++)
	{
		std::string name = getFileNameFromFilePath(fileName) + "_" + std::to_string(i);


		result.push_back(rawDataToMesh(objLoader.loadedObjects[i]->fVerC.data(), int(objLoader.loadedObjects[i]->fVerC.size()),
									   objLoader.loadedObjects[i]->fTexC.data(), int(objLoader.loadedObjects[i]->fTexC.size()),
									   objLoader.loadedObjects[i]->fNorC.data(), int(objLoader.loadedObjects[i]->fNorC.size()),
									   objLoader.loadedObjects[i]->fTanC.data(), int(objLoader.loadedObjects[i]->fTanC.size()),
								       objLoader.loadedObjects[i]->fInd.data(), int(objLoader.loadedObjects[i]->fInd.size()),
									   objLoader.loadedObjects[i]->matIDs.data(), int(objLoader.loadedObjects[i]->matIDs.size()), int(objLoader.loadedObjects[i]->materialRecords.size()), name));
	
	
		// in rawDataToMesh() hidden FEMesh allocation and it will go to hash table so we need to use setMeshName() not setName.
		result.back()->setName(name);
		meshes[result.back()->getObjectID()] = reinterpret_cast<FEMesh*>(result.back());
	}

	createMaterialsFromOBJData(result);

	return result;
}

FEMesh* FEResourceManager::loadFEMesh(const char* fileName, std::string Name)
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
	for (int i = 0; i <= 2; i++)
	{
		file.read(buffer, 4);
		meshAABB.min[i] = *(float*)buffer;
	}

	for (int i = 0; i <= 2; i++)
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

	FEPrefab* tempPrefab = createPrefab(gameModel, gameModel->getName());
	FEEntity* newEntity = new FEEntity(tempPrefab, Name);
	if (forceObjectID != "")
		newEntity->setID(forceObjectID);
	return newEntity;
}

FEEntity* FEResourceManager::createEntity(FEPrefab* prefab, std::string Name, std::string forceObjectID)
{
	if (prefab == nullptr)
		prefab = new FEPrefab(standardGameModels["67251E393508013ZV579315F"], Name);

	FEEntity* newEntity = new FEEntity(prefab, Name);
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

std::vector<FEMaterial*> FEResourceManager::getMaterialByName(std::string Name)
{
	std::vector<FEMaterial*> result;

	auto it = materials.begin();
	while (it != materials.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	it = standardMaterials.begin();
	while (it != standardMaterials.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
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

std::vector<FEMesh*> FEResourceManager::getMeshByName(std::string Name)
{
	std::vector<FEMesh*> result;

	auto it = meshes.begin();
	while (it != meshes.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	it = standardMeshes.begin();
	while (it != standardMeshes.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
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
	newMaterial->setID("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);
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
	FEShader* FEPBRShader = createShader("FEPBRShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS_GBUFFER.glsl").c_str(),
														loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str());

	FEShader* FEPBRShaderForward = createShader("FEPBRShaderForward", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str(),
																	  loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str());

	shaders.erase(FEPBRShaderForward->getObjectID());
	FEPBRShaderForward->setID("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
	shaders[FEPBRShaderForward->getObjectID()] = FEPBRShaderForward;

	makeShaderStandard(getShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/));

	FEShader* FEPBRGBufferShader = createShader("FEPBRGBufferShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str(),
															          loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str());

	shaders.erase(FEPBRGBufferShader->getObjectID());
	FEPBRGBufferShader->setID("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/);
	shaders[FEPBRGBufferShader->getObjectID()] = FEPBRGBufferShader;

	makeShaderStandard(getShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));

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

	FEShader* FEPBRInstancedGBufferShader = createShader("FEPBRInstancedGBufferShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str(),
																						loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str());

	shaders.erase(FEPBRInstancedGBufferShader->getObjectID());
	FEPBRInstancedGBufferShader->setID("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/);
	shaders[FEPBRInstancedGBufferShader->getObjectID()] = FEPBRInstancedGBufferShader;

	makeShaderStandard(getShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/)); 
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
																loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_FS_GBUFFER.glsl").c_str(),
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
	// ****************************** TERRAIN ******************************
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

	FEShader* terrainLayersNormalize = createShader("terrainLayersNormalize", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_VS.glsl").c_str(),
																			  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_LayersNormalize_FS.glsl").c_str());
	shaders.erase(terrainLayersNormalize->getObjectID());
	terrainLayersNormalize->setID("19294C00394A346A576F401C"/*"terrainLayersNormalize"*/);
	shaders[terrainLayersNormalize->getObjectID()] = terrainLayersNormalize;
	makeShaderStandard(getShader("19294C00394A346A576F401C"/*"terrainLayersNormalize"*/));
	// ****************************** TERRAIN END ******************************

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

	// same as FERenderer::updateFogInShaders()
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);

	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);

#ifdef USE_DEFERRED_RENDERER
	
#else
	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogDensity")->updateData(0.007f);
	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogGradient")->updateData(2.5f);
	getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("shadowBlurFactor")->updateData(1.0f);
#endif // USE_DEFERRED_RENDERER
	
}

void FEResourceManager::loadStandardGameModels()
{
	FEGameModel* newGameModel = new FEGameModel(getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/), getMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/), "standardGameModel");
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


	auto prefabIt = prefabs.begin();
	while (prefabIt != prefabs.end())
	{
		delete prefabIt->second;
		prefabIt++;
	}
	prefabs.clear();
}

void FEResourceManager::saveFEMesh(FEMesh* Mesh, const char* fileName)
{
	std::fstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	// version of FEMesh file type
	float version = FE_MESH_VERSION;
	file.write((char*)&version, sizeof(float));

	int objectIDSize = int(Mesh->getObjectID().size());
	file.write((char*)&objectIDSize, sizeof(int));
	file.write((char*)Mesh->getObjectID().c_str(), sizeof(char) * objectIDSize);

	int nameSize = int(Mesh->getName().size());
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

std::vector<FETexture*> FEResourceManager::getTextureByName(std::string Name)
{
	std::vector<FETexture*> result;

	auto it = textures.begin();
	while (it != textures.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
}

void FEResourceManager::deleteFETexture(FETexture* texture)
{
	if (texture == noTexture)
	{
		LOG.add(std::string("Attempt to delete noTexture texture in function FEResourceManager::deleteFETexture.", FE_LOG_WARNING, FE_LOG_GENERAL));
		return;
	}

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

std::vector<FEGameModel*> FEResourceManager::getGameModelByName(std::string Name)
{
	std::vector<FEGameModel*> result;

	auto it = gameModels.begin();
	while (it != gameModels.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	it = standardGameModels.begin();
	while (it != standardGameModels.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
}

FEGameModel* FEResourceManager::createGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedGameModel";

	if (Mesh == nullptr)
		Mesh = getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);

	if (Material == nullptr)
		Material = getMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

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

	// Shader with compute stage cannot contain any other stage.
	if (computeText != nullptr && (vertexText != nullptr || fragmentText != nullptr ||
								   tessControlText != nullptr || tessEvalText != nullptr ||
								   geometryText != nullptr))
	{
		return nullptr;
	}

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

std::vector<FEShader*> FEResourceManager::getShaderByName(std::string Name)
{
	std::vector<FEShader*> result;

	auto it = shaders.begin();
	while (it != shaders.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	it = standardShaders.begin();
	while (it != standardShaders.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
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
		//*(shaders[oldShaderID]) = *newShader;
		//newShader->setID(oldShaderID);
		shaders[oldShaderID]->reCompile(newShader->getName(), newShader->getVertexShaderText(), newShader->getFragmentShaderText(),
										newShader->getTessControlShaderText(), newShader->getTessEvalShaderText(),
										newShader->getGeometryShaderText(), newShader->getComputeShaderText());
	}
	else if (standardShaders.find(oldShaderID) != standardShaders.end())
	{
		//*(standardShaders[oldShaderID]) = *newShader;
		//newShader->setID(oldShaderID);

		standardShaders[oldShaderID]->reCompile(newShader->getName(), newShader->getVertexShaderText(), newShader->getFragmentShaderText(),
												newShader->getTessControlShaderText(), newShader->getTessEvalShaderText(),
												newShader->getGeometryShaderText(), newShader->getComputeShaderText());
	}

	return true;
}

FETerrain* FEResourceManager::createTerrain(bool createHeightMap, std::string name, std::string forceObjectID)
{
	FETerrain* newTerrain = new FETerrain(name);
	if (forceObjectID != "")
		newTerrain->setID(forceObjectID);

	newTerrain->shader = getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);

	if (createHeightMap)
	{
		//creating blank heightMap
		FETexture* newTexture = createTexture(name + "_heightMap");
		newTexture->width = FE_TERRAIN_STANDARD_HIGHT_MAP_RESOLUTION;
		newTexture->height = FE_TERRAIN_STANDARD_HIGHT_MAP_RESOLUTION;
		newTexture->internalFormat = GL_R16;
		newTexture->magFilter = FE_LINEAR;
		newTexture->fileName = "NULL";

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));

		unsigned short* rawPixels = new unsigned short[newTexture->width * newTexture->height];
		for (size_t i = 0; i < size_t(newTexture->width * newTexture->height); i++)
		{
			rawPixels[i] = unsigned short(0xffff * 0.5);
		}

		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, (unsigned char*)rawPixels);
		delete[] rawPixels;

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		newTerrain->heightMapArray.resize(newTexture->width * newTexture->height);
		for (size_t i = 0; i < newTerrain->heightMapArray.size(); i++)
		{
			newTerrain->heightMapArray[i] = 0.5f;
		}

		glm::vec3 minPoint = glm::vec3(-1.0f, 0.5f, -1.0f);
		glm::vec3 maxPoint = glm::vec3(1.0f, 0.5f, 1.0f);
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
	terrain->layersNormalizeShader = getShader("19294C00394A346A576F401C"/*"terrainLayersNormalize"*/);
	terrain->brushVisualShader = getShader("40064B7B4287805B296E526E"/*"terrainBrushVisual"*/);

	terrain->planeMesh = getMesh("1Y251E6E6T78013635793156"/*"plane"*/);
}

FETexture* FEResourceManager::LoadPNGHeightmap(const char* fileName, FETerrain* terrain, std::string Name)
{
	FETexture* newTexture = createTexture(Name);

	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	char* fileData = new char[int(fileSize)];
	file.read(fileData, fileSize);
	file.close();

	unsigned uWidth, uHeight;
	lodepng::State pngState;
	pngState.info_raw.colortype = LCT_GREY;
	pngState.info_raw.bitdepth = 16;
	std::vector<unsigned char> rawData;
	lodepng::decode(rawData, uWidth, uHeight, pngState, (unsigned char*)fileData, int(fileSize));

	if (pngState.info_png.color.colortype != LCT_GREY || pngState.info_png.color.bitdepth != 16)
	{
		delete newTexture;
		LOG.add(std::string("File: ") + fileName + " in function FEResourceManager::LoadPNGHeightmap is not 16 bit gray scale png.", FE_LOG_ERROR, FE_LOG_LOADING);
		return this->noTexture;
	}
		
	if (rawData.size() == 0)
	{
		delete newTexture;
		LOG.add(std::string("can't read file: ") + fileName + " in function FEResourceManager::LoadPNGHeightmap.", FE_LOG_ERROR, FE_LOG_LOADING);
		return this->noTexture;
	}

	newTexture->width = uWidth;
	newTexture->height = uHeight;
	newTexture->internalFormat = GL_R16;
	newTexture->magFilter = FE_LINEAR;
	newTexture->fileName = fileName;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	// lodepng returns data with different bytes order that openGL expects.
	FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, TRUE));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData.data());
	FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE));
	
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

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
	terrain->updateCPUHeightInfo();

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
		HDR ? newFramebuffer->setColorAttachment(createTexture(GL_RGBA16F, GL_RGBA, Width, Height)) : newFramebuffer->setColorAttachment(new FETexture(Width, Height, freeObjectName(FE_TEXTURE)));
	}

	if (attachments & FE_DEPTH_ATTACHMENT)
	{
		newFramebuffer->setDepthAttachment(createTexture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, Width, Height));

		// if only DEPTH_ATTACHMENT
		if (!(attachments & FE_COLOR_ATTACHMENT))
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		newFramebuffer->getDepthAttachment()->bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		newFramebuffer->getDepthAttachment()->unBind();
	}

	if (attachments & FE_STENCIL_ATTACHMENT)
	{
		//to-do: make it correct
		newFramebuffer->setStencilAttachment(new FETexture(Width, Height, freeObjectName(FE_TEXTURE)));
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
	// currently postProcess is not using intermediateFramebuffer colorAttachment directly.
	newPostProcess->intermediateFramebuffer = createFramebuffer(0, ScreenWidth, ScreenHeight);

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
			gameModelIterator->second->material = getMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

		gameModelIterator++;
	}

	materials.erase(Material->getObjectID());
	delete Material;
}

std::string FEResourceManager::getDefaultResourcesFolder()
{
	return defaultResourcesFolder;
}

std::vector<FETexture*> FEResourceManager::channelsToFETextures(FETexture* sourceTexture)
{
	std::vector<FETexture*> result;

	size_t textureDataLenght = 0;
	unsigned char* pixels = getFETextureRawData(sourceTexture, &textureDataLenght);

	unsigned char* redChannel = new unsigned char[size_t(textureDataLenght / 4.0f)];
	int index = 0;
	for (size_t i = 0; i < textureDataLenght; i += 4)
	{
		redChannel[index++] = pixels[i];
	}

	unsigned char* greenChannel = new unsigned char[size_t(textureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 1; i < textureDataLenght; i += 4)
	{
		greenChannel[index++] = pixels[i];
	}

	unsigned char* blueChannel = new unsigned char[size_t(textureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 2; i < textureDataLenght; i += 4)
	{
		blueChannel[index++] = pixels[i];
	}

	unsigned char* alphaChannel = new unsigned char[size_t(textureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 3; i < textureDataLenght; i += 4)
	{
		alphaChannel[index++] = pixels[i];
	}

	result.push_back(rawDataToFETexture(redChannel, sourceTexture->getWidth(), sourceTexture->getHeight(), GL_RED, GL_RED));
	result.back()->setName(sourceTexture->getName() + "_R");

	result.push_back(rawDataToFETexture(greenChannel, sourceTexture->getWidth(), sourceTexture->getHeight(), GL_RED, GL_RED));
	result.back()->setName(sourceTexture->getName() + "_G");

	result.push_back(rawDataToFETexture(blueChannel, sourceTexture->getWidth(), sourceTexture->getHeight(), GL_RED, GL_RED));
	result.back()->setName(sourceTexture->getName() + "_B");

	result.push_back(rawDataToFETexture(alphaChannel, sourceTexture->getWidth(), sourceTexture->getHeight(), GL_RED, GL_RED));
	result.back()->setName(sourceTexture->getName() + "_A");

	delete[] pixels;
	delete[] redChannel;
	delete[] greenChannel;
	delete[] blueChannel;
	delete[] alphaChannel;

	return result;
}

void FEResourceManager::updateAsyncLoadedResources()
{
	static int totalJobs = 0;
	for (size_t i = 0; i < JOB_MANAGER.textureLoadJobs.size(); i++)
	{
		size_t count = JOB_MANAGER.textureLoadJobs[i]->getReadyJobCount();
		if (count == 0)
			continue;

		if (JOB_MANAGER.textureLoadJobs[i]->beginJobsUpdate())
		{
			totalJobs += int(count);
			for (size_t j = 0; j < count; j++)
			{
				std::pair<char**, void*> jobInfo = JOB_MANAGER.textureLoadJobs[i]->getJobByIndex(j);
				// File was not found, or it can't be read.
				if (*jobInfo.first == nullptr)
				{
					// Get info about problematic texture.
					FETexture* notLoadedTexture = reinterpret_cast<FETexture*>(jobInfo.second);
					// We will spill out error into a log.
					LOG.add("FEResourceManager::updateAsyncLoadedResources texture with ID: " + notLoadedTexture->getObjectID() + " was not loaded!", FE_LOG_ERROR, FE_LOG_LOADING);
					// And delete entry for that texture in a general list of textures.
					// That will prevent it from saving in a scene file.
					deleteFETexture(notLoadedTexture);
					//textures.erase(notLoadedTexture->getObjectID());
				}
				else
				{
					FETexture* newlyCreatedTexture = LoadFETexture(*jobInfo.first, "", reinterpret_cast<FETexture*>(jobInfo.second));

					// If some material uses this texture we should set dirty flag.
					// Game model will updated as a consequences.
					std::vector<std::string> materialList = getMaterialList();

					for (size_t i = 0; i < materialList.size(); i++)
					{
						FEMaterial* currentMaterial = getMaterial(materialList[i]);
						if (currentMaterial->isTextureInList(newlyCreatedTexture))
							currentMaterial->setDirtyFlag(true);
					}
				}
			}

			JOB_MANAGER.textureLoadJobs[i]->clearJobs();
			JOB_MANAGER.textureLoadJobs[i]->endJobsUpdate();
		}

		// a bit waste of time, after each batch of textures loads just set dirtyFlag to recreate all materials preview
		std::vector<std::string> materialList = getMaterialList();
		for (size_t i = 0; i < materialList.size(); i++)
		{
			getMaterial(materialList[i])->setDirtyFlag(true);
		}
	}

	int jobsToHandle = int(JOB_MANAGER.textureListToLoad.size());
	int freeThreadCount = JOB_MANAGER.getFreeTextureThreadCount();
	if (freeThreadCount == 0 || jobsToHandle == 0)
		return;

	size_t jobsPerThread = jobsToHandle / freeThreadCount;

	if (JOB_MANAGER.textureListToLoad.size() == 0)
		return;

	for (size_t i = 0; i < JOB_MANAGER.textureLoadJobs.size(); i++)
	{
		if (JOB_MANAGER.textureLoadJobs[i]->beginJobsUpdate())
		{
			size_t lastElement = jobsPerThread < JOB_MANAGER.textureListToLoad.size() ? jobsPerThread : JOB_MANAGER.textureListToLoad.size();
			if (jobsPerThread == 0)
				lastElement = JOB_MANAGER.textureListToLoad.size();
			for (size_t j = 0; j < lastElement; j++)
			{
				JOB_MANAGER.textureLoadJobs[i]->addTextureToLoad(JOB_MANAGER.textureListToLoad[j]);
			}

			std::vector<std::pair<std::string, void*>>& list = JOB_MANAGER.textureListToLoad;
			JOB_MANAGER.textureListToLoad.erase(JOB_MANAGER.textureListToLoad.begin(), JOB_MANAGER.textureListToLoad.begin() + lastElement);
			JOB_MANAGER.textureLoadJobs[i]->endJobsUpdate();

			/*for (size_t j = 0; j < JOB_MANAGER.textureListToLoad.size(); j++)
			{
				JOB_MANAGER.textureLoadJobs[i]->addTextureToLoad(JOB_MANAGER.textureListToLoad[j]);
			}
			JOB_MANAGER.textureListToLoad.clear();
			JOB_MANAGER.textureLoadJobs[i]->endJobsUpdate();

			break;*/
		}
	}
}

void FEResourceManager::activateTerrainVacantLayerSlot(FETerrain* terrain, FEMaterial* material)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::activateTerrainVacantLayerSlot with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}
	
	// If this terrain does not have layerMaps we would create them.
	if (terrain->layerMaps[0] == nullptr)
	{
		FETexture* newTexture = createTexture();
		newTexture->width = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		newTexture->height = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		newTexture->internalFormat = GL_RGBA;

		std::vector<unsigned char> rawData;
		size_t dataLenght = newTexture->getWidth() * newTexture->getHeight() * 4;
		rawData.resize(dataLenght);
		for (size_t i = 0; i < dataLenght; i++)
		{
			rawData[i] = 0;
		}

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());

		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

		terrain->layerMaps[0] = newTexture;

		newTexture = createTexture();
		newTexture->width = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		newTexture->height = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		newTexture->internalFormat = GL_RGBA;

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());

		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

		terrain->layerMaps[1] = newTexture;

		FETerrainLayer* newlyCreatedLayer = terrain->activateVacantLayerSlot(material);
		fillTerrainLayerMask(terrain, 0);
		return;
	}

	FETerrainLayer* newlyCreatedLayer = terrain->activateVacantLayerSlot(material);
}

void FEResourceManager::fillTerrainLayerMaskWithRawData(unsigned char* rawData, FETerrain* terrain, size_t layerIndex)
{
	if (rawData == nullptr)
	{
		LOG.add("FEResourceManager::fillTerrainLayerMaskWithRawData with nullptr rawData", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::fillTerrainLayerMaskWithRawData with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	int index = 0;
	size_t textureWidht = terrain->layerMaps[0]->getWidth();
	size_t textureHeight = terrain->layerMaps[0]->getHeight();

	std::vector<unsigned char*> layersPerTextureData;
	layersPerTextureData.resize(2);
	layersPerTextureData[0] = getFETextureRawData(terrain->layerMaps[0]); 
	layersPerTextureData[1] = getFETextureRawData(terrain->layerMaps[1]);

	std::vector<unsigned char*> layersPerChannelData;
	layersPerChannelData.resize(FE_TERRAIN_MAX_LAYERS);
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		layersPerChannelData[i] = new unsigned char[textureWidht * textureHeight];
	}

	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		index = 0;
		if (layerIndex == i)
		{
			for (size_t j = 0; j < textureWidht * textureHeight; j++)
			{
				layersPerChannelData[i][index++] = rawData[j];
			}
		}
		else
		{
			for (size_t j = i % FE_TERRAIN_LAYER_PER_TEXTURE; j < textureWidht * textureHeight * 4; j += 4)
			{
				layersPerChannelData[i][index++] = layersPerTextureData[i / FE_TERRAIN_LAYER_PER_TEXTURE][j];
			}
		}
	}

	std::vector<unsigned char*> finalTextureChannels;
	finalTextureChannels.resize(2);
	finalTextureChannels[0] = new unsigned char[textureWidht * textureHeight * 4];
	finalTextureChannels[1] = new unsigned char[textureWidht * textureHeight * 4];

	index = 0;

	int* allChannelsPixels = new int[8];

	for (size_t i = 0; i < textureWidht * textureHeight * 4; i += 4)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < 8; j++)
		{
			allChannelsPixels[j] = layersPerChannelData[j][index];
		}

		//int amountOfOverIntansity = sum - 255;
		//for (size_t j = 0; j < 8; j++)
		//{
		//	if (j == layerIndex)
		//		continue;

		//	allChannelsPixels[j] -= allChannelsPixels[layerIndex];
		//	if (allChannelsPixels[j] < 0)
		//		allChannelsPixels[j] = 0;
		//}

		//sum = 0.0f;
		//for (size_t j = 0; j < 8; j++)
		//{
		//	sum += allChannelsPixels[j];
		//}

		//sum /= 255;
		//if (sum < 1.0f)
		//{
		//	allChannelsPixels[0] += int((1.0f - sum) * 255);
		//}

		finalTextureChannels[0][i] = unsigned char(allChannelsPixels[0]);
		finalTextureChannels[0][i + 1] = unsigned char(allChannelsPixels[1]);
		finalTextureChannels[0][i + 2] = unsigned char(allChannelsPixels[2]);
		finalTextureChannels[0][i + 3] = unsigned char(allChannelsPixels[3]);

		finalTextureChannels[1][i] = unsigned char(allChannelsPixels[4]);
		finalTextureChannels[1][i + 1] = unsigned char(allChannelsPixels[5]);
		finalTextureChannels[1][i + 2] = unsigned char(allChannelsPixels[6]);
		finalTextureChannels[1][i + 3] = unsigned char(allChannelsPixels[7]);

		index++;
	}

	int maxDimention = std::max(int(textureWidht), int(textureHeight));
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);

	updateFETextureRawData(terrain->layerMaps[0], finalTextureChannels[0], mipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	updateFETextureRawData(terrain->layerMaps[1], finalTextureChannels[1], mipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	delete[] layersPerTextureData[0];
	delete[] layersPerTextureData[1];
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete[]layersPerChannelData[i];
	}

	delete[] finalTextureChannels[0];
	delete[] finalTextureChannels[1];
	delete[] allChannelsPixels;
}

void FEResourceManager::loadTerrainLayerMask(const char* fileName, FETerrain* terrain, size_t layerIndex)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (terrain->layers[layerIndex] == nullptr)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask on indicated layer slot layer is nullptr", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	// Reading data from file.
	std::vector<unsigned char> rawData;
	unsigned uWidth, uHeight;
	lodepng::decode(rawData, uWidth, uHeight, fileName);

	if (rawData.size() == 0)
	{
		LOG.add(std::string("can't read file: ") + fileName + " in function FEResourceManager::loadTerrainLayerMask.", FE_LOG_ERROR, FE_LOG_LOADING);
		return;
	}

	// It should be just ordinary png not gray scale.
	if (rawData.size() != uWidth * uHeight * 4)
	{
		LOG.add(std::string("can't use file: ") + fileName + " in function FEResourceManager::loadTerrainLayerMask as a mask.", FE_LOG_ERROR, FE_LOG_LOADING);
		return;
	}

	// If new texture have different resolution.
	FETexture* firstLayerMap = terrain->layerMaps[0];
	if (uWidth != firstLayerMap->getWidth() || uHeight != firstLayerMap->getHeight())
	{
		bool needToResizeMaskTexture = false;
		// Firstly we check if current masks has any data.
		std::vector<unsigned char*> layersPerTextureData;
		layersPerTextureData.resize(2);
		layersPerTextureData[0] = getFETextureRawData(terrain->layerMaps[0]);
		layersPerTextureData[1] = getFETextureRawData(terrain->layerMaps[1]);

		// We fill first layer by default so we should check it differently
		unsigned char firstValue = layersPerTextureData[0][0];
		for (size_t i = 0; i < size_t(terrain->layerMaps[0]->getWidth() * terrain->layerMaps[0]->getHeight()); i+=4)
		{
			if (layersPerTextureData[0][i] != firstValue || layersPerTextureData[0][i + 1] != 0 ||
				layersPerTextureData[0][i + 2] != 0 || layersPerTextureData[0][i + 3] != 0 ||
				layersPerTextureData[1][i] != 0 || layersPerTextureData[1][i + 1] != 0 ||
				layersPerTextureData[1][i + 2] != 0 || layersPerTextureData[1][i + 3] != 0)
			{
				needToResizeMaskTexture = true;
				break;
			}
		}

		if (needToResizeMaskTexture)
		{
			LOG.add("FEResourceManager::loadTerrainLayerMask resizing loaded mask to match currently used one.", FE_LOG_WARNING, FE_LOG_LOADING);
			unsigned char* newRawData = resizeTextureRawData(rawData.data(), uWidth, uHeight, firstLayerMap->getWidth(), firstLayerMap->getHeight(), GL_RGBA, 1);
			if (newRawData == nullptr)
			{
				LOG.add("FEResourceManager::loadTerrainLayerMask resizing loaded mask failed.", FE_LOG_ERROR, FE_LOG_LOADING);
				return;
			}

			rawData.clear();

			for (size_t i = 0; i < size_t(firstLayerMap->getWidth() * firstLayerMap->getHeight() * 4); i++)
			{
				rawData.push_back(newRawData[i]);
			}

			delete[] newRawData;
		}
		else
		{
			LOG.add("FEResourceManager::loadTerrainLayerMask resizing terrainLayerMap to match currently loaded one.", FE_LOG_WARNING, FE_LOG_LOADING);

			FETexture* newTexture = createTexture();
			newTexture->width = uWidth;
			newTexture->height = uHeight;
			newTexture->internalFormat = GL_RGBA;

			std::vector<unsigned char> rawData;
			size_t dataLenght = newTexture->getWidth() * newTexture->getHeight() * 4;
			rawData.resize(dataLenght);
			for (size_t i = 0; i < dataLenght; i++)
			{
				rawData[i] = 0;
			}

			FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
			FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());

			FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

			deleteFETexture(terrain->layerMaps[0]);
			terrain->layerMaps[0] = newTexture;
			firstLayerMap = terrain->layerMaps[0];

			newTexture = createTexture();
			newTexture->width = uWidth;
			newTexture->height = uHeight;
			newTexture->internalFormat = GL_RGBA;

			FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
			FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());

			FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

			deleteFETexture(terrain->layerMaps[1]);
			terrain->layerMaps[1] = newTexture;
		}
	}

	unsigned char* filledChannel = new unsigned char[firstLayerMap->getWidth() * firstLayerMap->getHeight()];
	int index = 0;
	for (size_t i = 0; i < size_t(firstLayerMap->getWidth() * firstLayerMap->getHeight() * 4); i += 4)
	{
		filledChannel[index++] = rawData[i];
	}

	fillTerrainLayerMaskWithRawData(filledChannel, terrain, layerIndex);
}

void FEResourceManager::saveTerrainLayerMask(const char* fileName, FETerrain* terrain, size_t layerIndex)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (terrain->layers[layerIndex] == nullptr)
	{
		LOG.add("FEResourceManager::loadTerrainLayerMask on indicated layer slot layer is nullptr", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	// Reading data from current layer map texture.
	size_t resultingTextureDataLenght = 0;
	FETexture* correctLayer = terrain->layerMaps[layerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	unsigned char* rawData = getFETextureRawData(correctLayer, &resultingTextureDataLenght);
	unsigned char* resultingData = new unsigned char[resultingTextureDataLenght];

	for (size_t i = 0; i < resultingTextureDataLenght; i += 4)
	{
		size_t index = i + layerIndex;
		resultingData[i] = rawData[index];
		resultingData[i + 1] = rawData[index];
		resultingData[i + 2] = rawData[index];
		resultingData[i + 3] = 255;
	}

	exportRawDataToPNG(fileName, resultingData, correctLayer->getWidth(), correctLayer->getHeight(), GL_RGBA);
}

void FEResourceManager::fillTerrainLayerMask(FETerrain* terrain, size_t layerIndex)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::fillTerrainLayerMask with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::fillTerrainLayerMask with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (terrain->layers[layerIndex] == nullptr)
	{
		LOG.add("FEResourceManager::fillTerrainLayerMask on indicated layer slot layer is nullptr", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	FETexture* correctLayer = terrain->layerMaps[layerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	size_t textureWidht = correctLayer->getWidth();
	size_t textureHeight = correctLayer->getHeight();
	unsigned char* filledChannel = new unsigned char[textureWidht * textureHeight];
	for (size_t i = 0; i < textureWidht * textureHeight; i++)
	{
		filledChannel[i] = 255;
	}

	fillTerrainLayerMaskWithRawData(filledChannel, terrain, layerIndex);
	delete[] filledChannel;
	
}

void FEResourceManager::clearTerrainLayerMask(FETerrain* terrain, size_t layerIndex)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::clearTerrainLayerMask with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::clearTerrainLayerMask with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (terrain->layers[layerIndex] == nullptr)
	{
		LOG.add("FEResourceManager::clearTerrainLayerMask on indicated layer slot layer is nullptr", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	FETexture* correctLayer = terrain->layerMaps[layerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	size_t textureWidht = correctLayer->getWidth();
	size_t textureHeight = correctLayer->getHeight();
	unsigned char* filledChannel = new unsigned char[textureWidht * textureHeight];
	for (size_t i = 0; i < textureWidht * textureHeight; i++)
	{
		filledChannel[i] = 0;
	}

	fillTerrainLayerMaskWithRawData(filledChannel, terrain, layerIndex);
	delete[] filledChannel;
}

bool FEResourceManager::exportFETextureToPNG(FETexture* textureToExport, const char* fileName)
{
	if (textureToExport == nullptr)
	{
		LOG.add("FEResourceManager::exportFETextureToPNG with nullptr textureToExport", FE_LOG_ERROR, FE_LOG_SAVING);
		return false;
	}

	if (textureToExport->internalFormat != GL_RGBA &&
		textureToExport->internalFormat != GL_RED &&
		textureToExport->internalFormat != GL_R16 &&
		textureToExport->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		textureToExport->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::exportFETextureToPNG internalFormat of textureToExport is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return false;
	}

	std::vector<unsigned char> rawData;
	if (textureToExport->internalFormat == GL_RED)
	{
		rawData.resize(textureToExport->getWidth() * textureToExport->getHeight() * 4);
		unsigned char* textreData = getFETextureRawData(textureToExport);

		for (size_t i = 0; i < rawData.size(); i += 4)
		{
			rawData[i] = textreData[i / 4];
			rawData[i + 1] = textreData[i / 4];
			rawData[i + 2] = textreData[i / 4];
			rawData[i + 3] = 255;
		}
	}
	else if (textureToExport->internalFormat == GL_R16)
	{
		rawData.resize(textureToExport->getWidth() * textureToExport->getHeight() * 2);
		unsigned char* textreData = getFETextureRawData(textureToExport);

		for (size_t i = 0; i < rawData.size(); i++)
		{
			rawData[i] = textreData[i];
		}

		for (size_t i = 0; i < rawData.size(); i+=2)
		{
			std::swap(rawData[i], rawData[i + 1]);
		}
	}
	else
	{
		rawData.resize(textureToExport->getWidth() * textureToExport->getHeight() * 4);
		unsigned char* textreData = getFETextureRawData(textureToExport);
		for (size_t i = 0; i < rawData.size(); i++)
		{
			rawData[i] = textreData[i];
		}
	}

	std::string filePath = fileName;
	int error = 0;
	if (textureToExport->internalFormat == GL_R16)
	{
		error = lodepng::encode(filePath, rawData, textureToExport->getWidth(), textureToExport->getHeight(), LCT_GREY, 16);
	}
	else
	{
		error = lodepng::encode(filePath, rawData, textureToExport->getWidth(), textureToExport->getHeight());
	}

	return error == 0;
}

bool FEResourceManager::exportRawDataToPNG(const char* fileName, unsigned char* textureData, int width, int height, GLint internalformat)
{
	if (internalformat != GL_RGBA &&
		internalformat != GL_RED &&
		internalformat != GL_R16 &&
		internalformat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		internalformat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::exportRawDataToPNG internalFormat is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return false;
	}

	std::string filePath = fileName;
	int error = 0;
	if (internalformat == GL_R16)
	{
		error = lodepng::encode(filePath, textureData, width, height, LCT_GREY, 16);
	}
	else
	{
		error = lodepng::encode(filePath, textureData, width, height);
	}

	return error == 0;
}

unsigned char* FEResourceManager::resizeTextureRawData(FETexture* sourceTexture, size_t targetWidth, size_t targetHeight, int filtrationLevel)
{
	if (sourceTexture == nullptr)
	{
		LOG.add("FEResourceManager::resizeTextureRawData with nullptr sourceTexture", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (targetWidth <= 0 || targetHeight <= 0 || targetWidth > 8192 || targetHeight > 8192)
	{
		LOG.add("FEResourceManager::resizeTextureRawData unsupported target resolution", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (targetWidth == sourceTexture->getWidth() && targetHeight == sourceTexture->getHeight())
	{
		LOG.add("FEResourceManager::resizeTextureRawData no operation needed", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (filtrationLevel < 0)
		filtrationLevel = 0;

	if (filtrationLevel > 16)
		filtrationLevel = 16;

	if (sourceTexture->internalFormat != GL_RGBA &&
		sourceTexture->internalFormat != GL_RED &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::resizeTextureRawData internalFormat of sourceTexture is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return nullptr;
	}

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->getTextureID()));

	unsigned char* currentData = getFETextureRawData(sourceTexture);
	unsigned char* result = resizeTextureRawData(currentData, sourceTexture->getWidth(), sourceTexture->getHeight(), targetWidth, targetHeight, sourceTexture->internalFormat, filtrationLevel);
	delete[] currentData;

	return result;
}

unsigned char* FEResourceManager::resizeTextureRawData(unsigned char* textureData, size_t width, size_t height, size_t targetWidth, size_t targetHeight, GLint internalFormat, int filtrationLevel)
{
	if (textureData == nullptr)
	{
		LOG.add("FEResourceManager::resizeTextureRawData with nullptr textureData", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (width <= 0 || height <= 0 || width > 8192 || height > 8192)
	{
		LOG.add("FEResourceManager::resizeTextureRawData unsupported current resolution", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (targetWidth == width && targetHeight == height)
	{
		LOG.add("FEResourceManager::resizeTextureRawData no operation needed", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (targetWidth <= 0 || targetHeight <= 0 || targetWidth > 8192 || targetHeight > 8192)
	{
		LOG.add("FEResourceManager::resizeTextureRawData unsupported target resolution", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (internalFormat != GL_RGBA &&
		internalFormat != GL_RED &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::resizeTextureRawData internalFormat of textureData is not supported", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (filtrationLevel < 0)
		filtrationLevel = 0;

	if (filtrationLevel > 16)
		filtrationLevel = 16;

	size_t sourceByteCount = 4;
	if (internalFormat == GL_RED)
		sourceByteCount = 1;

	float resizeFactorX = float(width) / float(targetWidth);
	float resizeFactorY = float(height) / float(targetHeight);

	//int newPixel[4];
	unsigned char* result = new unsigned char[targetWidth * targetHeight * 4];
	for (size_t i = 0; i < targetHeight; i++)
	{
		for (size_t j = 0; j < targetWidth; j++)
		{
			size_t targetIndex = (i * targetWidth + j) * 4;

			if (targetIndex + 3 >= targetWidth * targetHeight * 4)
				continue;

			/*newPixel[0] = 0;
			newPixel[1] = 0;
			newPixel[2] = 0;
			newPixel[3] = 0;

			int pixelsRead = 0;
			for (int p = -filtrationLevel; p <= filtrationLevel; p++)
			{
				if (i + p < 0 || i + p >= targetHeight)
					continue;

				for (int k = -filtrationLevel; k <= filtrationLevel; k++)
				{
					if (j + k < 0 || j + k >= targetWidth)
						continue;

					float scaledI_f = (i + p) * resizeFactorY;
					float scaledJ_f = (j + k) * resizeFactorX;

					size_t scaledI = size_t((i + p) * resizeFactorY);
					size_t scaledJ = size_t((j + k) * resizeFactorX);

					float denominator = sqrt(pow(scaledI_f - float(scaledI), 2.0f) + pow(scaledJ_f - float(scaledJ), 2.0f));
					float weight = 1.0f;
					if (denominator != 0.0)
					{
						weight = 1.0f / denominator;
					}

					if (weight > 1.0f)
						weight = 1.0f;

					
					size_t sourceIndex = (scaledI * width + scaledJ) * sourceByteCount;

					if (sourceIndex + 3 >= width * height * sourceByteCount)
						continue;

					pixelsRead++;

					if (sourceByteCount == 4)
					{
						newPixel[0] += unsigned char(float(textureData[sourceIndex]) * weight);
						newPixel[1] += unsigned char(float(textureData[sourceIndex + 1]) * weight);
						newPixel[2] += unsigned char(float(textureData[sourceIndex + 2]) * weight);
						newPixel[3] += unsigned char(float(textureData[sourceIndex + 3]) * weight);
					}
					else
					{
						newPixel[0] += textureData[sourceIndex];
						newPixel[1] += textureData[sourceIndex];
						newPixel[2] += textureData[sourceIndex];
						newPixel[3] += 255;
					}
				}
			}

			if (pixelsRead != 0)
			{
				newPixel[0] /= pixelsRead;
				newPixel[1] /= pixelsRead;
				newPixel[2] /= pixelsRead;
				newPixel[3] /= pixelsRead;
			}

			result[targetIndex] = newPixel[0];
			result[targetIndex + 1] = newPixel[1];
			result[targetIndex + 2] = newPixel[2];
			result[targetIndex + 3] = newPixel[3];*/

			size_t scaledI = size_t(i * resizeFactorY);
			size_t scaledJ = size_t(j * resizeFactorX);
			size_t sourceIndex = (scaledI * width + scaledJ) * sourceByteCount;

			if (sourceByteCount == 4)
			{
				result[targetIndex] = textureData[sourceIndex];
				result[targetIndex + 1] = textureData[sourceIndex + 1];
				result[targetIndex + 2] = textureData[sourceIndex + 2];
				result[targetIndex + 3] = textureData[sourceIndex + 3];
			}
			else
			{
				result[targetIndex] = textureData[sourceIndex];
				result[targetIndex + 1] = textureData[sourceIndex];
				result[targetIndex + 2] = textureData[sourceIndex];
				result[targetIndex + 3] = 255;
			}
		}
	}

	return result;
}

void FEResourceManager::resizeTexture(FETexture* sourceTexture, int targetWidth, int targetHeight, int filtrationLevel)
{
	if (sourceTexture == nullptr)
	{
		LOG.add("FEResourceManager::resizeTexture with nullptr sourceTexture", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (targetWidth <= 0 || targetHeight <= 0 || targetWidth > 8192 || targetHeight > 8192)
	{
		LOG.add("FEResourceManager::resizeTexture unsupported target resolution", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (targetWidth == sourceTexture->getWidth() && targetHeight == sourceTexture->getHeight())
	{
		LOG.add("FEResourceManager::resizeTexture no operation needed", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (sourceTexture->internalFormat != GL_RGBA &&
		sourceTexture->internalFormat != GL_RED &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::resizeTexture internalFormat of sourceTexture is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return;
	}

	if (filtrationLevel < 0)
		filtrationLevel = 0;

	if (filtrationLevel > 16)
		filtrationLevel = 16;

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->getTextureID()));

	unsigned char* currentData = getFETextureRawData(sourceTexture);
	unsigned char* result = resizeTextureRawData(currentData, sourceTexture->getWidth(), sourceTexture->getHeight(), targetWidth, targetHeight, sourceTexture->internalFormat, filtrationLevel);

	sourceTexture->width = targetWidth;
	sourceTexture->height = targetHeight;
	int maxDimention = std::max(sourceTexture->width, sourceTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);

	if (sourceTexture->internalFormat == GL_RGBA)
	{
		updateFETextureRawData(sourceTexture, result, mipCount);
	}
	else if (sourceTexture->internalFormat == GL_RED)
	{
		// Function resizeTextureRawData will output RGBA data  we will need to take only R channel.
		std::vector<unsigned char> redChannel;
		redChannel.resize(sourceTexture->getWidth() * sourceTexture->getHeight());
		for (size_t i = 0; i < redChannel.size() * 4; i+=4)
		{
			redChannel[i / 4] = result[i];
		}

		updateFETextureRawData(sourceTexture, redChannel.data(), mipCount);
	}
	else
	{
		updateFETextureRawData(sourceTexture, result, mipCount);
	}

	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	delete[] currentData;
	delete[] result;
}

unsigned char* FEResourceManager::getFETextureRawData(FETexture* sourceTexture, size_t* rawDataSize)
{
	unsigned char* result = nullptr;
	if (rawDataSize != nullptr)
		*rawDataSize = 0;

	if (sourceTexture == nullptr)
	{
		LOG.add("FEResourceManager::getFETextureRawData with nullptr sourceTexture", FE_LOG_ERROR, FE_LOG_GENERAL);
		return result;
	}

	if (sourceTexture->internalFormat != GL_RGBA &&
		sourceTexture->internalFormat != GL_RED &&
		sourceTexture->internalFormat != GL_R16 &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		sourceTexture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::getFETextureRawData internalFormat of sourceTexture is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return result;
	}

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->textureID));

	if (sourceTexture->internalFormat == GL_R16)
	{
		if (rawDataSize != nullptr)
			*rawDataSize = sourceTexture->getWidth() * sourceTexture->getHeight() * 2;
		result = new unsigned char[sourceTexture->getWidth() * sourceTexture->getHeight() * 2];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (sourceTexture->internalFormat == GL_RED)
	{
		if (rawDataSize != nullptr)
			*rawDataSize = sourceTexture->getWidth() * sourceTexture->getHeight();
		result = new unsigned char[sourceTexture->getWidth() * sourceTexture->getHeight()];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else
	{
		if (rawDataSize != nullptr)
			*rawDataSize = sourceTexture->getWidth() * sourceTexture->getHeight() * 4;
		result = new unsigned char[sourceTexture->getWidth() * sourceTexture->getHeight() * 4];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, result));
	}
	
	return result;
}

void FEResourceManager::updateFETextureRawData(FETexture* texture, unsigned char* newRawData, size_t mipCount)
{
	if (texture == nullptr)
	{
		LOG.add("FEResourceManager::updateFETextureRawData with nullptr texture", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (texture->internalFormat != GL_RGBA &&
		texture->internalFormat != GL_RED &&
		texture->internalFormat != GL_R16 &&
		texture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		texture->internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FEResourceManager::updateFETextureRawData internalFormat of texture is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return;
	}

	if (texture->internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT || texture->internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		texture->internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < size_t(texture->getWidth() * texture->getHeight() * 4); i += 4)
		{
			if (newRawData[i] != 255)
			{
				texture->internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glDeleteTextures(1, &texture->textureID));
	FE_GL_ERROR(glGenTextures(1, &texture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture->textureID));

	if (texture->internalFormat == GL_RGBA)
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_RGBA8, texture->getWidth(), texture->getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(newRawData)));
	}
	else if (texture->internalFormat == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_R8, texture->getWidth(), texture->getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GL_RED, GL_UNSIGNED_BYTE, (void*)(newRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (texture->internalFormat == GL_R16)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_R16, texture->getWidth(), texture->getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GL_RED, GL_UNSIGNED_SHORT, (void*)(newRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), texture->internalFormat, texture->getWidth(), texture->getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(newRawData)));
	}
}

void FEResourceManager::deleteTerrainLayerMask(FETerrain* terrain, size_t layerIndex)
{
	if (terrain == nullptr)
	{
		LOG.add("FEResourceManager::deleteTerrainLayerMask with nullptr terrain", FE_LOG_WARNING, FE_LOG_GENERAL);
		return;
	}

	if (layerIndex < 0 || layerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.add("FEResourceManager::deleteTerrainLayerMask with out of bound \"layerIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (terrain->layers[layerIndex] == nullptr)
	{
		LOG.add("FEResourceManager::deleteTerrainLayerMask on indicated layer slot layer is nullptr", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	clearTerrainLayerMask(terrain, layerIndex);

	std::vector<unsigned char*> layersPerTextureData;
	layersPerTextureData.resize(2);
	size_t rawDataSize = 0;
	layersPerTextureData[0] = getFETextureRawData(terrain->layerMaps[0], &rawDataSize);
	layersPerTextureData[1] = getFETextureRawData(terrain->layerMaps[1]);

	std::vector<unsigned char*> allLayers;
	allLayers.resize(8);
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		allLayers[i] = new unsigned char[rawDataSize / 4];
	}

	// Gathering channels from 2 textures.
	int channelIndex = 0;
	for (size_t i = 0; i < rawDataSize; i+=4)
	{
		allLayers[0][channelIndex] = layersPerTextureData[0][i];
		allLayers[1][channelIndex] = layersPerTextureData[0][i + 1];
		allLayers[2][channelIndex] = layersPerTextureData[0][i + 2];
		allLayers[3][channelIndex] = layersPerTextureData[0][i + 3];

		allLayers[4][channelIndex] = layersPerTextureData[1][i];
		allLayers[5][channelIndex] = layersPerTextureData[1][i + 1];
		allLayers[6][channelIndex] = layersPerTextureData[1][i + 2];
		allLayers[7][channelIndex] = layersPerTextureData[1][i + 3];

		channelIndex++;
	}

	// Shifting existing layers masks to place where was deleted mask.
	for (size_t i = layerIndex; i < FE_TERRAIN_MAX_LAYERS - 1; i++)
	{
		for (size_t j = 0; j < rawDataSize / 4; j++)
		{
			allLayers[i][j] = allLayers[i + 1][j];
		}
	}

	unsigned char* firstTextureData = new unsigned char[rawDataSize];
	unsigned char* secondTextureData = new unsigned char[rawDataSize];

	// Putting individual channels back to 2 distinct textures.
	channelIndex = 0;
	for (size_t i = 0; i < rawDataSize; i += 4)
	{
		firstTextureData[i] = allLayers[0][channelIndex];
		firstTextureData[i + 1] = allLayers[1][channelIndex];
		firstTextureData[i + 2] = allLayers[2][channelIndex];
		firstTextureData[i + 3] = allLayers[3][channelIndex];

		secondTextureData[i] = allLayers[4][channelIndex];
		secondTextureData[i + 1] = allLayers[5][channelIndex];
		secondTextureData[i + 2] = allLayers[6][channelIndex];
		secondTextureData[i + 3] = allLayers[7][channelIndex];

		channelIndex++;
	}

	int maxDimention = std::max(terrain->layerMaps[0]->getWidth(), terrain->layerMaps[0]->getHeight());
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);

	updateFETextureRawData(terrain->layerMaps[0], firstTextureData, mipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	updateFETextureRawData(terrain->layerMaps[1], secondTextureData, mipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete[] allLayers[i];
	}

	delete[] firstTextureData;
	delete[] secondTextureData;

	terrain->deleteLayerInSlot(layerIndex);
}

FETexture* FEResourceManager::LoadJPGTexture(const char* fileName, std::string Name)
{
	int uWidth, uHeight, channels;
	unsigned char* rawData = stbi_load(fileName, &uWidth, &uHeight, &channels, 0);

	if (rawData == nullptr)
	{
		LOG.add(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadJPGTexture.", FE_LOG_ERROR, FE_LOG_LOADING);
		if (standardTextures.size() > 0)
		{
			return getTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	FETexture* newTexture = createTexture(Name);
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, newTexture->width, newTexture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, rawData);
	delete rawData;
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

FETexture* FEResourceManager::LoadBMPTexture(const char* fileName, std::string Name)
{
	return LoadJPGTexture(fileName, Name);
}

void FEResourceManager::createMaterialsFromOBJData(std::vector<FEObject*>& resultArray)
{
	std::unordered_map<std::string, bool> loadedTextures;
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	for (size_t i = 0; i < objLoader.loadedObjects.size(); i++)
	{
		if (objLoader.loadedObjects[i]->materialRecords.size() == 0)
			continue;

		FEMaterial* material = nullptr;
		std::string name = "";
		FETexture* texture = nullptr;
		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
				material->setAlbedoMap(texture);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].albedoMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].normalMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].normalMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].normalMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].normalMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].normalMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
				material->setNormalMap(texture);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].normalMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].specularMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].specularMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].specularMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].specularMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].specularMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].specularMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].specularHighlightMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].alphaMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].displacementMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (loadedTextures.find(objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile) == loadedTextures.end() &&
			objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile != "")
		{
			std::vector<FEObject*> loadedObjects = importAsset(objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile.c_str());
			if (loadedObjects.size() != 0 && loadedObjects[0] != nullptr)
			{
				texture = reinterpret_cast<FETexture*>(loadedObjects[0]);

				loadedTextures[objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile] = true;
				name = FILE_SYSTEM.getFileName(objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile.c_str());

				resultArray.push_back(texture);
				resultArray.back()->setName(name);

				if (material == nullptr)
					material = createMaterial(objLoader.loadedObjects[i]->materialRecords[0].name);
			}
			else
			{
				LOG.add(std::string("can't load texture: ") + objLoader.loadedObjects[i]->materialRecords[0].stencilDecalMapFile + " in function FEResourceManager::createMaterialsFromOBJData.", FE_LOG_ERROR, FE_LOG_LOADING);
			}
		}

		if (material != nullptr)
		{
			material->shader = getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
			resultArray.push_back(material);

			FEGameModel* gameModel = createGameModel(reinterpret_cast<FEMesh*>(resultArray[i]), material);
			resultArray.push_back(gameModel);
		}
	}
}

std::vector<FEObject*> FEResourceManager::importAsset(const char* fileName)
{
	std::vector<FEObject*> result; 
	if (fileName == nullptr)
	{
		LOG.add("call of FEResourceManager::importAsset with nullptr fileName", FE_LOG_ERROR, FE_LOG_LOADING);
		return result;
	}

	if (!FILE_SYSTEM.checkFile(fileName))
	{
		LOG.add("Can't locate file: " + std::string(fileName) + " in FEResourceManager::importAsset", FE_LOG_ERROR, FE_LOG_LOADING);
		return result;
	}

	std::string fileExtention = FILE_SYSTEM.getFileExtension(fileName);
	// To lower case
	std::transform(fileExtention.begin(), fileExtention.end(), fileExtention.begin(), [](unsigned char c) { return std::tolower(c); });

	if (fileExtention == ".png")
	{
		FETexture* tempTexture = LoadPNGTexture(fileName);
		if (tempTexture != nullptr)
		{
			result.push_back(tempTexture);
			return result;
		}
	}
	else if (fileExtention == ".jpg")
	{
		FETexture* tempTexture = LoadJPGTexture(fileName);
		if (tempTexture != nullptr)
		{
			result.push_back(tempTexture);
			return result;
		}
	}
	else if (fileExtention == ".bmp")
	{
		FETexture* tempTexture = LoadBMPTexture(fileName);
		if (tempTexture != nullptr)
		{
			result.push_back(tempTexture);
			return result;
		}
	}
	else if (fileExtention == ".obj")
	{
		result = importOBJ(fileName);
		return result;
	}
	else if (fileExtention == ".gltf")
	{
		return LoadGLTF(fileName);
	}

	return result;
}

FETexture* FEResourceManager::createTextureWithTransparency(FETexture* originalTexture, FETexture* maskTexture)
{
	if (originalTexture == nullptr || maskTexture == nullptr)
	{
		LOG.add("call of FEResourceManager::createTextureWithTransparency with nullptr argument(s)", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	if (originalTexture->getWidth() != maskTexture->getWidth() || originalTexture->getHeight() != maskTexture->getHeight())
	{
		LOG.add("originalTexture and maskTexture dimensions mismatch in FEResourceManager::createTextureWithTransparency", FE_LOG_ERROR, FE_LOG_GENERAL);
		return nullptr;
	}

	FETexture* result = createTexture();
	unsigned char* rawData = getFETextureRawData(originalTexture);
	result->width = originalTexture->getWidth();
	result->height = originalTexture->getHeight();

	unsigned char* maskRawData = getFETextureRawData(maskTexture);
	for (size_t i = 4; i < size_t(result->width * result->height * 4); i += 4)
	{
		rawData[-1 + i] = maskRawData[-4 + i];
	}

	int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, result->textureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, result->width, result->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData);
	result->internalFormat = internalFormat;

	if (result->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (result->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	result->fileName = originalTexture->fileName;

	std::string filePath = result->fileName;
	std::size_t index = filePath.find_last_of("/\\");
	std::string newFileName = filePath.substr(index + 1);
	index = newFileName.find_last_of(".");
	std::string fileNameWithOutExtention = newFileName.substr(0, index);
	result->setName(fileNameWithOutExtention);

	delete[] rawData;
	delete[] maskRawData;

	return result;
}

std::vector<FEObject*> FEResourceManager::LoadGLTF(const char* fileName)
{
	std::vector<FEObject*> result;
	if (!FILE_SYSTEM.checkFile(fileName))
	{
		LOG.add("call of FEResourceManager::LoadGLTF can't locate file: " + std::string(fileName), FE_LOG_ERROR, FE_LOG_LOADING);
		return result;
	}

	FEglTFLoader& glTF = FEglTFLoader::getInstance();
	glTF.load(fileName);

	std::unordered_map<std::string, FETexture*> alreadyLoadedTextures;
	std::unordered_map<int, FETexture*> textureMap;
	for (size_t i = 0; i < glTF.textures.size(); i++)
	{
		if (alreadyLoadedTextures.find(glTF.textures[i]) != alreadyLoadedTextures.end())
		{
			textureMap[int(textureMap.size())] = alreadyLoadedTextures[glTF.textures[i]];
			continue;
		}

		if (!FILE_SYSTEM.checkFile(glTF.textures[i].c_str()))
		{
			textureMap[int(textureMap.size())] = nullptr;
			continue;
		}

		std::vector<FEObject*> tempResult = importAsset(glTF.textures[i].c_str());
		if (tempResult.size() > 0)
		{
			textureMap[int(textureMap.size())] = reinterpret_cast<FETexture*>(tempResult[0]);
			alreadyLoadedTextures[glTF.textures[i]] = reinterpret_cast<FETexture*>(tempResult[0]);
			result.push_back(tempResult[0]);
		}
	}

	std::unordered_map<int, FEMaterial*> materialsMap;
	for (size_t i = 0; i < glTF.materials.size(); i++)
	{
		FEMaterial* newMaterial = createMaterial(glTF.materials[i].name);
		materialsMap[int(i)] = newMaterial;
		newMaterial->shader = getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		if (textureMap.find(glTF.materials[i].baseColorTexture.index) != textureMap.end() && textureMap[glTF.materials[i].baseColorTexture.index] != nullptr)
		{
			newMaterial->addTexture(textureMap[glTF.materials[i].baseColorTexture.index]);
			newMaterial->setAlbedoMap(textureMap[glTF.materials[i].baseColorTexture.index]);
		}
		else if (glTF.materials[i].baseColor[0] != -1)
		{
			newMaterial->baseColor = glTF.materials[i].baseColor;
		}

		if (textureMap.find(glTF.materials[i].metallicRoughnessTexture.index) != textureMap.end() && textureMap[glTF.materials[i].metallicRoughnessTexture.index] != nullptr)
		{
			/*
				https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/Specification.adoc#reference-material
				The textures for metalnessand roughness properties are packed together in a single texture called metallicRoughnessTexture.
				Its green channel contains roughness values and its blue channel contains metalness values.
			*/
			newMaterial->addTexture(textureMap[glTF.materials[i].metallicRoughnessTexture.index]);
			newMaterial->setRoughtnessMap(textureMap[glTF.materials[i].metallicRoughnessTexture.index], 1, 0);
			newMaterial->setMetalnessMap(textureMap[glTF.materials[i].metallicRoughnessTexture.index], 2, 0);
		}

		if (textureMap.find(glTF.materials[i].normalTexture.index) != textureMap.end() && textureMap[glTF.materials[i].normalTexture.index] != nullptr)
		{
			newMaterial->addTexture(textureMap[glTF.materials[i].normalTexture.index]);
			newMaterial->setNormalMap(textureMap[glTF.materials[i].normalTexture.index]);
		}

		if (textureMap.find(glTF.materials[i].occlusionTexture.index) != textureMap.end() && textureMap[glTF.materials[i].occlusionTexture.index] != nullptr)
		{
			newMaterial->addTexture(textureMap[glTF.materials[i].occlusionTexture.index]);
			newMaterial->setAOMap(textureMap[glTF.materials[i].occlusionTexture.index]);
		}

		result.push_back(newMaterial);
	}

	std::unordered_map<int, FEGameModel*> gameModelMap;
	for (size_t i = 0; i < glTF.primitives.size(); i++)
	{
		gameModelMap[int(i)] = nullptr;
		
		if (glTF.primitives[i].rawData.indices.size() != 0)
		{
			int UVIndex = 0;
			if (glTF.primitives[i].material != -1)
			{
				UVIndex = glTF.materials[glTF.primitives[i].material].baseColorTexture.texCoord;
				if (glTF.primitives[i].rawData.UVs.size() <= UVIndex)
					UVIndex = 0;
			}
			
			result.push_back(rawDataToMesh(glTF.primitives[i].rawData.positions,
										   glTF.primitives[i].rawData.normals,
										   glTF.primitives[i].rawData.tangents,
										   glTF.primitives[i].rawData.UVs[0/*UVIndex*/],
										   glTF.primitives[i].rawData.indices));

			if (glTF.primitives[i].material != -1)
			{
				FEGameModel* newGameModel = createGameModel(reinterpret_cast<FEMesh*>(result.back()), materialsMap[glTF.primitives[i].material]);
				//glTF.primitives[i].
				//newGameModel->setName();
				gameModelMap[int(i)] = newGameModel;
				result.push_back(newGameModel);

				FEPrefab* newPrefab = createPrefab(newGameModel);
				result.push_back(newPrefab);
			}
		}
	}

	for (size_t i = 0; i < glTF.entities.size(); i++)
	{
		int gameModelIndex = -1;
		for (size_t j = 0; j < glTF.gameModels.size(); j++)
		{
			if (glTF.gameModels[j].meshParent == glTF.entities[i].mesh)
			{
				gameModelIndex = int(j);
				break;
			}
		}

		if (gameModelIndex != -1)
		{
			FEEntity* newEntity = createEntity(gameModelMap[gameModelIndex], glTF.entities[i].name);
			if (gameModelMap[gameModelIndex] != nullptr)
				gameModelMap[gameModelIndex]->setName(glTF.entities[i].name);

			newEntity->transform.setPosition(glTF.entities[i].translation);
			newEntity->transform.setScale(glTF.entities[i].scale);
			newEntity->transform.setRotation(glTF.entities[i].rotation);

			result.push_back(newEntity);
		}
	}

	return result;
}

std::vector<std::string> FEResourceManager::getPrefabList()
{
	FE_MAP_TO_STR_VECTOR(prefabs)
}

std::vector<std::string> FEResourceManager::getStandardPrefabList()
{
	FE_MAP_TO_STR_VECTOR(standardPrefabs)
}

FEPrefab* FEResourceManager::getPrefab(std::string ID)
{
	if (prefabs.find(ID) == prefabs.end())
	{
		if (standardPrefabs.find(ID) != standardPrefabs.end())
		{
			return standardPrefabs[ID];
		}

		return nullptr;
	}

	return prefabs[ID];
}

std::vector<FEPrefab*> FEResourceManager::getPrefabByName(std::string Name)
{
	std::vector<FEPrefab*> result;

	auto it = prefabs.begin();
	while (it != prefabs.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	it = standardPrefabs.begin();
	while (it != standardPrefabs.end())
	{
		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
}

FEPrefab* FEResourceManager::createPrefab(FEGameModel* gameModel, std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedPrefab";

	FEPrefab* newPrefab = new FEPrefab();
	if (forceObjectID != "")
	{
		prefabs[forceObjectID] = newPrefab;
		prefabs[forceObjectID]->setID(forceObjectID);
	}
	else
	{
		prefabs[newPrefab->ID] = newPrefab;
	}

	prefabs[newPrefab->ID]->setName(Name);
	if (gameModel != nullptr)
	{
		prefabs[newPrefab->ID]->components.push_back(new FEPrefabComponent());
		prefabs[newPrefab->ID]->components.back()->gameModel = gameModel;
	}
	
	return prefabs[newPrefab->ID];
}

bool FEResourceManager::makePrefabStandard(FEPrefab* prefab)
{
	if (prefab == nullptr)
	{
		LOG.add("prefab is nullptr in function FEResourceManager::makePrefabStandard.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (standardPrefabs.find(prefab->getObjectID()) == standardPrefabs.end())
	{
		if (prefabs.find(prefab->getObjectID()) != prefabs.end())
			prefabs.erase(prefab->getObjectID());
		standardPrefabs[prefab->getObjectID()] = prefab;

		return true;
	}

	return false;
}

void FEResourceManager::deletePrefab(FEPrefab* prefab)
{
	prefabs.erase(prefab->getObjectID());
	delete prefab;
}

void FEResourceManager::loadStandardPrefabs()
{
	FEPrefab* newPrefab = new FEPrefab(getGameModel("67251E393508013ZV579315F"/*"standardGameModel"*/), "standardPrefab");
	newPrefab->setID("4575527C773848040760656F");
	makePrefabStandard(newPrefab);
}