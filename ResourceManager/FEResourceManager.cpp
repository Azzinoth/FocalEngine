#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

FETexture* FEResourceManager::createTexture(std::string Name)
{
	if (Name.size() == 0 || textures.find(Name) != textures.end())
		Name = freeAssetName(FE_TEXTURE);
	
	FETexture* newTexture = new FETexture(Name);
	textures[Name] = newTexture;

	return newTexture;
}

bool FEResourceManager::makeTextureStandard(FETexture* texture)
{
	if (standardTextures.find(texture->getName()) == standardTextures.end())
	{
		if (textures.find(texture->getName()) != textures.end())
			textures.erase(texture->getName());
		standardTextures[texture->getName()] = texture;

		return true;
	}

	return false;
}

bool FEResourceManager::setTextureName(FETexture* Texture, std::string TextureName)
{
	if (TextureName.size() == 0 || textures.find(TextureName) != textures.end() || standardTextures.find(TextureName) != standardTextures.end())
		return false;

	textures.erase(Texture->getName());
	textures[TextureName] = Texture;
	
	Texture->setName(TextureName);
	return true;
}

FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name)
{
	if (Name.size() == 0 || meshes.find(Name) != meshes.end() || standardMeshes.find(Name) != standardMeshes.end())
		Name = freeAssetName(FE_MESH);

	FEMesh* newMesh = new FEMesh(VaoID, VertexCount, VertexBuffersTypes, AABB, Name);
	newMesh->setName(Name);
	meshes[Name] = newMesh;

	return newMesh;
}

bool FEResourceManager::setMeshName(FEMesh* Mesh, std::string MeshName)
{
	if (MeshName.size() == 0 || meshes.find(MeshName) != meshes.end() || standardMeshes.find(MeshName) != standardMeshes.end())
		return false;

	meshes.erase(Mesh->getName());
	meshes[MeshName] = Mesh;

	Mesh->setName(MeshName);
	return true;
}

bool FEResourceManager::makeMeshStandard(FEMesh* mesh)
{
	if (standardMeshes.find(mesh->getName()) == standardMeshes.end())
	{
		if (meshes.find(mesh->getName()) != meshes.end())
			meshes.erase(mesh->getName());
		standardMeshes[mesh->getName()] = mesh;

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
		//Log...
		assert(rawData.size());
	}
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	int internalFormat = usingAlpha ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data()));
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
		setTextureName(newTexture, fileNameWithOutExtention);
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
		//Log...
		assert(rawData.size());
	}
	newTexture->width = uWidth;
	newTexture->height = uHeight;

	// Transparency mask part
	std::vector<unsigned char> maskRawData;
	lodepng::decode(maskRawData, uWidth, uHeight, maskFileName);
	if (maskRawData.size() == 0)
	{
		//Log...
		assert(maskRawData.size());
	}

	for (size_t i = 4; i < uWidth * uHeight * 4; i+=4)
	{
		rawData[-1 + i] = maskRawData[-4 + i];
	}

	int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	FE_GL_ERROR(glGenTextures(1, &newTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, newTexture->textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data()));
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
		setTextureName(newTexture, fileNameWithOutExtention);
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

		int assetIDSize = texture->getAssetID().size() + 1;
		file.write((char*)&assetIDSize, sizeof(int));
		file.write((char*)texture->getAssetID().c_str(), sizeof(char) * assetIDSize);

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

	int assetIDSize = texture->getAssetID().size() + 1;
	file.write((char*)&assetIDSize, sizeof(int));
	file.write((char*)texture->getAssetID().c_str(), sizeof(char) * assetIDSize);

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
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));
	
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

FETexture* FEResourceManager::LoadFETexture(const char* fileName, std::string Name, FETexture* existingTexture)
{
	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
	{
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture.");
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
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture. File was created in different version of engine!");
		if (standardTextures.size() > 0)
		{
			return getTexture("noTexture");
		}
		else
		{
			return nullptr;
		}
	}

	int assetIDSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* assetID = new char[assetIDSize];
	strcpy_s(assetID, assetIDSize, (char*)(&fileData[currentShift]));
	currentShift += assetIDSize;

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
		setTextureName(newTexture, textureName);
	}
	else
	{
		newTexture = createTexture(textureName);
	}
	
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

	// Height map should not be loaded by this function
	if (newTexture->internalFormat == GL_R16)
		return nullptr;

	int maxDimention = std::max(newTexture->width, newTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, mipCount, newTexture->internalFormat, newTexture->width, newTexture->height));

	if (newTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
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

	// overwrite assetID with assetID from file.
	if (assetID != nullptr)
		newTexture->ID = assetID;

	delete[]assetID;
	delete[] fileData;
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
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture.");
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
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFETexture. File was created in different version of engine!");
		if (standardTextures.size() > 0)
		{
			return getTexture("noTexture");
		}
		else
		{
			return nullptr;
		}
	}

	int assetIDSize = *(int*)(&fileData[currentShift]);
	currentShift += 4;

	char* assetID = new char[assetIDSize];
	strcpy_s(assetID, assetIDSize, (char*)(&fileData[currentShift]));
	currentShift += assetIDSize;

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

	// overwrite assetID with assetID from file.
	if (assetID != nullptr)
		newTexture->ID = assetID;

	delete[]assetID;
	delete[] fileData;
	delete[] textureName;

	terrain->heightMap = newTexture;
	initTerrainEditTools(terrain);
	return newTexture;
}

FETexture* FEResourceManager::LoadFETextureUnmanaged(const char* fileName, std::string Name)
{
	FETexture* newTexture = LoadFETexture(fileName, Name);
	textures.erase(newTexture->getName());
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
	if (meshes.find("cube") != meshes.end())
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

	standardMeshes["cube"] = rawDataToMesh(cubePositions, cubeNormals, cubeTangents, cubeUV, cubeIndices, "cube");
	meshes.erase("cube");

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

	standardMeshes["plane"] = rawDataToMesh(planePositions, planeNormals, planeTangents, planeUV, planeIndices, "plane");
	meshes.erase("plane");

	standardMeshes["sphere"] = LoadFEMesh("7F251E3E0D08013E3579315F.model", "sphere");
	meshes.erase("sphere");
}

FEResourceManager::FEResourceManager()
{
	noTexture = LoadFETexture("48271F005A73241F5D7E7134.texture", "noTexture");
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
	setMeshName(newMesh, Name);
	meshes[newMesh->getName()] = newMesh;

	return newMesh;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* fileName, std::string Name)
{
	std::fstream file;

	file.open(fileName, std::ios::in | std::ios::binary);
	std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
	{
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFEMesh.");
		if (standardMeshes.size() > 0)
		{
			return getMesh("cube");
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
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::LoadFEMesh. File was created in different version of engine!");
		if (standardMeshes.size() > 0)
		{
			return getMesh("cube");
		}
		else
		{
			return nullptr;
		}
	}

	int assetIDSize = 0;
	file.read(buffer, 4);
	assetIDSize = *(int*)buffer;

	char* assetID = new char[assetIDSize + 1];
	file.read(assetID, assetIDSize);
	assetID[assetIDSize] = '\0';

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

	// overwrite assetID with assetID from file.
	if (assetID != nullptr)
		newMesh->ID = assetID;

	delete[]assetID;

	delete[] buffer;
	delete[] vertexBuffer;
	delete[] texBuffer;
	delete[] normBuffer;
	delete[] tangBuffer;
	delete[] indexBuffer;

	newMesh->AABB = meshAABB;
	newMesh->setName(Name);

	meshes[newMesh->getName()] = newMesh;

	return newMesh;
}

FEMaterial* FEResourceManager::createMaterial(std::string Name, std::string forceAssetID)
{
	if (Name.size() == 0 || materials.find(Name) != materials.end())
		Name = freeAssetName(FE_MATERIAL);

	materials[Name] = new FEMaterial(Name);
	if (forceAssetID != "")
		materials[Name]->ID = forceAssetID;

	return materials[Name];
}

bool FEResourceManager::setMaterialName(FEMaterial* Material, std::string MaterialName)
{
	if (MaterialName.size() == 0 || materials.find(MaterialName) != materials.end() || standardMaterials.find(MaterialName) != standardMaterials.end())
		return false;

	materials.erase(Material->getName());
	materials[MaterialName] = Material;

	Material->setName(MaterialName);
	return true;
}

FEEntity* FEResourceManager::createEntity(FEGameModel* gameModel, std::string Name, std::string forceAssetID)
{
	FEEntity* newEntity = new FEEntity(gameModel, Name);
	if (forceAssetID != "")
		newEntity->ID = forceAssetID;
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

FEMaterial* FEResourceManager::getMaterial(std::string name)
{
	if (materials.find(name) == materials.end())
	{
		if (standardMaterials.find(name) != standardMaterials.end())
		{
			return standardMaterials[name];
		}

		return nullptr;
	}

	return materials[name];
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

FEMesh* FEResourceManager::getMesh(std::string meshName)
{
	if (meshes.find(meshName) == meshes.end())
	{
		if (standardMeshes.find(meshName) != standardMeshes.end())
		{
			return standardMeshes[meshName];
		}
		
		return nullptr;
	}
	else
	{
		return meshes[meshName];
	}
}

bool FEResourceManager::makeMaterialStandard(FEMaterial* material)
{
	if (standardMaterials.find(material->getName()) == standardMaterials.end())
	{
		if (materials.find(material->getName()) != materials.end())
			materials.erase(material->getName());
		standardMaterials[material->getName()] = material;

		return true;
	}

	return false;
}

void FEResourceManager::loadStandardMaterial()
{
	FEMaterial* newMaterial = createMaterial("SolidColorMaterial");
	newMaterial->shader = createShader("FESolidColorShader", loadGLSL("CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_VS.glsl").c_str(),
															 loadGLSL("CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_FS.glsl").c_str());
	makeShaderStandard(newMaterial->shader);
	FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	newMaterial->addParameter(color);

	makeMaterialStandard(newMaterial);

	createShader("FEPhongShader", loadGLSL("CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_VS.glsl").c_str(),
								  loadGLSL("CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_FS.glsl").c_str());
	makeShaderStandard(getShader("FEPhongShader"));

	createShader("FEPBRShader", loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str(),
								loadGLSL("CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str());
	makeShaderStandard(getShader("FEPBRShader"));

	createShader("FETerrainShader", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_VS.glsl").c_str(),
									loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_FS.glsl").c_str(),
									loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TCS.glsl").c_str(), 
									loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TES.glsl").c_str(),
									loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_GS.glsl").c_str());
	makeShaderStandard(getShader("FETerrainShader"));

	createShader("FESMTerrainShader", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_VS.glsl").c_str(),
									  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_FS.glsl").c_str(),
									  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TCS.glsl").c_str(),
									  loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TES.glsl").c_str());

	FEShaderParam colorParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor");
	getShader("FESMTerrainShader")->addParameter(colorParam);

	makeShaderStandard(getShader("FESMTerrainShader"));

	createShader("FESkyDome", loadGLSL("CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_VS.glsl").c_str(),
							  loadGLSL("CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_FS.glsl").c_str());
	makeShaderStandard(getShader("FESkyDome"));

	createShader("terrainBrushOutput", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_VS.glsl").c_str(),
									   loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_FS.glsl").c_str());
	makeShaderStandard(getShader("terrainBrushOutput"));

	createShader("terrainBrushVisual", loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_VS.glsl").c_str(),
									   loadGLSL("CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_FS.glsl").c_str());
	makeShaderStandard(getShader("terrainBrushVisual"));
}

void FEResourceManager::loadStandardGameModels()
{
	FEGameModel* newGameModel = new FEGameModel(getMesh("sphere"), getMaterial("SolidColorMaterial"), "standardGameModel");
	makeGameModelStandard(newGameModel);
		
	standardGameModels["standardGameModel"] = new FEGameModel(getMesh("sphere"), getMaterial("SolidColorMaterial"), "standardGameModel");
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

	auto terrainIt = terrains.begin();
	while (terrainIt != terrains.end())
	{
		delete terrainIt->second;
		terrainIt++;
	}
	terrains.clear();
}

void FEResourceManager::saveFEMesh(FEMesh* Mesh, const char* fileName)
{
	std::fstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	// version of FEMesh file type
	float version = FE_MESH_VERSION;
	file.write((char*)&version, sizeof(float));

	int assetIDSize = Mesh->getAssetID().size();
	file.write((char*)&assetIDSize, sizeof(int));
	file.write((char*)Mesh->getAssetID().c_str(), sizeof(char) * assetIDSize);

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
		/*for (size_t i = 0; i < Mesh->materialInstances.size(); i++)
		{
			int nameSize = Mesh->materialInstances[i].name.size();
			file.write((char*)&nameSize, sizeof(int));
			file.write((char*)Mesh->materialInstances[i].name.c_str(), sizeof(char) * nameSize);

			int beginIndex = Mesh->materialInstances[i].firstFace;
			file.write((char*)&beginIndex, sizeof(int));

			int endIndex = Mesh->materialInstances[i].lastFace;
			file.write((char*)&endIndex, sizeof(int));
		}*/

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

FETexture* FEResourceManager::getTexture(std::string name)
{
	if (textures.find(name) == textures.end())
		return nullptr;

	return textures[name];
}

void FEResourceManager::deleteFETexture(FETexture* texture)
{
	// looking if this texture is used in some materials
	// to-do: should be done through list of pointers to materials that uses this texture.
	auto materialIterator = materials.begin();
	while (materialIterator != materials.end())
	{
		if (texture == materialIterator->second->albedoMap)
			materialIterator->second->albedoMap = noTexture;

		if (texture == materialIterator->second->normalMap)
			materialIterator->second->normalMap = noTexture;

		if (texture == materialIterator->second->roughtnessMap)
			materialIterator->second->roughtnessMap = noTexture;

		if (texture == materialIterator->second->metalnessMap)
			materialIterator->second->metalnessMap = noTexture;

		if (texture == materialIterator->second->AOMap)
			materialIterator->second->AOMap = noTexture;

		if (texture == materialIterator->second->displacementMap)
			materialIterator->second->displacementMap = noTexture;

		materialIterator++;
	}

	// after we make sure that texture is no more referenced by any material, we can delete it
	textures.erase(texture->getName());
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
			gameModelIterator->second->mesh = getMesh("sphere");
		}

		gameModelIterator++;
	}

	meshes.erase(mesh->getName());
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

FEGameModel* FEResourceManager::getGameModel(std::string name)
{
	if (gameModels.find(name) == gameModels.end())
	{
		if (standardGameModels.find(name) != standardGameModels.end())
		{
			return standardGameModels[name];
		}

		return nullptr;
	}

	return gameModels[name];
}

FEGameModel* FEResourceManager::createGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name, std::string forceAssetID)
{
	if (Name.size() == 0 || gameModels.find(Name) != gameModels.end() || standardGameModels.find(Name) != standardGameModels.end())
		Name = freeAssetName(FE_GAMEMODEL);

	if (Mesh == nullptr)
		Mesh = getMesh("sphere");

	if (Material == nullptr)
		Material = getMaterial("SolidColorMaterial");

	gameModels[Name] = new FEGameModel(Mesh, Material, Name);
	if (forceAssetID != "")
		gameModels[Name]->ID = forceAssetID;
	gameModels[Name]->setName(Name);
	return gameModels[Name];
}

bool FEResourceManager::setGameModelName(FEGameModel* gameModel, std::string gameModelName)
{
	if (gameModelName.size() == 0 || gameModels.find(gameModelName) != gameModels.end() || standardGameModels.find(gameModelName) != standardGameModels.end())
		return false;

	gameModels.erase(gameModel->getName());
	gameModels[gameModelName] = gameModel;

	gameModel->setName(gameModelName);
	return true;
}

void FEResourceManager::deleteGameModel(FEGameModel* gameModel)
{
	gameModels.erase(gameModel->getName());
	delete gameModel;
}

bool FEResourceManager::makeGameModelStandard(FEGameModel* gameModel)
{
	if (standardGameModels.find(gameModel->getName()) == standardGameModels.end())
	{
		if (gameModels.find(gameModel->getName()) != gameModels.end())
			gameModels.erase(gameModel->getName());
		standardGameModels[gameModel->getName()] = gameModel;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::createShader(std::string shaderName, const char* vertexText, const char* fragmentText,
										  const char* tessControlText, const char* tessEvalText,
										  const char* geometryText, const char* computeText)
{
	if (shaderName.size() == 0 || shaders.find(shaderName) != shaders.end() || standardShaders.find(shaderName) != standardShaders.end())
		shaderName = freeAssetName(FE_SHADER);

	shaders[shaderName] = new FEShader(shaderName, vertexText, fragmentText, tessControlText, tessEvalText, geometryText, computeText);
	return shaders[shaderName];
}

bool FEResourceManager::setShaderName(FEShader* shader, std::string shaderName)
{
	if (shader == nullptr)
		return false;

	if (shaderName.size() == 0 || shaders.find(shaderName) != shaders.end() || standardShaders.find(shaderName) != standardShaders.end())
		return false;

	shaders.erase(shader->getName());
	shaders[shaderName] = shader;

	shader->setName(shaderName);
	return true;
}

bool FEResourceManager::makeShaderStandard(FEShader* shader)
{
	if (shader == nullptr)
		return false;

	if (standardShaders.find(shader->getName()) == standardShaders.end())
	{
		if (shaders.find(shader->getName()) != shaders.end())
			shaders.erase(shader->getName());
		standardShaders[shader->getName()] = shader;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::getShader(std::string shaderName)
{
	if (shaders.find(shaderName) == shaders.end())
	{
		if (standardShaders.find(shaderName) != standardShaders.end())
		{
			return standardShaders[shaderName];
		}

		return nullptr;
	}
	else
	{
		return shaders[shaderName];
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

void FEResourceManager::deleteShader(std::string shaderName)
{
	FEShader* shaderToDelete = getShader(shaderName);
	if (shaderToDelete == nullptr)
		return;

	auto it = materials.begin();
	while (it != materials.end())
	{
		if (it->second->shader->getNameHash() == shaderToDelete->getNameHash())
			it->second->shader = getShader("FESolidColorShader");
		
		it++;
	}

	it = standardMaterials.begin();
	while (it != standardMaterials.end())
	{
		if (it->second->shader->getNameHash() == shaderToDelete->getNameHash())
			it->second->shader = getShader("FESolidColorShader");

		it++;
	}

	shaders.erase(shaderName);
	standardShaders.erase(shaderName);
	delete shaderToDelete;
}

bool FEResourceManager::replaceShader(std::string oldShaderName, FEShader* newShader)
{
	FEShader* shaderToReplace = getShader(oldShaderName);
	if (shaderToReplace == nullptr)
		return false;

	if (newShader->getName().size() == 0 && (shaders.find(newShader->getName()) != shaders.end() || standardShaders.find(newShader->getName()) != standardShaders.end()))
		return false;

	if (shaders.find(newShader->getName()) != shaders.end())
	{
		*(shaders[oldShaderName]) = *newShader;
	}
	else if (standardShaders.find(newShader->getName()) != standardShaders.end())
	{
		*(standardShaders[oldShaderName]) = *newShader;
	}

	return true;
}

FETerrain* FEResourceManager::createTerrain(bool createHeightMap, std::string name, std::string forceAssetID)
{
	if (name.size() == 0 || terrains.find(name) != terrains.end())
		name = freeAssetName(FE_TERRAIN);

	terrains[name] = new FETerrain(name);
	if (forceAssetID != "")
		terrains[name]->ID = forceAssetID;

	terrains[name]->shader = getShader("FETerrainShader");
	terrains[name]->layer0 = getMaterial("SolidColorMaterial");

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
		FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData.data()));
		//FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE));

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		terrains[name]->heightMapArray.resize(rawData.size() / sizeof(unsigned short));
		for (size_t i = 0; i < terrains[name]->heightMapArray.size(); i++)
		{
			terrains[name]->heightMapArray[i] = 0.0f;
		}

		glm::vec3 minPoint = glm::vec3(-1.0f, 0.0f, -1.0f);
		glm::vec3 maxPoint = glm::vec3(1.0f, 0.0f, 1.0f);
		terrains[name]->AABB = FEAABB(minPoint, maxPoint);
		terrains[name]->heightMap = newTexture;

		initTerrainEditTools(terrains[name]);
	}

	return terrains[name];
}

void FEResourceManager::initTerrainEditTools(FETerrain* terrain)
{
	if (terrain == nullptr)
	{
		LOG.logError("called FEResourceManager::initTerrainEditTools with nullptr terrain");
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

	terrain->brushOutputShader = getShader("terrainBrushOutput");
	terrain->brushVisualShader = getShader("terrainBrushVisual");

	terrain->planeMesh = getMesh("plane");
}

FETerrain* FEResourceManager::getTerrain(std::string terrainName)
{
	if (terrains.find(terrainName) == terrains.end())
		return nullptr;

	return terrains[terrainName];
}

bool FEResourceManager::setTerrainName(FETerrain* terrain, std::string terrainName)
{
	if (terrainName.size() == 0 || terrains.find(terrainName) != terrains.end())
		return false;

	terrains.erase(terrain->getName());
	terrains[terrainName] = terrain;

	terrain->setName(terrainName);
	return true;
}

std::vector<std::string> FEResourceManager::getTerrainList()
{
	FE_MAP_TO_STR_VECTOR(terrains)
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
		LOG.logError(std::string("can't read file: ") + fileName + " in function FEResourceManager::LoadHeightmap.");
		return this->noTexture;
	}

	if ((uWidth != 0 && (uWidth & (uWidth - 1)) == 0) && (uWidth != 0 && (uWidth & (uWidth - 1)) == 0))
	{
		// it is power of 2
	}
	else
	{
		delete newTexture;
		LOG.logError(std::string("texture has dementions not power of two! file: ") + fileName + " in function FEResourceManager::LoadHeightmap.");
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
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, newTexture->internalFormat, newTexture->width, newTexture->height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData.data()));
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
		setTextureName(newTexture, fileNameWithOutExtention);
	}

	if (terrain->heightMap != nullptr)
		delete terrain->heightMap;
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
		LOG.logError(std::string("can't load file: ") + fileName + " in function FEResourceManager::loadGLSL.");
	}

	return shaderData;
}

FETexture* FEResourceManager::createTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool unManaged, std::string Name)
{
	if (Name.size() == 0 || textures.find(Name) != textures.end())
		Name = freeAssetName(FE_TEXTURE);

	FETexture* newTexture = new FETexture(InternalFormat, Format, Width, Height, Name);
	if (!unManaged)
		textures[Name] = newTexture;

	return newTexture;
}

FEFramebuffer* FEResourceManager::createFramebuffer(int attachments, int Width, int Height, bool HDR)
{
	FEFramebuffer* newFramebuffer = new FEFramebuffer();

	newFramebuffer->width = Width;
	newFramebuffer->height = Height;

	FE_GL_ERROR(glGenFramebuffers(1, &newFramebuffer->fbo));
	newFramebuffer->bind();

	if (attachments & FE_COLOR_ATTACHMENT)
	{
		HDR ? newFramebuffer->colorAttachment = createTexture(GL_RGBA16F, GL_RGBA, Width, Height) : newFramebuffer->colorAttachment = new FETexture(Width, Height, freeAssetName(FE_TEXTURE));
		// Allocate the mipmaps
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		newFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newFramebuffer->colorAttachment);
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
		newFramebuffer->stencilAttachment = new FETexture(Width, Height, freeAssetName(FE_TEXTURE));
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
	newPostProcess->screenQuad = getMesh("plane");
	newPostProcess->screenQuadShader = getShader("FEScreenQuadShader");
	newPostProcess->intermediateFramebuffer = createFramebuffer(FocalEngine::FE_COLOR_ATTACHMENT, ScreenWidth, ScreenHeight);

	return newPostProcess;
}

std::string FEResourceManager::freeAssetName(FEAssetType assetType)
{
	std::string result = "NULL";
	switch (assetType)
	{
		case FocalEngine::FE_NULL:
		{
			return result;
			break;
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
			size_t nextID = terrains.size();
			size_t index = 0;
			result = "terrain_" + std::to_string(nextID + index);
			while (terrains.find(result) != terrains.end())
			{
				index++;
				result = "terrain_" + std::to_string(nextID + index);
			}

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
		LOG.logError("FEResourceManager::createSameFormatTexture called with nullptr pointer as exampleTexture");
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
		saveFEMesh(currentMesh, (std::string("C://Users//Azzinoth//Desktop//FocalEngine//FocalEnginePrivate//") + currentMesh->getAssetID() + std::string(".model")).c_str());
	}
}

void FEResourceManager::reSaveStandardTextures()
{
	auto it = standardTextures.begin();
	while (it != standardTextures.end())
	{
		saveFETexture(it->second, (std::string("C://Users//Azzinoth//Desktop//FocalEngine//FocalEnginePrivate//") + it->second->getAssetID() + std::string(".texture")).c_str());
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
			gameModelIterator->second->material = getMaterial("SolidColorMaterial");

		gameModelIterator++;
	}

	materials.erase(Material->getName());
	delete Material;
}