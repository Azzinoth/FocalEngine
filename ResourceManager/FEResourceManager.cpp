#define STB_IMAGE_IMPLEMENTATION
#include "FEResourceManager.h"
using namespace FocalEngine;

FEResourceManager* FEResourceManager::Instance = nullptr;

FETexture* FEResourceManager::CreateTexture(std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedTexture";

	FETexture* NewTexture = new FETexture(Name);
	if (!ForceObjectID.empty())
		NewTexture->SetID(ForceObjectID);
	Textures[NewTexture->GetObjectID()] = NewTexture;

	return NewTexture;
}

bool FEResourceManager::MakeTextureStandard(FETexture* Texture)
{
	if (Texture == nullptr)
	{
		LOG.Add("texture is nullptr in function FEResourceManager::makeTextureStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardTextures.find(Texture->GetObjectID()) == StandardTextures.end())
	{
		if (Textures.find(Texture->GetObjectID()) != Textures.end())
			Textures.erase(Texture->GetObjectID());
		StandardTextures[Texture->GetObjectID()] = Texture;

		return true;
	}

	return false;
}

FEMesh* FEResourceManager::CreateMesh(const GLuint VaoID, const unsigned int VertexCount, const int VertexBuffersTypes, const FEAABB AABB, std::string Name)
{
	if (Name.empty())
		Name = "unnamedMesh";

	FEMesh* NewMesh = new FEMesh(VaoID, VertexCount, VertexBuffersTypes, AABB, Name);
	NewMesh->SetName(Name);
	Meshes[NewMesh->GetObjectID()] = NewMesh;

	return NewMesh;
}

bool FEResourceManager::MakeMeshStandard(FEMesh* Mesh)
{
	if (Mesh == nullptr)
	{
		LOG.Add("mesh is nullptr in function FEResourceManager::makeMeshStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardMeshes.find(Mesh->GetObjectID()) == StandardMeshes.end())
	{
		if (Meshes.find(Mesh->GetObjectID()) != Meshes.end())
			Meshes.erase(Mesh->GetObjectID());
		StandardMeshes[Mesh->GetObjectID()] = Mesh;

		return true;
	}

	return false;
}

FETexture* FEResourceManager::LoadPNGTexture(const char* FileName, const std::string Name)
{
	std::vector<unsigned char> RawFileData;
	std::ifstream File(FileName, std::ios::binary);
	if (!File)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function FEResourceManager::LoadPNGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardTextures.empty())
		{
			return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	File.unsetf(std::ios::skipws);
	RawFileData.insert(RawFileData.begin(),std::istream_iterator<unsigned char>(File), std::istream_iterator<unsigned char>());

	std::vector<unsigned char> RawExtractedData;
	unsigned int Width, Height;
	lodepng::State State;

	unsigned int Error = lodepng::decode(RawExtractedData, Width, Height, State, RawFileData);
	if (Error != 0)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function FEResourceManager::LoadPNGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardTextures.empty())
		{
			return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	bool bUsingAlpha = false;
	for (size_t i = 3; i < RawExtractedData.size(); i+=4)
	{
		if (RawExtractedData[i] != 255)
		{
			bUsingAlpha = true;
			break;
		}
	}

	FETexture* NewTexture = CreateTexture(Name);
	NewTexture->Width = Width;
	NewTexture->Height = Height;

	if (State.info_png.color.bitdepth == 16 && State.info_png.color.colortype == LCT_GREY)
	{
		// Using lodepng second time to decode to a proper format.
		RawExtractedData.clear();
		lodepng::State NewState;
		NewState.info_raw.colortype = LCT_GREY;
		NewState.info_raw.bitdepth = 16;
		lodepng::decode(RawExtractedData, Width, Height, NewState, (unsigned char*)RawFileData.data(), RawFileData.size());

		NewTexture->InternalFormat = GL_R16;
		NewTexture->MagFilter = FE_LINEAR;
		NewTexture->FileName = FileName;

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
		// lodepng returns 16-bit data with different bytes order that OpenGL expects.
		FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, TRUE));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RED, GL_UNSIGNED_SHORT, RawExtractedData.data());
		FE_GL_ERROR(glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE));

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}
	else
	{
		NewTexture->InternalFormat = bUsingAlpha ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawExtractedData.data());

		if (NewTexture->MipEnabled)
		{
			FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
		}

		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		if (NewTexture->MagFilter == FE_LINEAR)
		{
			FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}
		else
		{
			FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		}
	}
	
	NewTexture->FileName = FileName;

	if (Name.empty())
	{
		const std::string FilePath = NewTexture->FileName;
		std::size_t Index = FilePath.find_last_of("/\\");
		const std::string NewFileName = FilePath.substr(Index + 1);
		Index = NewFileName.find_last_of(".");
		const std::string FileNameWithOutExtention = NewFileName.substr(0, Index);
		NewTexture->SetName(FileNameWithOutExtention);
	}

	return NewTexture;
}

void FEResourceManager::SaveFETexture(FETexture* Texture, const char* FileName)
{
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, Texture->TextureID));

	GLint ImgSize = 0;
	std::fstream File;
	File.open(FileName, std::ios::out | std::ios::binary);

	// Version of FETexture File type
	float Version = FE_TEXTURE_VERSION;
	File.write((char*)&Version, sizeof(float));

	int ObjectIDSize = static_cast<int>(Texture->GetObjectID().size() + 1);
	File.write((char*)&ObjectIDSize, sizeof(int));
	File.write((char*)Texture->GetObjectID().c_str(), sizeof(char) * ObjectIDSize);

	File.write((char*)&Texture->Width, sizeof(int));
	File.write((char*)&Texture->Height, sizeof(int));
	File.write((char*)&Texture->InternalFormat, sizeof(int));

	int NameSize = static_cast<int>(Texture->GetName().size() + 1);
	File.write((char*)&NameSize, sizeof(int));

	char* TextureName = new char[NameSize];
	strcpy_s(TextureName, NameSize, Texture->GetName().c_str());
	File.write((char*)TextureName, sizeof(char) * NameSize);

	if (Texture->InternalFormat == GL_R16 || Texture->InternalFormat == GL_RED || Texture->InternalFormat == GL_RGBA)
	{
		size_t DataSize = 0;
		unsigned char* Pixels = Texture->GetRawData(&DataSize);

		File.write((char*)&DataSize, sizeof(int));
		File.write((char*)Pixels, sizeof(char) * DataSize);
		File.close();

		delete[] Pixels;
		return;
	}

	const int MaxDimention = std::max(Texture->Width, Texture->Height);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
	char** PixelData = new char*[MipCount];

	for (size_t i = 0; i < MipCount; i++)
	{
		FE_GL_ERROR(glGetTexLevelParameteriv(GL_TEXTURE_2D, static_cast<GLint>(i), GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &ImgSize));

		char* Pixels = new char[ImgSize * 2];
		for (size_t j = 0; j < static_cast<size_t>(ImgSize * 2); j++)
		{
			Pixels[j] = ' ';
		}

		char* AdditionalTestPixels = new char[ImgSize * 2];
		for (size_t j = 0; j < static_cast<size_t>(ImgSize * 2); j++)
		{
			AdditionalTestPixels[j] = '1';
		}

		FE_GL_ERROR(glGetCompressedTexImage(GL_TEXTURE_2D, static_cast<GLint>(i), Pixels));
		FE_GL_ERROR(glGetCompressedTexImage(GL_TEXTURE_2D, static_cast<GLint>(i), AdditionalTestPixels));

		int RealSize = 0;
		for (size_t j = ImgSize * 2 - 1; j > 0 ; j--)
		{
			if (Pixels[j] != ' ')
			{
				RealSize = static_cast<int>(j + 1);
				break;
			}
		}

		int AdditionalRealSize = 0;
		for (size_t j = ImgSize * 2 - 1; j > 0; j--)
		{
			if (AdditionalTestPixels[j] != '1')
			{
				AdditionalRealSize = static_cast<int>(j + 1);
				break;
			}
		}

		RealSize = std::max(RealSize, AdditionalRealSize);

		PixelData[i] = new char[RealSize];
		memcpy(PixelData[i], Pixels, RealSize);
		delete[] Pixels;
		delete[] AdditionalTestPixels;

		File.write((char*)&RealSize, sizeof(int));
		File.write((char*)PixelData[i], sizeof(char) * RealSize);
	}

	File.close();

	for (size_t i = 0; i < MipCount; i++)
	{
		delete[] PixelData[i];
	}
	delete[] PixelData;
}

FETexture* FEResourceManager::RawDataToFETexture(unsigned char* TextureData, const int Width, const int Height, GLint Internalformat, const GLenum Format, GLenum Type)
{
	FETexture* NewTexture = CreateTexture();
	NewTexture->Width = Width;
	NewTexture->Height = Height;

	if (Format == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		NewTexture->InternalFormat = GL_RED;
	}
	else
	{
		NewTexture->InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < static_cast<size_t>(Width * Height * 4); i += 4)
		{
			if (TextureData[i] != 255)
			{
				NewTexture->InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, Format, GL_UNSIGNED_BYTE, TextureData);
	
	if (NewTexture->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));// to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (NewTexture->MagFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	if (Format == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	return NewTexture;
}

struct LoadTextureAsyncInfo
{
	std::string FileName;
	FETexture* NewTexture;
	char* FileData;
};

void LoadTextureFileAsync(void* InputData, void* OutputData)
{
	const LoadTextureAsyncInfo* Input = reinterpret_cast<LoadTextureAsyncInfo*>(InputData);
	std::fstream File;
	File.open(Input->FileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	const std::streamsize FileSize = File.tellg();
	if (FileSize <= 0)
	{
		LoadTextureAsyncInfo* Output = reinterpret_cast<LoadTextureAsyncInfo*>(OutputData);
		Output->FileData = nullptr;
		Output->NewTexture = Input->NewTexture;

		delete InputData;
		return;
	}

	File.seekg(0, std::ios::beg);
	char* FileData = new char[static_cast<int>(FileSize)];
	File.read(FileData, FileSize);
	File.close();

	LoadTextureAsyncInfo* Output = reinterpret_cast<LoadTextureAsyncInfo*>(OutputData);
	Output->FileData = FileData;
	Output->NewTexture = Input->NewTexture;

	delete InputData;
}

void FEResourceManager::LoadTextureFileAsyncCallBack(void* OutputData)
{
	const LoadTextureAsyncInfo* Input = reinterpret_cast<LoadTextureAsyncInfo*>(OutputData);

	// File was not found, or TextureIterator can't be read.
	if (Input->FileData == nullptr)
	{
		// Get info about problematic texture.
		const FETexture* NotLoadedTexture = Input->NewTexture;
		// We will spill out error into a log.
		LOG.Add("FEResourceManager::updateAsyncLoadedResources texture with ID: " + NotLoadedTexture->GetObjectID() + " was not loaded!", "FE_LOG_LOADING", FE_LOG_ERROR);
		// And delete entry for that texture in a general list of textures.
		// That will prevent TextureIterator from saving in a scene File.
		RESOURCE_MANAGER.DeleteFETexture(NotLoadedTexture);
		//textures.erase(notLoadedTexture->getObjectID());
	}
	else
	{
		const FETexture* NewlyCreatedTexture = RESOURCE_MANAGER.LoadFETexture(Input->FileData, "", Input->NewTexture);

		// If some material uses this texture we should set dirty flag.
		// Game model will updated as a consequences.
		const std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();

		for (size_t p = 0; p < MaterialList.size(); p++)
		{
			FEMaterial* CurrentMaterial = RESOURCE_MANAGER.GetMaterial(MaterialList[p]);
			if (CurrentMaterial->IsTextureInList(NewlyCreatedTexture))
				CurrentMaterial->SetDirtyFlag(true);
		}
	}

	delete Input->FileData;
	delete Input;
}

FETexture* FEResourceManager::LoadFETextureAsync(const char* FileName, const std::string Name, FETexture* ExistingTexture, const std::string ForceObjectID)
{
	FETexture* NewTexture = CreateTexture(Name, ForceObjectID);
	FE_GL_ERROR(glDeleteTextures(1, &NewTexture->TextureID));
	NewTexture->TextureID = NoTexture->TextureID;
	NewTexture->Width = NoTexture->Width;
	NewTexture->Height = NoTexture->Height;
	NewTexture->InternalFormat = NoTexture->InternalFormat;
	NewTexture->FileName = NoTexture->FileName;

	LoadTextureAsyncInfo* InputData = new LoadTextureAsyncInfo();
	InputData->FileName = FileName;
	InputData->NewTexture = NewTexture;
	LoadTextureAsyncInfo* OutputData = new LoadTextureAsyncInfo();

	THREAD_POOL.Execute(LoadTextureFileAsync, InputData, OutputData, &LoadTextureFileAsyncCallBack);

	return NewTexture;
}

FETexture* FEResourceManager::LoadFETexture(const char* FileName, const std::string Name, FETexture* ExistingTexture)
{
	std::fstream File;
	File.open(FileName, std::ios::in | std::ios::binary | std::ios::ate);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadFETexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return this->NoTexture;
	}
	
	File.seekg(0, std::ios::beg);
	char* FileData = new char[static_cast<int>(FileSize)];
	File.read(FileData, FileSize);
	File.close();

	FETexture* Result = LoadFETexture(FileData, Name, ExistingTexture);
	delete[] FileData;

	return Result;
}

FETexture* FEResourceManager::LoadFETexture(char* FileData, std::string Name, FETexture* ExistingTexture)
{
	int CurrentShift = 0;
	// Version of FETexture File type
	const float Version = *(float*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	if (Version != FE_TEXTURE_VERSION)
	{
		LOG.Add(std::string("can't load fileData: in function FEResourceManager::LoadFETexture. FileData was created in different version of engine!"), "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardTextures.empty())
		{
			return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	const int ObjectIDSize = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;

	char* ObjectID = new char[ObjectIDSize];
	strcpy_s(ObjectID, ObjectIDSize, (char*)(&FileData[CurrentShift]));
	CurrentShift += ObjectIDSize;

	const int width = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	const int height = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	const int InternalFormat = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	const int NameSize = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;

	char* TextureName = new char[NameSize];
	strcpy_s(TextureName, NameSize, (char*)(&FileData[CurrentShift]));
	CurrentShift += NameSize;

	FETexture* NewTexture = nullptr;
	if (ExistingTexture != nullptr)
	{
		NewTexture = ExistingTexture;
		NewTexture->SetName(TextureName);
		FE_GL_ERROR(glGenTextures(1, &NewTexture->TextureID));
	}
	else
	{
		NewTexture = CreateTexture(TextureName);
	}

	NewTexture->Width = width;
	NewTexture->Height = height;
	NewTexture->InternalFormat = InternalFormat;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));

	if (NewTexture->InternalFormat == GL_RED || NewTexture->InternalFormat == GL_RGBA || NewTexture->InternalFormat == GL_R16)
	{
		int Size = *(int*)(&FileData[CurrentShift]);
		CurrentShift += 4;
		
		NewTexture->UpdateRawData((unsigned char*)(&FileData[CurrentShift]));
	}
	else
	{
		const int MaxDimention = std::max(NewTexture->Width, NewTexture->Height);
		const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height));

		int MipW = NewTexture->Width / 2;
		int MipH = NewTexture->Height / 2;
		for (size_t i = 0; i < MipCount; i++)
		{
			const int Size = *(int*)(&FileData[CurrentShift]);
			CurrentShift += 4;

			if (i == 0)
			{
				FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NewTexture->Width, NewTexture->Height, NewTexture->InternalFormat, Size, static_cast<void*>(&FileData[CurrentShift])));
			}
			else
			{
				FE_GL_ERROR(glCompressedTexSubImage2D(GL_TEXTURE_2D, static_cast<int>(i), 0, 0, MipW, MipH, NewTexture->InternalFormat, Size, static_cast<void*>(&FileData[CurrentShift])));

				MipW = MipW / 2;
				MipH = MipH / 2;

				if (MipW <= 0 || MipH <= 0)
					break;
			}

			CurrentShift += Size;
		}
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (NewTexture->MagFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	if (NewTexture->MipEnabled)
	{
		//FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	// Overwrite objectID with objectID from File.
	if (ObjectID != nullptr)
	{
		const std::string OldID = NewTexture->GetObjectID();
		NewTexture->SetID(ObjectID);

		if (Textures.find(OldID) != Textures.end())
		{
			Textures.erase(OldID);
			Textures[NewTexture->GetObjectID()] = NewTexture;
		}
	}

	delete[] ObjectID;
	delete[] TextureName;

	return NewTexture;
}

FETexture* FEResourceManager::LoadFETextureUnmanaged(const char* FileName, const std::string Name)
{
	FETexture* NewTexture = LoadFETexture(FileName, Name);
	Textures.erase(NewTexture->GetObjectID());
	return NewTexture;
}

FEMesh* FEResourceManager::RawDataToMesh(std::vector<float>& Positions, std::vector<float>& Normals, std::vector<float>& Tangents, std::vector<float>& UV, std::vector<int>& Index, std::string Name)
{
	return RawDataToMesh(Positions.data(), static_cast<int>(Positions.size()), UV.data(), static_cast<int>(UV.size()), Normals.data(), static_cast<int>(Normals.size()), Tangents.data(), static_cast<int>(Tangents.size()), Index.data(), static_cast<int>(Index.size()), nullptr, 0, nullptr, 0, 0, Name);
}

FEMesh* FEResourceManager::RawDataToMesh(float* Positions, const int PosSize,
										 float* UV, const int UVSize,
										 float* Normals, const int NormSize,
										 float* Tangents, const int TanSize,
										 int* Indices, const int IndexSize,
										 float* Colors, int ColorSize,
										 float* MatIndexs, const int MatIndexsSize, const int MatCount, 
										 const std::string Name)
{
	int VertexType = FE_POSITION | FE_INDEX;

	GLuint VaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &VaoID));
	FE_GL_ERROR(glBindVertexArray(VaoID));

	GLuint IndicesBufferID;
	// Index
	FE_GL_ERROR(glGenBuffers(1, &IndicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * IndexSize, Indices, GL_STATIC_DRAW));

	GLuint PositionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &PositionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PositionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PosSize, Positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint ColorsBufferID = 0;
	if (Colors != nullptr)
	{
		VertexType |= FE_COLOR;
		// colors
		FE_GL_ERROR(glGenBuffers(1, &ColorsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ColorsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ColorSize, Colors, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint NormalsBufferID = 0;
	if (Normals != nullptr)
	{
		VertexType |= FE_NORMAL;
		// normals
		FE_GL_ERROR(glGenBuffers(1, &NormalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, NormalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NormSize, Normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint TangentsBufferID = 0;
	if (Tangents != nullptr)
	{
		VertexType |= FE_TANGENTS;
		// tangents
		FE_GL_ERROR(glGenBuffers(1, &TangentsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, TangentsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TanSize, Tangents, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr)
	{
		VertexType |= FE_UV;
		// UV
		FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint MaterialsIndicesBufferID = -1;
	if (MatIndexs != nullptr && MatIndexsSize > 1)
	{
		// Material ID
		FE_GL_ERROR(glGenBuffers(1, &MaterialsIndicesBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, MaterialsIndicesBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MatIndexsSize, MatIndexs, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(5/*FE_MATINDEX*/, 1, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	//int vertexType = FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX;
	if (MatCount > 1)
	{
		VertexType |= FE_MATINDEX;
	}

	FEMesh* NewMesh = CreateMesh(VaoID, IndexSize, VertexType, FEAABB(Positions, PosSize), Name);
	NewMesh->IndicesCount = IndexSize;
	NewMesh->IndicesBufferID = IndicesBufferID;

	NewMesh->PositionsCount = PosSize;
	NewMesh->PositionsBufferID = PositionsBufferID;

	NewMesh->ColorCount = ColorSize;
	NewMesh->ColorBufferID = ColorsBufferID;

	NewMesh->NormalsCount = NormSize;
	NewMesh->NormalsBufferID = NormalsBufferID;

	NewMesh->TangentsCount = TanSize;
	NewMesh->TangentsBufferID = TangentsBufferID;

	NewMesh->UVCount = UVSize;
	NewMesh->UVBufferID = UVBufferID;

	NewMesh->MaterialsIndicesCount = MatIndexsSize;
	NewMesh->MaterialsIndicesBufferID = MaterialsIndicesBufferID;

	NewMesh->MaterialsCount = MatCount;

	return NewMesh;
}

void FEResourceManager::LoadStandardMeshes()
{
	if (Meshes.find("84251E6E0D0801363579317R"/*"cube"*/) != Meshes.end())
		return;

	std::vector<int> CubeIndices = {
		4, 2, 0, 9, 7, 3, 6, 5,	20,	21,	15,
		22,	10,	12,	18,	8, 1, 19, 4, 17, 2,
		9, 23, 7, 6, 13, 5, 24, 16, 15, 10,
		14, 12, 8, 11, 1
	};

	std::vector<float> CubePositions = {
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

	std::vector<float> CubeNormals = {
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

	std::vector<float> CubeTangents = {
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

	std::vector<float> CubeUV = {
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

	StandardMeshes["84251E6E0D0801363579317R"] = RawDataToMesh(CubePositions, CubeNormals, CubeTangents, CubeUV, CubeIndices, "cube");
	Meshes.erase(StandardMeshes["84251E6E0D0801363579317R"/*"cube"*/]->GetObjectID());
	StandardMeshes["84251E6E0D0801363579317R"/*"cube"*/]->SetID("84251E6E0D0801363579317R"/*"cube"*/);
	StandardMeshes["84251E6E0D0801363579317R"/*"cube"*/]->SetName("FECube");

	std::vector<int> PlaneIndices = {
		0, 1, 2, 3, 0, 2
	};

	std::vector<float> PlanePositions = {
		-1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	std::vector<float> PlaneNormals = {
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	std::vector<float> PlaneTangents = {
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f
	};

	std::vector<float> PlaneUV = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	StandardMeshes["1Y251E6E6T78013635793156"] = RawDataToMesh(PlanePositions, PlaneNormals, PlaneTangents, PlaneUV, PlaneIndices, "plane");
	Meshes.erase(StandardMeshes["1Y251E6E6T78013635793156"/*"plane"*/]->GetObjectID());
	StandardMeshes["1Y251E6E6T78013635793156"/*"plane"*/]->SetID("1Y251E6E6T78013635793156"/*"plane"*/);
	StandardMeshes["1Y251E6E6T78013635793156"/*"plane"*/]->SetName("FEPlane");

	StandardMeshes["7F251E3E0D08013E3579315F"] = LoadFEMesh((ResourcesFolder + "7F251E3E0D08013E3579315F.model").c_str(), "sphere");
	Meshes.erase(StandardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->GetObjectID());
	StandardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->SetID("7F251E3E0D08013E3579315F"/*"sphere"*/);
	StandardMeshes["7F251E3E0D08013E3579315F"/*"sphere"*/]->SetName("FESphere");
}

FEResourceManager::FEResourceManager()
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);
	NoTexture = LoadFETexture((ResourcesFolder + "48271F005A73241F5D7E7134.texture").c_str(), "noTexture");
	MakeTextureStandard(NoTexture);
	FETexture::AddToNoDeletingList(NoTexture->GetTextureID());

	FEShader* NewShader = CreateShader("FECombineFrameBuffers", LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																	     LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_CombineFrameBuffers_FS.glsl").c_str()).c_str());
	NewShader->SetID("5C267A01466A545E7D1A2E66"/*FECombineFrameBuffers*/);
	MakeShaderStandard(NewShader);

	LoadStandardMaterial();
	LoadStandardMeshes();
	LoadStandardGameModels();
	LoadStandardPrefabs();
}

FEResourceManager::~FEResourceManager()
{
	Clear();
}

std::vector<FEObject*> FEResourceManager::ImportOBJ(const char* FileName, const bool bForceOneMesh)
{
	FEObjLoader& OBJLoader = FEObjLoader::getInstance();
	OBJLoader.bForceOneMesh = bForceOneMesh;
	OBJLoader.ReadFile(FileName);

	std::vector<FEObject*> Result;
	for (size_t i = 0; i < OBJLoader.LoadedObjects.size(); i++)
	{
		const std::string name = GetFileNameFromFilePath(FileName) + "_" + std::to_string(i);


		Result.push_back(RawDataToMesh(OBJLoader.LoadedObjects[i]->FVerC.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->FVerC.size()),
									   OBJLoader.LoadedObjects[i]->FTexC.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->FTexC.size()),
									   OBJLoader.LoadedObjects[i]->FNorC.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->FNorC.size()),
									   OBJLoader.LoadedObjects[i]->FTanC.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->FTanC.size()),
								       OBJLoader.LoadedObjects[i]->FInd.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->FInd.size()),
									   nullptr, 0,
									   OBJLoader.LoadedObjects[i]->MatIDs.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->MatIDs.size()), static_cast<int>(OBJLoader.LoadedObjects[i]->MaterialRecords.size()), name));
	
	
		// in rawDataToMesh() hidden FEMesh allocation and TextureIterator will go to hash table so we need to use setMeshName() not setName.
		Result.back()->SetName(name);
		Meshes[Result.back()->GetObjectID()] = reinterpret_cast<FEMesh*>(Result.back());
	}

	CreateMaterialsFromOBJData(Result);

	return Result;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* FileName, const std::string Name)
{
	std::fstream File;

	File.open(FileName, std::ios::in | std::ios::binary);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardMeshes.empty())
		{
			return GetMesh("84251E6E0D0801363579317R"/*"cube"*/);
		}
		else
		{
			return nullptr;
		}
	}

	char* Buffer = new char[4];

	// Version of FEMesh File type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version != FE_MESH_VERSION)
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadFEMesh. File was created in different version of engine!", "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardMeshes.empty())
		{
			return GetMesh("84251E6E0D0801363579317R"/*"cube"*/);
		}
		else
		{
			return nullptr;
		}
	}

	File.read(Buffer, 4);
	const int ObjectIDSize = *(int*)Buffer;

	char* ObjectID = new char[ObjectIDSize + 1];
	File.read(ObjectID, ObjectIDSize);
	ObjectID[ObjectIDSize] = '\0';

	File.read(Buffer, 4);
	const int MeshNameSize = *(int*)Buffer;
	char* MeshName = new char[MeshNameSize + 1];
	File.read(MeshName, MeshNameSize);
	MeshName[MeshNameSize] = '\0';
	
	File.read(Buffer, 4);
	const int VertexCout = *(int*)Buffer;
	char* VertexBuffer = new char[VertexCout * 4];
	File.read(VertexBuffer, VertexCout * 4);

	File.read(Buffer, 4);
	const int TexCout = *(int*)Buffer;
	char* TexBuffer = new char[TexCout * 4];
	File.read(TexBuffer, TexCout * 4);

	File.read(Buffer, 4);
	const int NormCout = *(int*)Buffer;
	char* NormBuffer = new char[NormCout * 4];
	File.read(NormBuffer, NormCout * 4);

	File.read(Buffer, 4);
	const int TangCout = *(int*)Buffer;
	char* TangBuffer = new char[TangCout * 4];
	File.read(TangBuffer, TangCout * 4);

	File.read(Buffer, 4);
	const int IndexCout = *(int*)Buffer;
	char* IndexBuffer = new char[IndexCout * 4];
	File.read(IndexBuffer, IndexCout * 4);

	int MatIndexCout = 0;
	char* MatIndexBuffer = nullptr;

	File.read(Buffer, 4);
	const int MatCount = *(int*)Buffer;

	if (MatCount > 1)
	{
		File.read(Buffer, 4);
		MatIndexCout = *(int*)Buffer;
		MatIndexBuffer = new char[MatIndexCout * 4];
		File.read(MatIndexBuffer, MatIndexCout * 4);
	}

	FEAABB MeshAABB;
	for (int i = 0; i <= 2; i++)
	{
		File.read(Buffer, 4);
		MeshAABB.Min[i] = *(float*)Buffer;
	}

	for (int i = 0; i <= 2; i++)
	{
		File.read(Buffer, 4);
		MeshAABB.Max[i] = *(float*)Buffer;
	}

	File.close();

	FEMesh* NewMesh = RawDataToMesh((float*)VertexBuffer, VertexCout,
									(float*)TexBuffer, TexCout,
									(float*)NormBuffer, NormCout,
									(float*)TangBuffer, TangCout,
									(int*)IndexBuffer, IndexCout,
									nullptr, 0,
									(float*)MatIndexBuffer, MatIndexCout, MatCount,
									Name);

	const std::string OldID = NewMesh->ID;
	// overwrite objectID with objectID from File.
	if (ObjectID != nullptr)
	{
		NewMesh->SetID(ObjectID);
	}

	delete[] ObjectID;

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	NewMesh->AABB = MeshAABB;
	NewMesh->SetName(Name);
	
	Meshes.erase(OldID);
	Meshes[NewMesh->GetObjectID()] = NewMesh;

	return NewMesh;
}

FEMaterial* FEResourceManager::CreateMaterial(std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedMaterial";

	FEMaterial* NewMaterial = new FEMaterial(Name);
	if (!ForceObjectID.empty())
		NewMaterial->SetID(ForceObjectID);
	Materials[NewMaterial->GetObjectID()] = NewMaterial;
	
	return Materials[NewMaterial->GetObjectID()];
}

std::vector<std::string> FEResourceManager::GetMaterialList()
{
	FE_MAP_TO_STR_VECTOR(Materials)
}

std::vector<std::string> FEResourceManager::GetStandardMaterialList()
{
	FE_MAP_TO_STR_VECTOR(StandardMaterials)
}

FEMaterial* FEResourceManager::GetMaterial(const std::string ID)
{
	if (Materials.find(ID) == Materials.end())
	{
		if (StandardMaterials.find(ID) != StandardMaterials.end())
		{
			return StandardMaterials[ID];
		}

		return nullptr;
	}

	return Materials[ID];
}

std::vector<FEMaterial*> FEResourceManager::GetMaterialByName(const std::string Name)
{
	std::vector<FEMaterial*> Result;

	auto it = Materials.begin();
	while (it != Materials.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	it = StandardMaterials.begin();
	while (it != StandardMaterials.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	return Result;
}

std::string FEResourceManager::GetFileNameFromFilePath(const std::string FilePath)
{
	for (size_t i = FilePath.size() - 1; i > 0; i--)
	{
		if (FilePath[i] == '\\' || FilePath[i] == '/')
			return FilePath.substr(i + 1, FilePath.size() - 1 - i);
	}

	return std::string("");
}

std::vector<std::string> FEResourceManager::GetMeshList()
{
	FE_MAP_TO_STR_VECTOR(Meshes)
}

std::vector<std::string> FEResourceManager::GetStandardMeshList()
{
	FE_MAP_TO_STR_VECTOR(StandardMeshes)
}

FEMesh* FEResourceManager::GetMesh(const std::string ID)
{
	if (Meshes.find(ID) == Meshes.end())
	{
		if (StandardMeshes.find(ID) != StandardMeshes.end())
		{
			return StandardMeshes[ID];
		}

		return nullptr;
	}
	else
	{
		return Meshes[ID];
	}
}

std::vector<FEMesh*> FEResourceManager::GetMeshByName(const std::string Name)
{
	std::vector<FEMesh*> Result;

	auto it = Meshes.begin();
	while (it != Meshes.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	it = StandardMeshes.begin();
	while (it != StandardMeshes.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	return Result;
}

bool FEResourceManager::MakeMaterialStandard(FEMaterial* Material)
{
	if (Material == nullptr)
	{
		LOG.Add("material is nullptr in function FEResourceManager::makeMaterialStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardMaterials.find(Material->GetObjectID()) == StandardMaterials.end())
	{
		if (Materials.find(Material->GetObjectID()) != Materials.end())
			Materials.erase(Material->GetObjectID());
		StandardMaterials[Material->GetObjectID()] = Material;

		return true;
	}

	return false;
}

void FEResourceManager::LoadStandardMaterial()
{
	FEMaterial* NewMaterial = CreateMaterial("SolidColorMaterial");
	Materials.erase(NewMaterial->GetObjectID());
	NewMaterial->SetID("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);
	NewMaterial->Shader = CreateShader("FESolidColorShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_VS.glsl").c_str()).c_str(),
															 LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_FS.glsl").c_str()).c_str());
	NewMaterial->Shader->SetID("6917497A5E0C05454876186F");

	MakeShaderStandard(NewMaterial->Shader);
	const FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	NewMaterial->AddParameter(color);
	MakeMaterialStandard(NewMaterial);

	FEShader* FEPhongShader = CreateShader("FEPhongShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_VS.glsl").c_str()).c_str(),
															LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_FS.glsl").c_str()).c_str());
	Shaders.erase(FEPhongShader->GetObjectID());
	FEPhongShader->SetID("4C41665B5E125C2A07456E44"/*"FEPhongShader"*/);
	Shaders[FEPhongShader->GetObjectID()] = FEPhongShader;

	MakeShaderStandard(GetShader("4C41665B5E125C2A07456E44"/*"FEPhongShader"*/));

	// ****************************** PBR SHADER ******************************
	FEShader* PBRShader = CreateShader("FEPBRShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS_GBUFFER.glsl").c_str()).c_str(),
													  LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str()).c_str());

	FEShader* PBRShaderForward = CreateShader("FEPBRShaderForward", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str()).c_str(),
																	LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str()).c_str());

	Shaders.erase(PBRShaderForward->GetObjectID());
	PBRShaderForward->SetID("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
	Shaders[PBRShaderForward->GetObjectID()] = PBRShaderForward;

	MakeShaderStandard(GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/));

	FEShader* PBRGBufferShader = CreateShader("FEPBRGBufferShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str()).c_str(),
															        LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str()).c_str());

	Shaders.erase(PBRGBufferShader->GetObjectID());
	PBRGBufferShader->SetID("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/);
	Shaders[PBRGBufferShader->GetObjectID()] = PBRGBufferShader;

	MakeShaderStandard(GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));

	Shaders.erase(PBRShader->GetObjectID());
	PBRShader->SetID("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	Shaders[PBRShader->GetObjectID()] = PBRShader;

	MakeShaderStandard(GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/));

	FEShader* PBRInstancedShader = CreateShader("FEPBRInstancedShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str()).c_str(),
																		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str()).c_str());

	FEShader* PBRInstancedGBufferShader = CreateShader("FEPBRInstancedGBufferShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str()).c_str(),
																					  LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str()).c_str());

	Shaders.erase(PBRInstancedGBufferShader->GetObjectID());
	PBRInstancedGBufferShader->SetID("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/);
	Shaders[PBRInstancedGBufferShader->GetObjectID()] = PBRInstancedGBufferShader;

	MakeShaderStandard(GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/)); 

	Shaders.erase(PBRInstancedShader->GetObjectID());
	PBRInstancedShader->SetID("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
	Shaders[PBRInstancedShader->GetObjectID()] = PBRInstancedShader;

	MakeShaderStandard(GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/));

	NewMaterial = CreateMaterial("FEPBRBaseMaterial");
	Materials.erase(NewMaterial->GetObjectID());
	NewMaterial->SetID("61649B9E0F08013Q3939316C"/*"FEPBRBaseMaterial"*/);
	NewMaterial->Shader = GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	NewMaterial->SetAlbedoMap(NoTexture);
	MakeMaterialStandard(NewMaterial);
	// ****************************** PBR SHADER END ******************************

	// same as FERenderer::updateFogInShaders()
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogDensity", 0.007f);
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogGradient", 2.5f);
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("shadowBlurFactor", 1.0f);

	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogDensity", 0.007f);
	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogGradient", 2.5f);
	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("shadowBlurFactor", 1.0f);
}

void FEResourceManager::LoadStandardGameModels()
{
	FEGameModel* NewGameModel = new FEGameModel(GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/), GetMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/), "standardGameModel");
	NewGameModel->SetID("67251E393508013ZV579315F");
	MakeGameModelStandard(NewGameModel);
}

void FEResourceManager::Clear()
{
	auto MaterialIt = Materials.begin();
	while (MaterialIt != Materials.end())
	{
		delete MaterialIt->second;
		MaterialIt++;
	}
	Materials.clear();

	auto MeshIt = Meshes.begin();
	while (MeshIt != Meshes.end())
	{
		delete MeshIt->second;
		MeshIt++;
	}
	Meshes.clear();

	auto TextureIt = Textures.begin();
	while (TextureIt != Textures.end())
	{
		delete TextureIt->second;
		TextureIt++;
	}
	Textures.clear();

	auto GameModelIt = GameModels.begin();
	while (GameModelIt != GameModels.end())
	{
		delete GameModelIt->second;
		GameModelIt++;
	}
	GameModels.clear();

	auto PrefabIt = Prefabs.begin();
	while (PrefabIt != Prefabs.end())
	{
		delete PrefabIt->second;
		PrefabIt++;
	}
	Prefabs.clear();
}

void FEResourceManager::SaveFEMesh(FEMesh* Mesh, const char* FileName)
{
	std::fstream File;
	File.open(FileName, std::ios::out | std::ios::binary);

	// Version of FEMesh File type.
	float version = FE_MESH_VERSION;
	File.write((char*)&version, sizeof(float));

	int ObjectIDSize = static_cast<int>(Mesh->GetObjectID().size());
	File.write((char*)&ObjectIDSize, sizeof(int));
	File.write((char*)Mesh->GetObjectID().c_str(), sizeof(char) * ObjectIDSize);

	int NameSize = static_cast<int>(Mesh->GetName().size());
	File.write((char*)&NameSize, sizeof(int));
	File.write((char*)Mesh->GetName().c_str(), sizeof(char) * NameSize);

	int Count = Mesh->GetPositionsCount();
	float* Positions = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetPositionsBufferID(), 0, sizeof(float) * Count, Positions));
	File.write((char*)&Count, sizeof(int));
	File.write((char*)Positions, sizeof(float) * Count);

	Count = Mesh->GetUVCount();
	float* UV = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetUVBufferID(), 0, sizeof(float) * Count, UV));
	File.write((char*)&Count, sizeof(int));
	File.write((char*)UV, sizeof(float) * Count);

	Count = Mesh->GetNormalsCount();
	float* Normals = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetNormalsBufferID(), 0, sizeof(float) * Count, Normals));
	File.write((char*)&Count, sizeof(int));
	File.write((char*)Normals, sizeof(float) * Count);
	
	Count = Mesh->GetTangentsCount();
	float* Tangents = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetTangentsBufferID(), 0, sizeof(float) * Count, Tangents));
	File.write((char*)&Count, sizeof(int));
	File.write((char*)Tangents, sizeof(float) * Count);

	Count = Mesh->GetIndicesCount();
	int* Indices = new int[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetIndicesBufferID(), 0, sizeof(int) * Count, Indices));
	File.write((char*)&Count, sizeof(int));
	File.write((char*)Indices, sizeof(int) * Count);

	int MaterialCount = Mesh->MaterialsCount;
	File.write((char*)&MaterialCount, sizeof(int));
	
	if (MaterialCount > 1)
	{
		Count = Mesh->GetMaterialsIndicesCount();
		float* MatIndices = new float[Count];
		FE_GL_ERROR(glGetNamedBufferSubData(Mesh->GetMaterialsIndicesBufferID(), 0, sizeof(float) * Count, MatIndices));
		File.write((char*)&Count, sizeof(int));
		File.write((char*)MatIndices, sizeof(float) * Count);
	}

	FEAABB TempAABB(Positions, Mesh->GetPositionsCount());
	File.write((char*)&TempAABB.Min[0], sizeof(float));
	File.write((char*)&TempAABB.Min[1], sizeof(float));
	File.write((char*)&TempAABB.Min[2], sizeof(float));

	File.write((char*)&TempAABB.Max[0], sizeof(float));
	File.write((char*)&TempAABB.Max[1], sizeof(float));
	File.write((char*)&TempAABB.Max[2], sizeof(float));

	File.close();

	delete[] Positions;
	delete[] UV;
	delete[] Normals;
	delete[] Tangents;
	delete[] Indices;
}

std::vector<std::string> FEResourceManager::GetTextureList()
{
	FE_MAP_TO_STR_VECTOR(Textures)
}

FETexture* FEResourceManager::GetTexture(const std::string ID)
{
	if (Textures.find(ID) == Textures.end())
		return nullptr;

	return Textures[ID];
}

std::vector<FETexture*> FEResourceManager::GetTextureByName(const std::string Name)
{
	std::vector<FETexture*> Result;

	auto TextureIterator = Textures.begin();
	while (TextureIterator != Textures.end())
	{
		if (TextureIterator->second->GetName() == Name)
		{
			Result.push_back(TextureIterator->second);
		}

		TextureIterator++;
	}

	return Result;
}

void FEResourceManager::DeleteFETexture(const FETexture* Texture)
{
	if (Texture == NoTexture)
	{
		LOG.Add("Attempt to delete noTexture texture in function FEResourceManager::deleteFETexture.", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	// looking if this texture is used in some materials
	// to-do: should be done through list of pointers to materials that uses this texture.
	auto MaterialIterator = Materials.begin();
	while (MaterialIterator != Materials.end())
	{
		for (size_t i = 0; i < MaterialIterator->second->Textures.size(); i++)
		{
			if (MaterialIterator->second->Textures[i] == Texture)
			{
				MaterialIterator->second->Textures[i] = NoTexture;
				break;
			}
		}

		MaterialIterator++;
	}

	// After we make sure that texture is no more referenced by any material, we can delete TextureIterator.
	Textures.erase(Texture->GetObjectID());

	delete Texture;
}

void FEResourceManager::DeleteFEMesh(const FEMesh* Mesh)
{
	// looking if this mesh is used in some gameModels
	// to-do: should be done through list of pointers to gameModels that uses this mesh.
	auto GameModelIterator = GameModels.begin();
	while (GameModelIterator != GameModels.end())
	{
		if (GameModelIterator->second->Mesh == Mesh)
		{
			GameModelIterator->second->Mesh = GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);
		}

		GameModelIterator++;
	}

	Meshes.erase(Mesh->GetObjectID());
	delete Mesh;
}

std::vector<std::string> FEResourceManager::GetGameModelList()
{
	FE_MAP_TO_STR_VECTOR(GameModels)
}

std::vector<std::string> FEResourceManager::GetStandardGameModelList()
{
	FE_MAP_TO_STR_VECTOR(StandardGameModels)
}

FEGameModel* FEResourceManager::GetGameModel(const std::string ID)
{
	if (GameModels.find(ID) == GameModels.end())
	{
		if (StandardGameModels.find(ID) != StandardGameModels.end())
		{
			return StandardGameModels[ID];
		}

		return nullptr;
	}

	return GameModels[ID];
}

std::vector<FEGameModel*> FEResourceManager::GetGameModelByName(const std::string Name)
{
	std::vector<FEGameModel*> Result;

	auto it = GameModels.begin();
	while (it != GameModels.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	it = StandardGameModels.begin();
	while (it != StandardGameModels.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	return Result;
}

FEGameModel* FEResourceManager::CreateGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedGameModel";

	if (Mesh == nullptr)
		Mesh = GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);

	if (Material == nullptr)
		Material = GetMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

	FEGameModel* NewGameModel = new FEGameModel(Mesh, Material, Name);
	if (!ForceObjectID.empty())
	{
		GameModels[ForceObjectID] = NewGameModel;
		GameModels[ForceObjectID]->SetID(ForceObjectID);
	}
	else
	{
		GameModels[NewGameModel->ID] = NewGameModel;
	}
	
	GameModels[NewGameModel->ID]->SetName(Name);
	return GameModels[NewGameModel->ID];
}

void FEResourceManager::DeleteGameModel(const FEGameModel* GameModel)
{
	GameModels.erase(GameModel->GetObjectID());
	delete GameModel;
}

bool FEResourceManager::MakeGameModelStandard(FEGameModel* GameModel)
{
	if (GameModel == nullptr)
	{
		LOG.Add("gameModel is nullptr in function FEResourceManager::makeGameModelStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardGameModels.find(GameModel->GetObjectID()) == StandardGameModels.end())
	{
		if (GameModels.find(GameModel->GetObjectID()) != GameModels.end())
			GameModels.erase(GameModel->GetObjectID());
		StandardGameModels[GameModel->GetObjectID()] = GameModel;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::CreateShader(std::string ShaderName, const char* VertexText, const char* FragmentText,
										  const char* TessControlText, const char* TessEvalText,
										  const char* GeometryText, const char* ComputeText, const std::string ForceObjectID)
{
	if (ShaderName.empty())
		ShaderName = "unnamedShader";

	// Shader with compute stage cannot contain any other stage.
	if (ComputeText != nullptr && (VertexText != nullptr || FragmentText != nullptr ||
								   TessControlText != nullptr || TessEvalText != nullptr ||
								   GeometryText != nullptr))
	{
		return nullptr;
	}

	FEShader* NewShader = new FEShader(ShaderName, VertexText, FragmentText, TessControlText, TessEvalText, GeometryText, ComputeText);
	if (!ForceObjectID.empty())
		NewShader->SetID(ForceObjectID);
	Shaders[NewShader->GetObjectID()] = NewShader;
	return NewShader;
}

bool FEResourceManager::MakeShaderStandard(FEShader* Shader)
{
	if (Shader == nullptr)
	{
		LOG.Add("shader is nullptr in function FEResourceManager::makeShaderStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardShaders.find(Shader->GetObjectID()) == StandardShaders.end())
	{
		if (Shaders.find(Shader->GetObjectID()) != Shaders.end())
			Shaders.erase(Shader->GetObjectID());
		StandardShaders[Shader->GetObjectID()] = Shader;

		return true;
	}

	return false;
}

FEShader* FEResourceManager::GetShader(const std::string ShaderID)
{
	if (Shaders.find(ShaderID) == Shaders.end())
	{
		if (StandardShaders.find(ShaderID) != StandardShaders.end())
		{
			return StandardShaders[ShaderID];
		}

		return nullptr;
	}
	else
	{
		return Shaders[ShaderID];
	}
}

std::vector<FEShader*> FEResourceManager::GetShaderByName(const std::string Name)
{
	std::vector<FEShader*> Result;

	auto it = Shaders.begin();
	while (it != Shaders.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	it = StandardShaders.begin();
	while (it != StandardShaders.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	return Result;
}

std::vector<std::string> FEResourceManager::GetShadersList()
{
	FE_MAP_TO_STR_VECTOR(Shaders)
}

std::vector<std::string> FEResourceManager::GetStandardShadersList()
{
	FE_MAP_TO_STR_VECTOR(StandardShaders)
}

void FEResourceManager::DeleteShader(const FEShader* Shader)
{
	if (Shader == nullptr)
		return;

	auto it = Materials.begin();
	while (it != Materials.end())
	{
		if (it->second->Shader->GetNameHash() == Shader->GetNameHash())
			it->second->Shader = GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
		
		it++;
	}

	it = StandardMaterials.begin();
	while (it != StandardMaterials.end())
	{
		if (it->second->Shader->GetNameHash() == Shader->GetNameHash())
			it->second->Shader = GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);

		it++;
	}

	Shaders.erase(Shader->GetObjectID());
	StandardShaders.erase(Shader->GetObjectID());
	delete Shader;
}

bool FEResourceManager::ReplaceShader(const std::string OldShaderID, FEShader* NewShader)
{
	const FEShader* ShaderToReplace = GetShader(OldShaderID);
	if (ShaderToReplace == nullptr)
		return false;

	if (NewShader->GetName().empty())
		return false;

	if (Shaders.find(OldShaderID) != Shaders.end())
	{
		//*(shaders[oldShaderID]) = *newShader;
		//newShader->setID(oldShaderID);
		Shaders[OldShaderID]->ReCompile(NewShader->GetName(), NewShader->GetVertexShaderText(), NewShader->GetFragmentShaderText(),
										NewShader->GetTessControlShaderText(), NewShader->GetTessEvalShaderText(),
										NewShader->GetGeometryShaderText(), NewShader->GetComputeShaderText());
	}
	else if (StandardShaders.find(OldShaderID) != StandardShaders.end())
	{
		//*(standardShaders[oldShaderID]) = *newShader;
		//newShader->setID(oldShaderID);

		StandardShaders[OldShaderID]->ReCompile(NewShader->GetName(), NewShader->GetVertexShaderText(), NewShader->GetFragmentShaderText(),
												NewShader->GetTessControlShaderText(), NewShader->GetTessEvalShaderText(),
												NewShader->GetGeometryShaderText(), NewShader->GetComputeShaderText());
	}

	return true;
}

FETexture* FEResourceManager::CreateBlankHightMapTexture(int Width, int Height, std::string Name)
{
	if (Name.empty())
		Name = "UnnamedHeightMap";

	FETexture* NewTexture = CreateTexture(Name);
	NewTexture->Width = Width;
	NewTexture->Height = Height;
	NewTexture->InternalFormat = GL_R16;
	NewTexture->MagFilter = FE_LINEAR;
	NewTexture->FileName = "NULL";

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));

	unsigned short* RawPixels = new unsigned short[NewTexture->Width * NewTexture->Height];
	for (size_t i = 0; i < static_cast<size_t>(NewTexture->Width * NewTexture->Height); i++)
	{
		RawPixels[i] = static_cast<unsigned short>(0xffff * 0.5);
	}

	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RED, GL_UNSIGNED_SHORT, (unsigned char*)RawPixels);
	delete[] RawPixels;

	return NewTexture;
}

std::string FEResourceManager::LoadGLSL(const char* FileName)
{
	std::string ShaderData;
	std::ifstream File(FileName);

	if (File.is_open())
	{
		std::string line;
		while (getline(File, line))
		{
			ShaderData += line;
			ShaderData += "\n";
		}
		File.close();
	}
	else
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadGLSL.", "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	return ShaderData;
}

FETexture* FEResourceManager::CreateTexture(const GLint InternalFormat, const GLenum Format, const int Width, const int Height, const bool bUnManaged, std::string Name)
{
	if (Name.empty())
		Name = "unnamedTexture";

	FETexture* NewTexture = new FETexture(InternalFormat, Format, Width, Height, Name);
	if (!bUnManaged)
		Textures[NewTexture->GetObjectID()] = NewTexture;

	return NewTexture;
}

FEFramebuffer* FEResourceManager::CreateFramebuffer(const int Attachments, const int Width, const int Height, const bool bHDR)
{
	FEFramebuffer* NewFramebuffer = new FEFramebuffer();
	NewFramebuffer->ColorAttachments.resize(MaxColorAttachments);
	for (size_t i = 0; i < static_cast<size_t>(MaxColorAttachments); i++)
	{
		NewFramebuffer->ColorAttachments[i] = nullptr;
	}
	
	NewFramebuffer->Width = Width;
	NewFramebuffer->Height = Height;

	FE_GL_ERROR(glGenFramebuffers(1, &NewFramebuffer->FBO));
	NewFramebuffer->Bind();

	if (Attachments & FE_COLOR_ATTACHMENT)
	{
		bHDR ? NewFramebuffer->SetColorAttachment(CreateTexture(GL_RGBA16F, GL_RGBA, Width, Height)) : NewFramebuffer->SetColorAttachment(new FETexture(Width, Height, FreeObjectName(FE_TEXTURE)));
	}

	if (Attachments & FE_DEPTH_ATTACHMENT)
	{
		NewFramebuffer->SetDepthAttachment(CreateTexture(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, Width, Height));

		// if only DEPTH_ATTACHMENT
		if (!(Attachments & FE_COLOR_ATTACHMENT))
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		NewFramebuffer->GetDepthAttachment()->Bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		NewFramebuffer->GetDepthAttachment()->UnBind();
	}

	if (Attachments & FE_STENCIL_ATTACHMENT)
	{
		//to-do: make TextureIterator correct
		NewFramebuffer->SetStencilAttachment(new FETexture(Width, Height, FreeObjectName(FE_TEXTURE)));
	}

	NewFramebuffer->UnBind();
	return NewFramebuffer;
}

FEPostProcess* FEResourceManager::CreatePostProcess(const int ScreenWidth, const int ScreenHeight, const std::string Name)
{
	FEPostProcess* NewPostProcess = new FEPostProcess(Name);

	NewPostProcess->ScreenWidth = ScreenWidth;
	NewPostProcess->ScreenHeight = ScreenHeight;
	// Currently postProcess is not using intermediateFramebuffer colorAttachment directly.
	NewPostProcess->IntermediateFramebuffer = CreateFramebuffer(0, ScreenWidth, ScreenHeight);

	return NewPostProcess;
}

std::string FEResourceManager::FreeObjectName(const FE_OBJECT_TYPE ObjectType)
{
	std::string Result = "NULL";
	switch (ObjectType)
	{
		case FocalEngine::FE_NULL:
		{
			return Result;
		}
		case FocalEngine::FE_SHADER:
		{
			const size_t NextID = Shaders.size() > StandardShaders.size() ? Shaders.size() : StandardShaders.size();
			size_t Index = 0;
			Result = "shader_" + std::to_string(NextID + Index);
			while (Shaders.find(Result) != Shaders.end() || StandardShaders.find(Result) != StandardShaders.end())
			{
				Index++;
				Result = "shader_" + std::to_string(NextID + Index);
			}

			return Result;
		}
		case FocalEngine::FE_TEXTURE:
		{
			const size_t NextID = Textures.size() > StandardTextures.size() ? Textures.size() : StandardTextures.size();
			size_t index = 0;
			Result = "texture_" + std::to_string(NextID + index);
			while (Textures.find(Result) != Textures.end() || StandardTextures.find(Result) != StandardTextures.end())
			{
				index++;
				Result = "texture_" + std::to_string(NextID + index);
			}
			
			return Result;
		}
		case FocalEngine::FE_MESH:
		{
			const size_t NextID = Meshes.size() > StandardMeshes.size() ? Meshes.size() : StandardMeshes.size();
			size_t index = 0;
			Result = "mesh_" + std::to_string(NextID + index);
			while (Meshes.find(Result) != Meshes.end() || StandardMeshes.find(Result) != StandardMeshes.end())
			{
				index++;
				Result = "mesh_" + std::to_string(NextID + index);
			}

			return Result;
		}
		case FocalEngine::FE_MATERIAL:
		{
			const size_t NextID = Materials.size() > StandardMaterials.size() ? Materials.size() : StandardMaterials.size();
			size_t index = 0;
			Result = "material_" + std::to_string(NextID + index);
			while (Materials.find(Result) != Materials.end() || StandardMaterials.find(Result) != StandardMaterials.end())
			{
				index++;
				Result = "material_" + std::to_string(NextID + index);
			}

			return Result;
		}
		case FocalEngine::FE_GAMEMODEL:
		{
			const size_t NextID = GameModels.size() > StandardGameModels.size() ? GameModels.size() : StandardGameModels.size();
			size_t index = 0;
			Result = "gameModel_" + std::to_string(NextID + index);
			while (GameModels.find(Result) != GameModels.end() || StandardGameModels.find(Result) != StandardGameModels.end())
			{
				index++;
				Result = "gameModel_" + std::to_string(NextID + index);
			}

			return Result;
		}
		case FocalEngine::FE_ENTITY:
		{
			return Result;
		}
		case FocalEngine::FE_TERRAIN_DEPRECATED:
		{
			return Result;
		}
		case FocalEngine::FE_ENTITY_INSTANCED_DEPRECATED:
		{
			return Result;
		}
		default:
		{
			return Result;
		}	
	}
}

FETexture* FEResourceManager::CreateSameFormatTexture(FETexture* ReferenceTexture, const int DifferentW, const int DifferentH, const bool bUnManaged, const std::string Name)
{
	if (ReferenceTexture == nullptr)
	{
		LOG.Add("FEResourceManager::CreateSameFormatTexture called with nullptr pointer as ReferenceTexture", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return nullptr;
	}

	if (DifferentW == 0 && DifferentH == 0)
		return CreateTexture(ReferenceTexture->InternalFormat, ReferenceTexture->Format, ReferenceTexture->Width, ReferenceTexture->Height, bUnManaged, Name);
	
	if (DifferentW != 0 && DifferentH == 0)
		return CreateTexture(ReferenceTexture->InternalFormat, ReferenceTexture->Format, DifferentW, ReferenceTexture->Height, bUnManaged, Name);
	
	if (DifferentW == 0 && DifferentH != 0)
		return CreateTexture(ReferenceTexture->InternalFormat, ReferenceTexture->Format, ReferenceTexture->Width, DifferentH, bUnManaged, Name);
	
	return CreateTexture(ReferenceTexture->InternalFormat, ReferenceTexture->Format, DifferentW, DifferentH, bUnManaged, Name);
}

// TO-DO: Ensure all formats are supported.
FETexture* FEResourceManager::CreateCopyOfTexture(FETexture* ReferenceTexture, bool bUnManaged, std::string Name)
{
	FETexture* Result = nullptr;
	if (ReferenceTexture == nullptr)
	{
		LOG.Add("FEResourceManager::CreateCopyOfTexture called with nullptr pointer as ReferenceTexture", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}

	unsigned char* ReferenceRawData = ReferenceTexture->GetRawData();
	Result = CreateSameFormatTexture(ReferenceTexture, 0, 0, bUnManaged, Name);

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, Result->TextureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, Result->InternalFormat, Result->Width, Result->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, ReferenceRawData);

	return Result;
}

void FEResourceManager::AddTextureToManaged(FETexture* Texture)
{
	if (Texture == nullptr)
	{
		LOG.Add("FEResourceManager::AddTextureToManaged called with nullptr pointer as texture", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Textures.find(Texture->GetObjectID()) != Textures.end())
	{
		LOG.Add("FEResourceManager::AddTextureToManaged called with already managed texture", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	Textures[Texture->GetObjectID()] = Texture;
}

void FEResourceManager::ReSaveStandardMeshes()
{
	const std::vector<std::string> StandardMeshes = GetStandardMeshList();
	for (size_t i = 0; i < StandardMeshes.size(); i++)
	{
		FEMesh* CurrentMesh = GetMesh(StandardMeshes[i]);
		SaveFEMesh(CurrentMesh, (ResourcesFolder + CurrentMesh->GetObjectID() + std::string(".model")).c_str());
	}
}

void FEResourceManager::ReSaveStandardTextures()
{
	auto it = StandardTextures.begin();
	while (it != StandardTextures.end())
	{
		SaveFETexture(it->second, (ResourcesFolder + it->second->GetObjectID() + std::string(".texture")).c_str());
		it++;
	}
}

void FEResourceManager::DeleteMaterial(const FEMaterial* Material)
{
	// looking if this mesh is used in some gameModels
	// to-do: should be done through list of pointers to gameModels that uses this mesh.
	auto GameModelIterator = GameModels.begin();
	while (GameModelIterator != GameModels.end())
	{
		if (GameModelIterator->second->Material == Material)
			GameModelIterator->second->Material = GetMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

		GameModelIterator++;
	}

	Materials.erase(Material->GetObjectID());
	delete Material;
}

std::string FEResourceManager::GetDefaultResourcesFolder()
{
	return ResourcesFolder;
}

std::vector<FETexture*> FEResourceManager::ChannelsToFETextures(FETexture* SourceTexture)
{
	std::vector<FETexture*> Result;

	size_t TextureDataLenght = 0;
	const unsigned char* pixels = SourceTexture->GetRawData(&TextureDataLenght);

	unsigned char* RedChannel = new unsigned char[static_cast<size_t>(TextureDataLenght / 4.0f)];
	int index = 0;
	for (size_t i = 0; i < TextureDataLenght; i += 4)
	{
		RedChannel[index++] = pixels[i];
	}

	unsigned char* GreenChannel = new unsigned char[static_cast<size_t>(TextureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 1; i < TextureDataLenght; i += 4)
	{
		GreenChannel[index++] = pixels[i];
	}

	unsigned char* BlueChannel = new unsigned char[static_cast<size_t>(TextureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 2; i < TextureDataLenght; i += 4)
	{
		BlueChannel[index++] = pixels[i];
	}

	unsigned char* AlphaChannel = new unsigned char[static_cast<size_t>(TextureDataLenght / 4.0f)];
	index = 0;
	for (size_t i = 3; i < TextureDataLenght; i += 4)
	{
		AlphaChannel[index++] = pixels[i];
	}

	Result.push_back(RawDataToFETexture(RedChannel, SourceTexture->GetWidth(), SourceTexture->GetHeight(), GL_RED, GL_RED));
	Result.back()->SetName(SourceTexture->GetName() + "_R");

	Result.push_back(RawDataToFETexture(GreenChannel, SourceTexture->GetWidth(), SourceTexture->GetHeight(), GL_RED, GL_RED));
	Result.back()->SetName(SourceTexture->GetName() + "_G");

	Result.push_back(RawDataToFETexture(BlueChannel, SourceTexture->GetWidth(), SourceTexture->GetHeight(), GL_RED, GL_RED));
	Result.back()->SetName(SourceTexture->GetName() + "_B");

	Result.push_back(RawDataToFETexture(AlphaChannel, SourceTexture->GetWidth(), SourceTexture->GetHeight(), GL_RED, GL_RED));
	Result.back()->SetName(SourceTexture->GetName() + "_A");

	delete[] pixels;
	delete[] RedChannel;
	delete[] GreenChannel;
	delete[] BlueChannel;
	delete[] AlphaChannel;

	return Result;
}

bool FEResourceManager::ExportFETextureToPNG(FETexture* TextureToExport, const char* FileName)
{
	if (TextureToExport == nullptr)
	{
		LOG.Add("FEResourceManager::ExportFETextureToPNG with nullptr TextureToExport", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	if (TextureToExport->InternalFormat != GL_RGBA &&
		TextureToExport->InternalFormat != GL_RED &&
		TextureToExport->InternalFormat != GL_R16 &&
		TextureToExport->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		TextureToExport->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT &&
		TextureToExport->InternalFormat != GL_RGBA16F)
	{
		LOG.Add("FEResourceManager::ExportFETextureToPNG InternalFormat of TextureToExport is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	std::vector<unsigned char> RawData;

	if (TextureToExport->InternalFormat == GL_RGBA16F)
	{
		auto HalfFloatToFloat = [](unsigned short HalfFloat) -> float {
			int Sign = (HalfFloat >> 15) & 0x00000001;
			int Exponent = (HalfFloat >> 10) & 0x0000001F;
			int Mantissa = HalfFloat & 0x000003FF;

			Exponent = Exponent + (127 - 15);
			int FloatValue = (Sign << 31) | (Exponent << 23) | (Mantissa << 13);

			return *reinterpret_cast<float*>(&FloatValue);
		};

		const unsigned char* TextureData = TextureToExport->GetRawData();
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 4);

		size_t RawDataIndex = 0;
		for (size_t i = 0; i < RawData.size() * sizeof(unsigned short); i+=2)
		{
			// Combine two bytes into one 16-bit half float.
			unsigned short Half = (TextureData[i + 1] << 8) | TextureData[i];
			float Value = HalfFloatToFloat(Half);

			// Clamp and scale the floating-point value to a byte.
			unsigned char ByteValue = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, Value)) * 255.0f);
			RawData[RawDataIndex++] = ByteValue;
		}

		// Flip image vertically
		const size_t RowBytes = TextureToExport->GetWidth() * 4;
		unsigned char* RowBuffer = new unsigned char[RowBytes];

		for (size_t y = 0; y < TextureToExport->GetHeight() / 2; y++)
		{
			// Copy the top row to a buffer
			std::memcpy(RowBuffer, RawData.data() + y * RowBytes, RowBytes);

			// Copy the bottom row to the top
			std::memcpy(RawData.data() + y * RowBytes, RawData.data() + (TextureToExport->GetHeight() - 1 - y) * RowBytes, RowBytes);

			// Copy the buffer contents (original top row) to the bottom
			std::memcpy(RawData.data() + (TextureToExport->GetHeight() - 1 - y) * RowBytes, RowBuffer, RowBytes);
		}

		delete[] RowBuffer;
	}
	else if (TextureToExport->InternalFormat == GL_RED)
	{
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 4);
		const unsigned char* TextreData = TextureToExport->GetRawData();

		for (size_t i = 0; i < RawData.size(); i += 4)
		{
			RawData[i] = TextreData[i / 4];
			RawData[i + 1] = TextreData[i / 4];
			RawData[i + 2] = TextreData[i / 4];
			RawData[i + 3] = 255;
		}
	}
	else if (TextureToExport->InternalFormat == GL_R16)
	{
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 2);
		const unsigned char* TextreData = TextureToExport->GetRawData();

		for (size_t i = 0; i < RawData.size(); i++)
		{
			RawData[i] = TextreData[i];
		}

		for (size_t i = 0; i < RawData.size(); i+=2)
		{
			std::swap(RawData[i], RawData[i + 1]);
		}
	}
	else
	{
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 4);
		const unsigned char* TextreData = TextureToExport->GetRawData();

		for (size_t i = 0; i < RawData.size(); i++)
		{
			RawData[i] = TextreData[i];
		}
	}

	const std::string FilePath = FileName;
	int Error = 0;
	if (TextureToExport->InternalFormat == GL_R16)
	{
		Error = lodepng::encode(FilePath, RawData, TextureToExport->GetWidth(), TextureToExport->GetHeight(), LCT_GREY, 16);
	}
	else
	{
		Error = lodepng::encode(FilePath, RawData, TextureToExport->GetWidth(), TextureToExport->GetHeight());
	}

	return Error == 0;
}

bool FEResourceManager::ExportRawDataToPNG(const char* FileName, const unsigned char* TextureData, const int Width, const int Height, const GLint Internalformat)
{
	if (Internalformat != GL_RGBA &&
		Internalformat != GL_RED &&
		Internalformat != GL_R16 &&
		Internalformat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		Internalformat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FEResourceManager::exportRawDataToPNG internalFormat is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	const std::string FilePath = FileName;
	int Error = 0;
	if (Internalformat == GL_R16)
	{
		Error = lodepng::encode(FilePath, TextureData, Width, Height, LCT_GREY, 16);
	}
	else
	{
		Error = lodepng::encode(FilePath, TextureData, Width, Height);
	}

	return Error == 0;
}

unsigned char* FEResourceManager::ResizeTextureRawData(FETexture* SourceTexture, const size_t TargetWidth, const size_t TargetHeight, int FiltrationLevel)
{
	if (SourceTexture == nullptr)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData with nullptr sourceTexture", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (TargetWidth <= 0 || TargetHeight <= 0 || TargetWidth > 8192 || TargetHeight > 8192)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData unsupported target resolution", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (TargetWidth == SourceTexture->GetWidth() && TargetHeight == SourceTexture->GetHeight())
	{
		LOG.Add("FEResourceManager::resizeTextureRawData no operation needed", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (FiltrationLevel < 0)
		FiltrationLevel = 0;

	if (FiltrationLevel > 16)
		FiltrationLevel = 16;

	if (SourceTexture->InternalFormat != GL_RGBA &&
		SourceTexture->InternalFormat != GL_RED &&
		SourceTexture->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		SourceTexture->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData internalFormat of sourceTexture is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return nullptr;
	}

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, SourceTexture->GetTextureID()));

	unsigned char* CurrentData = SourceTexture->GetRawData();
	unsigned char* Result = ResizeTextureRawData(CurrentData, SourceTexture->GetWidth(), SourceTexture->GetHeight(), TargetWidth, TargetHeight, SourceTexture->InternalFormat, FiltrationLevel);
	delete[] CurrentData;

	return Result;
}

unsigned char* FEResourceManager::ResizeTextureRawData(const unsigned char* TextureData, const size_t Width, const size_t Height, const size_t TargetWidth, const size_t TargetHeight, const GLint InternalFormat, int FiltrationLevel)
{
	if (TextureData == nullptr)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData with nullptr textureData", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (Width <= 0 || Height <= 0 || Width > 8192 || Height > 8192)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData unsupported current resolution", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (TargetWidth == Width && TargetHeight == Height)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData no operation needed", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (TargetWidth <= 0 || TargetHeight <= 0 || TargetWidth > 8192 || TargetHeight > 8192)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData unsupported target resolution", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (InternalFormat != GL_RGBA &&
		InternalFormat != GL_RED &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FEResourceManager::resizeTextureRawData internalFormat of textureData is not supported", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (FiltrationLevel < 0)
		FiltrationLevel = 0;

	if (FiltrationLevel > 16)
		FiltrationLevel = 16;

	size_t SourceByteCount = 4;
	if (InternalFormat == GL_RED)
		SourceByteCount = 1;

	const float ResizeFactorX = static_cast<float>(Width) / static_cast<float>(TargetWidth);
	const float ResizeFactorY = static_cast<float>(Height) / static_cast<float>(TargetHeight);

	//int newPixel[4];
	unsigned char* Result = new unsigned char[TargetWidth * TargetHeight * 4];
	for (size_t i = 0; i < TargetHeight; i++)
	{
		for (size_t j = 0; j < TargetWidth; j++)
		{
			const size_t TargetIndex = (i * TargetWidth + j) * 4;

			if (TargetIndex + 3 >= TargetWidth * TargetHeight * 4)
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

			Result[targetIndex] = newPixel[0];
			Result[targetIndex + 1] = newPixel[1];
			Result[targetIndex + 2] = newPixel[2];
			Result[targetIndex + 3] = newPixel[3];*/

			const size_t ScaledI = static_cast<size_t>(i * ResizeFactorY);
			const size_t ScaledJ = static_cast<size_t>(j * ResizeFactorX);
			const size_t SourceIndex = (ScaledI * Width + ScaledJ) * SourceByteCount;

			if (SourceByteCount == 4)
			{
				Result[TargetIndex] = TextureData[SourceIndex];
				Result[TargetIndex + 1] = TextureData[SourceIndex + 1];
				Result[TargetIndex + 2] = TextureData[SourceIndex + 2];
				Result[TargetIndex + 3] = TextureData[SourceIndex + 3];
			}
			else
			{
				Result[TargetIndex] = TextureData[SourceIndex];
				Result[TargetIndex + 1] = TextureData[SourceIndex];
				Result[TargetIndex + 2] = TextureData[SourceIndex];
				Result[TargetIndex + 3] = 255;
			}
		}
	}

	return Result;
}

void FEResourceManager::ResizeTexture(FETexture* SourceTexture, const int TargetWidth, const int TargetHeight, int FiltrationLevel)
{
	if (SourceTexture == nullptr)
	{
		LOG.Add("FEResourceManager::resizeTexture with nullptr sourceTexture", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (TargetWidth <= 0 || TargetHeight <= 0 || TargetWidth > 8192 || TargetHeight > 8192)
	{
		LOG.Add("FEResourceManager::resizeTexture unsupported target resolution", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (TargetWidth == SourceTexture->GetWidth() && TargetHeight == SourceTexture->GetHeight())
	{
		LOG.Add("FEResourceManager::resizeTexture no operation needed", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (SourceTexture->InternalFormat != GL_RGBA &&
		SourceTexture->InternalFormat != GL_RED &&
		SourceTexture->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		SourceTexture->InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FEResourceManager::resizeTexture internalFormat of sourceTexture is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	if (FiltrationLevel < 0)
		FiltrationLevel = 0;

	if (FiltrationLevel > 16)
		FiltrationLevel = 16;

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, SourceTexture->GetTextureID()));

	const unsigned char* CurrentData = SourceTexture->GetRawData();
	unsigned char* Result = ResizeTextureRawData(CurrentData, SourceTexture->GetWidth(), SourceTexture->GetHeight(), TargetWidth, TargetHeight, SourceTexture->InternalFormat, FiltrationLevel);

	SourceTexture->Width = TargetWidth;
	SourceTexture->Height = TargetHeight;
	const int MaxDimention = std::max(SourceTexture->Width, SourceTexture->Height);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);

	if (SourceTexture->InternalFormat == GL_RGBA)
	{
		SourceTexture->UpdateRawData(Result, MipCount);
	}
	else if (SourceTexture->InternalFormat == GL_RED)
	{
		// Function resizeTextureRawData will output RGBA data, we need to take only R channel.
		std::vector<unsigned char> RedChannel;
		RedChannel.resize(SourceTexture->GetWidth() * SourceTexture->GetHeight());
		for (size_t i = 0; i < RedChannel.size() * 4; i+=4)
		{
			RedChannel[i / 4] = Result[i];
		}

		SourceTexture->UpdateRawData(RedChannel.data(), MipCount);
	}
	else
	{
		SourceTexture->UpdateRawData(Result, MipCount);
	}

	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	delete[] CurrentData;
	delete[] Result;
}

FETexture* FEResourceManager::LoadJPGTexture(const char* FileName, const std::string Name)
{
	int UWidth, UHeight, Channels;
	const unsigned char* RawData = stbi_load(FileName, &UWidth, &UHeight, &Channels, 0);

	if (RawData == nullptr)
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadJPGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		if (!StandardTextures.empty())
		{
			return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
		}
		else
		{
			return nullptr;
		}
	}

	FETexture* NewTexture = CreateTexture(Name);
	NewTexture->Width = UWidth;
	NewTexture->Height = UHeight;

	const int InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, RawData);
	delete RawData;
	NewTexture->InternalFormat = InternalFormat;

	if (NewTexture->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (NewTexture->MagFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	NewTexture->FileName = FileName;

	if (Name.empty())
	{
		const std::string FilePath = NewTexture->FileName;
		std::size_t Index = FilePath.find_last_of("/\\");
		const std::string NewFileName = FilePath.substr(Index + 1);
		Index = NewFileName.find_last_of(".");
		const std::string FileNameWithOutExtention = NewFileName.substr(0, Index);
		NewTexture->SetName(FileNameWithOutExtention);
	}

	return NewTexture;
}

FETexture* FEResourceManager::LoadBMPTexture(const char* FileName, const std::string Name)
{
	return LoadJPGTexture(FileName, Name);
}

void FEResourceManager::CreateMaterialsFromOBJData(std::vector<FEObject*>& ResultArray)
{
	std::unordered_map<std::string, bool> LoadedTextures;
	const FEObjLoader& OBJLoader = FEObjLoader::getInstance();

	for (size_t i = 0; i < OBJLoader.LoadedObjects.size(); i++)
	{
		if (OBJLoader.LoadedObjects[i]->MaterialRecords.empty())
			continue;

		FEMaterial* Material = nullptr;
		std::string Name;
		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
				Material->SetAlbedoMap(LoadedTexture);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlbedoMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
				Material->SetNormalMap(LoadedTexture);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].NormalMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].SpecularHighlightMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].AlphaMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].DisplacementMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (LoadedTextures.find(OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile) == LoadedTextures.end() &&
			!OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile.empty())
		{
			FETexture* LoadedTexture = ImportTexture(OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile.c_str());
			if (LoadedTexture != nullptr)
			{
				LoadedTextures[OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile] = true;
				Name = FILE_SYSTEM.GetFileName(OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile.c_str());

				ResultArray.push_back(LoadedTexture);
				ResultArray.back()->SetName(Name);

				if (Material == nullptr)
					Material = CreateMaterial(OBJLoader.LoadedObjects[i]->MaterialRecords[0].Name);
			}
			else
			{
				LOG.Add(std::string("can't load texture: ") + OBJLoader.LoadedObjects[i]->MaterialRecords[0].StencilDecalMapFile + " in function FEResourceManager::CreateMaterialsFromOBJData.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
		}

		if (Material != nullptr)
		{
			Material->Shader = GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
			ResultArray.push_back(Material);

			FEGameModel* GameModel = CreateGameModel(reinterpret_cast<FEMesh*>(ResultArray[i]), Material);
			ResultArray.push_back(GameModel);
		}
	}
}

FETexture* FEResourceManager::ImportTexture(const char* FileName)
{
	FETexture* Result = nullptr;

	if (FileName == nullptr)
	{
		LOG.Add("call of FEResourceManager::ImportTexture with nullptr FileName", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}
	
	if (!FILE_SYSTEM.CheckFile(FileName))
	{
		LOG.Add("Can't locate file: " + std::string(FileName) + " in FEResourceManager::ImportTexture", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	std::string FileExtention = FILE_SYSTEM.GetFileExtension(FileName);
	std::transform(FileExtention.begin(), FileExtention.end(), FileExtention.begin(), [](const unsigned char C) { return std::tolower(C); });
	
	if (FileExtention == ".png")
	{
		Result = LoadPNGTexture(FileName);
	}
	else if (FileExtention == ".jpg")
	{
		Result = LoadJPGTexture(FileName);

	}
	else if (FileExtention == ".bmp")
	{
		Result = LoadBMPTexture(FileName);
	}
	
	return Result;
}

FETexture* FEResourceManager::CreateTextureWithTransparency(FETexture* OriginalTexture, FETexture* MaskTexture)
{
	if (OriginalTexture == nullptr || MaskTexture == nullptr)
	{
		LOG.Add("call of FEResourceManager::CreateTextureWithTransparency with nullptr argument(s)", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (OriginalTexture->GetWidth() != MaskTexture->GetWidth() || OriginalTexture->GetHeight() != MaskTexture->GetHeight())
	{
		LOG.Add("OriginalTexture and MaskTexture dimensions mismatch in FEResourceManager::CreateTextureWithTransparency", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	FETexture* Result = CreateTexture();
	unsigned char* RawData = OriginalTexture->GetRawData();
	Result->Width = OriginalTexture->GetWidth();
	Result->Height = OriginalTexture->GetHeight();

	const unsigned char* MaskRawData = MaskTexture->GetRawData();
	for (size_t i = 4; i < static_cast<size_t>(Result->Width * Result->Height * 4); i += 4)
	{
		RawData[-1 + i] = MaskRawData[-4 + i];
	}

	const int InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, Result->TextureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, InternalFormat, Result->Width, Result->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawData);
	Result->InternalFormat = InternalFormat;

	if (Result->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (Result->MagFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	Result->FileName = OriginalTexture->FileName;

	const std::string FilePath = Result->FileName;
	std::size_t index = FilePath.find_last_of("/\\");
	const std::string NewFileName = FilePath.substr(index + 1);
	index = NewFileName.find_last_of(".");
	const std::string FileNameWithOutExtention = NewFileName.substr(0, index);
	Result->SetName(FileNameWithOutExtention);

	delete[] RawData;
	delete[] MaskRawData;

	return Result;
}

std::vector<std::string> FEResourceManager::GetPrefabList()
{
	FE_MAP_TO_STR_VECTOR(Prefabs)
}

std::vector<std::string> FEResourceManager::GetStandardPrefabList()
{
	FE_MAP_TO_STR_VECTOR(StandardPrefabs)
}

FEPrefab* FEResourceManager::GetPrefab(const std::string ID)
{
	if (Prefabs.find(ID) == Prefabs.end())
	{
		if (StandardPrefabs.find(ID) != StandardPrefabs.end())
		{
			return StandardPrefabs[ID];
		}

		return nullptr;
	}

	return Prefabs[ID];
}

std::vector<FEPrefab*> FEResourceManager::GetPrefabByName(const std::string Name)
{
	std::vector<FEPrefab*> Result;

	auto it = Prefabs.begin();
	while (it != Prefabs.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	it = StandardPrefabs.begin();
	while (it != StandardPrefabs.end())
	{
		if (it->second->GetName() == Name)
		{
			Result.push_back(it->second);
		}

		it++;
	}

	return Result;
}

FEPrefab* FEResourceManager::CreatePrefab(FEGameModel* GameModel, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedPrefab";

	FEPrefab* NewPrefab = new FEPrefab();
	if (!ForceObjectID.empty())
	{
		Prefabs[ForceObjectID] = NewPrefab;
		Prefabs[ForceObjectID]->SetID(ForceObjectID);
	}
	else
	{
		Prefabs[NewPrefab->ID] = NewPrefab;
	}

	Prefabs[NewPrefab->ID]->SetName(Name);
	if (GameModel != nullptr)
	{
		Prefabs[NewPrefab->ID]->Components.push_back(new FEPrefabComponent());
		Prefabs[NewPrefab->ID]->Components.back()->GameModel = GameModel;
	}
	
	return Prefabs[NewPrefab->ID];
}

bool FEResourceManager::MakePrefabStandard(FEPrefab* Prefab)
{
	if (Prefab == nullptr)
	{
		LOG.Add("prefab is nullptr in function FEResourceManager::makePrefabStandard.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (StandardPrefabs.find(Prefab->GetObjectID()) == StandardPrefabs.end())
	{
		if (Prefabs.find(Prefab->GetObjectID()) != Prefabs.end())
			Prefabs.erase(Prefab->GetObjectID());
		StandardPrefabs[Prefab->GetObjectID()] = Prefab;

		return true;
	}

	return false;
}

void FEResourceManager::DeletePrefab(const FEPrefab* Prefab)
{
	Prefabs.erase(Prefab->GetObjectID());
	delete Prefab;
}

void FEResourceManager::LoadStandardPrefabs()
{
	FEPrefab* NewPrefab = new FEPrefab(GetGameModel("67251E393508013ZV579315F"/*"standardGameModel"*/), "standardPrefab");
	NewPrefab->SetID("4575527C773848040760656F");
	MakePrefabStandard(NewPrefab);
}

void FEResourceManager::AddColorToFEMeshVertices(FEMesh* Mesh, float* Colors, int ColorSize)
{
	if (Mesh == nullptr)
		return;

	if (Colors == nullptr || ColorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(Mesh->VaoID));

	Mesh->ColorCount = ColorSize;
	Mesh->ColorBufferID = 0;
	Mesh->VertexAttributes |= FE_COLOR;
	FE_GL_ERROR(glGenBuffers(1, &Mesh->ColorBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Mesh->ColorBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Mesh->ColorCount, Colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}