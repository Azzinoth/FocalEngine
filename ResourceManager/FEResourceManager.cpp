#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::_instance = nullptr;

FETexture* FEResourceManager::createTexture(std::string Name)
{
	FETexture* newTexture = new FETexture();

	if (Name.size() == 0 || textures.find(Name) != textures.end())
	{
		size_t nextID = textures.size();
		size_t index = 0;
		Name = "texture_" + std::to_string(nextID + index);
		while (textures.find(Name) != textures.end())
		{
			index++;
			Name = "texture_" + std::to_string(nextID + index);
		}
	}

	newTexture->setName(Name);
	textures[Name] = newTexture;

	return newTexture;
}

bool FEResourceManager::setTextureName(FETexture* Texture, std::string TextureName)
{
	if (TextureName.size() == 0 || textures.find(TextureName) != textures.end())
		return false;

	textures.erase(Texture->getName());
	textures[TextureName] = Texture;
	
	Texture->setName(TextureName);
	return true;
}

FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name)
{
	FEMesh* newMesh = new FEMesh(VaoID, VertexCount, VertexBuffersTypes, AABB);
	
	if (Name.size() == 0 || meshes.find(Name) != meshes.end() || standardMeshes.find(Name) != standardMeshes.end())
	{
		size_t nextID = meshes.size();
		size_t index = 0;
		Name = "mesh_" + std::to_string(nextID + index);
		while (meshes.find(Name) != meshes.end())
		{
			index++;
			Name = "mesh_" + std::to_string(nextID + index);
		}
	}

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

FETexture* FEResourceManager::LoadPngTextureAndCompress(const char* fileName, bool usingAlpha, std::string Name)
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

	std::string filePath = newTexture->fileName;
	std::size_t index = filePath.find_last_of("/\\");
	std::string newFileName = filePath.substr(index + 1);
	index = newFileName.find_last_of(".");
	std::string fileNameWithOutExtention = newFileName.substr(0, index);
	setTextureName(newTexture, fileNameWithOutExtention);
	/*index = fileNameWithOutExtention.find_last_of(".");
	fileNameWithOutExtention = fileNameWithOutExtention.substr(0, index);*/
	

	return newTexture;
}

void FEResourceManager::saveFETexture(const char* fileName, FETexture* texture)
{
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture->textureID));
	int maxDimention = std::max(texture->width, texture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	char** pixelData = new char*[mipCount];
	GLint imgSize = 0;
	std::fstream file;

	file.open(fileName, std::ios::out | std::ios::binary);

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
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imgSize);

		char* pixels = new char[imgSize * 2];
		for (size_t i = 0; i < size_t(imgSize * 2); i++)
		{
			pixels[i] = ' ';
		}
		glGetCompressedTexImage(GL_TEXTURE_2D, GLint(i), pixels);

		int realSize = 0;
		for (size_t i = imgSize * 2 - 1; i > 0 ; i--)
		{
			if (pixels[i] != ' ')
			{
				realSize = i + 1;
				break;
			}
		}

		pixelData[i] = new char[realSize];
		memcpy(pixelData[i], pixels, realSize);
		delete[] pixels;

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

void FEResourceManager::saveFETexture(const char* fileName, char* textureData, int width, int height, bool isAlphaUsed)
{
	FETexture* tempTexture = new FETexture();
	tempTexture->width = width;
	tempTexture->height = height;
	tempTexture->internalFormat = isAlphaUsed ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glGenTextures(1, &tempTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, tempTexture->textureID));
	FE_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, tempTexture->internalFormat, tempTexture->width, tempTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));
	

	if (tempTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));// to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (tempTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	saveFETexture(fileName, tempTexture);
	delete tempTexture;
}

void FEResourceManager::LoadFETexture(const char* fileName, FETexture* existingTexture)
{
	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	char* fileData = new char[int(fileSize)];
	file.read(fileData, fileSize);
	file.close();

	int currentShift = 0;
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

	existingTexture->width = width;
	existingTexture->height = height;
	existingTexture->internalFormat = internalFormat;
	existingTexture->fileName = fileName;

	FE_GL_ERROR(glGenTextures(1, &existingTexture->textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, existingTexture->textureID));

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (existingTexture->magFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	int maxDimention = std::max(existingTexture->width, existingTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	glTexStorage2D(GL_TEXTURE_2D, mipCount, existingTexture->internalFormat, existingTexture->width, existingTexture->height);

	if (existingTexture->mipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	int mipW = existingTexture->width / 2;
	int mipH = existingTexture->height / 2;
	for (size_t i = 0; i < mipCount; i++)
	{
		int size = *(int*)(&fileData[currentShift]);
		currentShift += 4;

		if (i == 0)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, existingTexture->width, existingTexture->height, existingTexture->internalFormat, size, (void*)(&fileData[currentShift]));
		}
		else
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipW, mipH, existingTexture->internalFormat, size, (void*)(&fileData[currentShift]));

			mipW = mipW / 2;
			mipH = mipH / 2;
		}

		currentShift += size;
	}
	delete[] fileData;
	delete[] textureName;
}

FETexture* FEResourceManager::LoadFETexture(const char* fileName, std::string Name)
{
	std::fstream file;
	file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	char* fileData = new char[int(fileSize)];
	file.read(fileData, fileSize);
	file.close();

	int currentShift = 0;
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

	FETexture* newTexture = createTexture(textureName);
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

	int maxDimention = std::max(newTexture->width, newTexture->height);
	size_t mipCount = size_t(floor(log2(maxDimention)) + 1);
	glTexStorage2D(GL_TEXTURE_2D, mipCount, newTexture->internalFormat, newTexture->width, newTexture->height);

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
			glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newTexture->width, newTexture->height, newTexture->internalFormat, size, (void*)(&fileData[currentShift]));
		}
		else
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipW, mipH, newTexture->internalFormat, size, (void*)(&fileData[currentShift]));
			
			mipW = mipW / 2;
			mipH = mipH / 2;
		}

		currentShift += size;
	}
	delete[] fileData;
	delete[] textureName;

	return newTexture;
}

FETexture* FEResourceManager::LoadFETextureStandAlone(const char* fileName, std::string Name)
{
	FETexture* temp = LoadFETexture(fileName, Name);
	textures.erase(temp->getName());
	return temp;
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::string Name)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	FEMesh* newMesh = createMesh(vaoID, positions.size() / 3, FE_POSITION, FEAABB(positions), Name);
	return newMesh;
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::string Name)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	FEMesh* newMesh = createMesh(vaoID, positions.size() / 3, FE_POSITION | FE_NORMAL, FEAABB(positions), Name);
	return newMesh;
}

FEMesh* FEResourceManager::rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index, std::string Name)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index.size(), index.data(), GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangents.size(), tangents.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UV.size(), UV.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	FEMesh* newMesh = createMesh(vaoID, index.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB(positions), Name);
	return newMesh;
}

FEMesh* FEResourceManager::rawDataToMesh(float* positions, int posSize, float* UV, int UVSize, float* normals, int normSize,
										 float* tangents, int tanSize, int* indices, int indexSize, std::string Name)
{
	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	FEMesh* newMesh = createMesh(vaoID, indexSize, FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB(), Name);
	return newMesh;
}

FEMesh* FEResourceManager::rawObjDataToMesh()
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();

	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint vboID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * objLoader.fInd.size(), objLoader.fInd.data(), GL_STATIC_DRAW));

	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fVerC.size(), objLoader.fVerC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// normals
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fNorC.size(), objLoader.fNorC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// tangents
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTanC.size(), objLoader.fTanC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// UV
	FE_GL_ERROR(glGenBuffers(1, &vboID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vboID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * objLoader.fTexC.size(), objLoader.fTexC.data(), GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	FE_GL_ERROR(glBindVertexArray(0));

	FEMesh* newMesh = createMesh(vaoID, objLoader.fInd.size(), FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX, FEAABB(objLoader.fVerC));
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
}

FEResourceManager::FEResourceManager()
{
	noTexture = new FETexture();
	LoadFETexture("noTexture.FETexture", noTexture);

	loadStandardMaterial();
	loadStandardMeshes();
}

FEResourceManager::~FEResourceManager()
{
	clear();
}

FEMesh* FEResourceManager::LoadOBJMesh(const char* fileName, std::string Name, const char* saveFEMeshTo)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.readFile(fileName);
	FEMesh* newMesh = rawObjDataToMesh();

	if (Name.size() == 0)
		Name = getFileNameFromFilePath(fileName);

	if (meshes.find(Name) != meshes.end())
		Name = "mesh_" + std::to_string(meshes.size());
	
	// in rawObjDataToMesh() hidden FEMesh allocation and it will go to hash table so we need to use setMeshName() not setName.
	setMeshName(newMesh, Name);
	newMesh->fileName = fileName;

	meshes[newMesh->getName()] = newMesh;
	
	if (saveFEMeshTo != nullptr)
	{
		saveFEMesh((saveFEMeshTo + Name + std::string(".model")).c_str());
		newMesh->fileName = (Name + std::string(".model")).c_str();
	}
		
	return newMesh;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* fileName, std::string Name)
{
	std::fstream file;

	file.open(fileName, std::ios::in | std::ios::binary);
	char * buffer = new char[4];

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
									Name);

	delete[] buffer;
	delete[] vertexBuffer;
	delete[] texBuffer;
	delete[] normBuffer;
	delete[] tangBuffer;
	delete[] indexBuffer;

	newMesh->AABB = meshAABB;
	newMesh->setName(Name);
	newMesh->fileName = fileName;

	meshes[newMesh->getName()] = newMesh;

	return newMesh;
}

FEPostProcess* FEResourceManager::createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name)
{
	return new FEPostProcess(getMesh("plane"), ScreenWidth, ScreenHeight, Name);
}

FEMaterial* FEResourceManager::createMaterial(std::string Name)
{
	if (Name.size() == 0 || materials.find(Name) != materials.end())
	{
		size_t nextID = materials.size();
		size_t index = 0;
		Name = "material_" + std::to_string(nextID + index);
		while (materials.find(Name) != materials.end())
		{
			index++;
			Name = "material_" + std::to_string(nextID + index);
		}
	}

	materials[Name] = new FEMaterial();
	materials[Name]->setName(Name);
	return materials[Name];
}

bool FEResourceManager::setMaterialName(FEMaterial* Material, std::string MaterialName)
{
	if (MaterialName.size() == 0 || materials.find(MaterialName) != materials.end())
		return false;

	materials.erase(Material->getName());
	materials[MaterialName] = Material;

	Material->setName(MaterialName);
	return true;
}

FEEntity* FEResourceManager::createEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	return new FEEntity(Mesh, Material, Name);
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
		if (material->getName().size() == 0 || standardMaterials.find(material->getName()) != standardMaterials.end())
		{
			size_t nextID = standardMaterials.size();
			size_t index = 0;
			material->setName("material_" + std::to_string(nextID + index));
			while (standardMaterials.find(material->getName()) != standardMaterials.end())
			{
				index++;
				material->setName("material_" + std::to_string(nextID + index));
			}
		}

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
	newMaterial->shader = new FEShader(FESolidColorVS, FESolidColorFS);
	FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	newMaterial->addParameter(color);

	makeMaterialStandard(newMaterial);
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
}

// save model raw data to FocalEngine binary file format
void FEResourceManager::saveFEMesh(const char* fileName)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	std::fstream file;
	
	file.open(fileName, std::ios::out | std::ios::binary);
	int count = objLoader.fVerC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fVerC.data(), sizeof(float) * objLoader.fVerC.size());

	count = objLoader.fTexC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fTexC.data(), sizeof(float) * objLoader.fTexC.size());

	count = objLoader.fNorC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fNorC.data(), sizeof(float) * objLoader.fNorC.size());

	count = objLoader.fTanC.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fTanC.data(), sizeof(float) * objLoader.fTanC.size());

	count = objLoader.fInd.size();
	file.write((char*)&count, sizeof(int));
	file.write((char*)objLoader.fInd.data(), sizeof(int) * objLoader.fInd.size());

	FEAABB tempAABB(objLoader.fVerC);
	file.write((char*)&tempAABB.min[0], sizeof(float));
	file.write((char*)&tempAABB.min[1], sizeof(float));
	file.write((char*)&tempAABB.min[2], sizeof(float));

	file.write((char*)&tempAABB.max[0], sizeof(float));
	file.write((char*)&tempAABB.max[1], sizeof(float));
	file.write((char*)&tempAABB.max[2], sizeof(float));

	file.close();
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
	meshes.erase(mesh->getName());
	delete mesh;
}
