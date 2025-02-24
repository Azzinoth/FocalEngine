#define STB_IMAGE_IMPLEMENTATION
#include "FEResourceManager.h"
#include "../SubSystems/Scene/Components/NativeScriptSystem/FENativeScriptProject.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetResourceManager()
{
	return FEResourceManager::GetInstancePointer();
}
#endif

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

bool FEResourceManager::SetTag(FEObject* Object, std::string NewTag)
{
	if (Object == nullptr)
	{
		LOG.Add("Object is nullptr in function FEResourceManager::SetTag.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (NewTag == ENGINE_RESOURCE_TAG)
	{
		LOG.Add("Can't set tag to ENGINE_RESOURCE_TAG in function FEResourceManager::SetTag.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	SetTagIternal(Object, NewTag);
	return true;
}

void FEResourceManager::SetTagIternal(FEObject* Object, std::string NewTag)
{
	if (Object == nullptr)
	{
		LOG.Add("Object is nullptr in function FEResourceManager::SetTagIternal.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	Object->SetTag(NewTag);
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

FETexture* FEResourceManager::LoadPNGTexture(const char* FileName, const std::string Name)
{
	std::vector<unsigned char> RawFileData;
	std::ifstream File(FileName, std::ios::binary);
	if (!File)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function FEResourceManager::LoadPNGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
	}

	File.unsetf(std::ios::skipws);
	RawFileData.insert(RawFileData.begin(), std::istream_iterator<unsigned char>(File), std::istream_iterator<unsigned char>());

	std::vector<unsigned char> RawExtractedData;
	unsigned int Width, Height;
	lodepng::State State;

	unsigned int Error = lodepng::decode(RawExtractedData, Width, Height, State, RawFileData);
	if (Error != 0)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function FEResourceManager::LoadPNGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
	}

	bool bUsingAlpha = false;
	for (size_t i = 3; i < RawExtractedData.size(); i += 4)
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

	OBJECT_MANAGER.SaveFEObjectPart(File, Texture);

	File.write((char*)&Texture->Width, sizeof(int));
	File.write((char*)&Texture->Height, sizeof(int));
	File.write((char*)&Texture->InternalFormat, sizeof(int));

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
	char** PixelData = new char* [MipCount];

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
		for (size_t j = ImgSize * 2 - 1; j > 0; j--)
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
		LOG.Add("FEResourceManager::LoadTextureFileAsyncCallBack texture with ID: " + NotLoadedTexture->GetObjectID() + " was not loaded!", "FE_LOG_LOADING", FE_LOG_ERROR);
		// And delete entry for that texture in a general list of textures.
		// That will prevent TextureIterator from saving in a scene File.
		RESOURCE_MANAGER.DeleteFETexture(NotLoadedTexture);
	}
	else
	{
		const FETexture* NewlyCreatedTexture = RESOURCE_MANAGER.LoadFETexture(Input->FileData, "", Input->NewTexture);

		// If any material uses this texture, set the dirty flag.
		// Game model will be updated as a consequence.
		const std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();

		for (size_t i = 0; i < MaterialList.size(); i++)
		{
			FEMaterial* CurrentMaterial = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
			if (CurrentMaterial->GetTag() == ENGINE_RESOURCE_TAG)
				continue;

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

	int Width = 0;
	int Height = 0;
	int InternalFormat = 0;
	char* TextureName = nullptr;
	std::string NameFromFile;

	char* ObjectID = nullptr;
	std::string ID;

	if (Version != FE_TEXTURE_VERSION)
	{
		LOG.Add(std::string("can't load fileData: in function FEResourceManager::LoadFETexture. FileData was created in different version of engine!"), "FE_LOG_LOADING", FE_LOG_ERROR);
		return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
	}

	FEObjectLoadedData ObjectData = OBJECT_MANAGER.LoadFEObjectPart(FileData, CurrentShift);
	ID = ObjectData.ID;
	NameFromFile = ObjectData.Name;

	Width = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	Height = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	InternalFormat = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;

	FETexture* NewTexture = nullptr;
	if (ExistingTexture != nullptr)
	{
		NewTexture = ExistingTexture;
		NewTexture->SetName(NameFromFile);
		FE_GL_ERROR(glGenTextures(1, &NewTexture->TextureID));
	}
	else
	{
		NewTexture = CreateTexture(NameFromFile);
	}

	NewTexture->Width = Width;
	NewTexture->Height = Height;
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
	if (!ID.empty())
	{
		const std::string OldID = NewTexture->GetObjectID();
		NewTexture->SetID(ID);

		if (Textures.find(OldID) != Textures.end())
		{
			Textures.erase(OldID);
			Textures[NewTexture->GetObjectID()] = NewTexture;
		}
	}

	NewTexture->Tag = ObjectData.Tag;
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

	FEMesh* NewMesh = RawDataToMesh(CubePositions, CubeNormals, CubeTangents, CubeUV, CubeIndices, "cube");
	Meshes.erase(NewMesh->GetObjectID());
	NewMesh->SetID("84251E6E0D0801363579317R"/*"cube"*/);
	NewMesh->SetName("FECube");
	NewMesh->SetTag(ENGINE_RESOURCE_TAG);
	Meshes[NewMesh->GetObjectID()] = NewMesh;

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

	NewMesh = RawDataToMesh(PlanePositions, PlaneNormals, PlaneTangents, PlaneUV, PlaneIndices, "plane");
	Meshes.erase(NewMesh->GetObjectID());
	NewMesh->SetID("1Y251E6E6T78013635793156"/*"plane"*/);
	NewMesh->SetName("FEPlane");
	NewMesh->SetTag(ENGINE_RESOURCE_TAG);
	Meshes[NewMesh->GetObjectID()] = NewMesh;

	NewMesh = LoadFEMesh((ResourcesFolder + "7F251E3E0D08013E3579315F.model").c_str(), "sphere");
	Meshes.erase(NewMesh->GetObjectID());
	NewMesh->SetID("7F251E3E0D08013E3579315F"/*"sphere"*/);
	NewMesh->SetName("FESphere");
	NewMesh->SetTag(ENGINE_RESOURCE_TAG);
	Meshes[NewMesh->GetObjectID()] = NewMesh;
}

FEResourceManager::FEResourceManager()
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);

	// FIX ME! It is temporary solution.
	// We need to understand should we use files or .fepackage
	if (FILE_SYSTEM.DoesFileExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/EngineResources.fepackage"))
	{
		PrivateEngineAssetPackage = new FEAssetPackage();
		if (!PrivateEngineAssetPackage->LoadFromFile((FILE_SYSTEM.GetCurrentWorkingPath() + "/EngineResources.fepackage").c_str()))
		{
			LOG.Add("FEResourceManager::FEResourceManager: Can't load EngineResources.fepackage file!", "FE_LOG_LOADING", FE_LOG_ERROR);
			delete PrivateEngineAssetPackage;
			PrivateEngineAssetPackage = nullptr;
		}

		UnPackPrivateEngineAssetPackage(PrivateEngineAssetPackage, FILE_SYSTEM.GetCurrentWorkingPath());
	}

	NoTexture = LoadFETexture((ResourcesFolder + "48271F005A73241F5D7E7134.texture").c_str(), "noTexture");
	NoTexture->SetTag(ENGINE_RESOURCE_TAG);
	FETexture::AddToNoDeletingList(NoTexture->GetTextureID());

	FEShader* NewShader = CreateShader("FECombineFrameBuffers", LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_CombineFrameBuffers_FS.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"5C267A01466A545E7D1A2E66");
	NewShader->SetTag(ENGINE_RESOURCE_TAG);

	LoadStandardMaterial();
	LoadStandardMeshes();
	LoadStandardGameModels();

	// FIX ME! Temporary code.
	/*FENativeScriptModule* NewNativeScriptModule = CreateNativeScriptModule("D:/Script__09_10_2024/OnlyCamera/UserScriptTest.dll",
																		   "D:/Script__09_10_2024/OnlyCamera/UserScriptTest.pdb",
																		   "D:/Script__09_10_2024/OnlyCamera/Release/UserScriptTest.dll",
																		   {}, "Camera scripts", "2B7956623302254F620A675F");
	NewNativeScriptModule->SetTag(ENGINE_RESOURCE_TAG);
	SaveFENativeScriptModule(NewNativeScriptModule, "CameraScripts.fescriptmodule");*/

	// Load all standard script modules.
	std::vector<std::string> PotentialScriptModuleFiles = FILE_SYSTEM.GetFileNamesInDirectory(ResourcesFolder);
	for (size_t i = 0; i < PotentialScriptModuleFiles.size(); i++)
	{
		if (PotentialScriptModuleFiles[i].substr(PotentialScriptModuleFiles[i].size() - 19, 19) == ".nativescriptmodule")
		{
			LoadFENativeScriptModule((ResourcesFolder + PotentialScriptModuleFiles[i]).c_str());
		}
	}
}

FEResourceManager::~FEResourceManager()
{
	Clear();
}

std::vector<FEObject*> FEResourceManager::ImportOBJ(const char* FileName, const bool bForceOneMesh)
{
	FEObjLoader& OBJLoader = FEObjLoader::GetInstance();
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
		return GetMesh("84251E6E0D0801363579317R"/*"cube"*/);
	}

	char* Buffer = new char[4];

	// Version of FEMesh File type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;

	std::string LoadedObjectID;
	std::string LoadedName;
	if (Version != FE_MESH_VERSION)
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadFEMesh. File was created in different version of engine!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return GetMesh("84251E6E0D0801363579317R"/*"cube"*/);
	}

	FEObjectLoadedData ObjectData = OBJECT_MANAGER.LoadFEObjectPart(File);
	LoadedObjectID = ObjectData.ID;
	LoadedName = ObjectData.Name;

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
		Name.empty() ? LoadedName : Name);

	const std::string OldID = NewMesh->ID;
	// Overwrite ID with Loaded ID.
	if (!LoadedObjectID.empty())
	{
		NewMesh->SetID(LoadedObjectID);
		Meshes.erase(OldID);
		Meshes[NewMesh->GetObjectID()] = NewMesh;
	}

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	NewMesh->AABB = MeshAABB;
	NewMesh->SetName(Name);
	NewMesh->Tag = ObjectData.Tag;

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

Json::Value FEResourceManager::SaveMaterialToJSON(FEMaterial* Material)
{
	Json::Value Root;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Material->Textures[i] != nullptr)
			Root["Textures"][std::to_string(i).c_str()] = Material->Textures[i]->GetObjectID();

		if (Material->TextureBindings[i] != -1)
			Root["Texture bindings"][std::to_string(i).c_str()] = Material->TextureBindings[i];

		if (Material->TextureChannels[i] != -1)
			Root["Texture channels"][std::to_string(i).c_str()] = Material->TextureChannels[i];
	}

	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Material);
	Root["Metalness"] = Material->GetMetalness();
	Root["Roughness"] = Material->GetRoughness();
	Root["NormalMap intensity"] = Material->GetNormalMapIntensity();
	Root["AmbientOcclusion intensity"] = Material->GetAmbientOcclusionIntensity();
	Root["AmbientOcclusionMap intensity"] = Material->GetAmbientOcclusionMapIntensity();
	Root["RoughnessMap intensity"] = Material->GetRoughnessMapIntensity();
	Root["MetalnessMap intensity"] = Material->GetMetalnessMapIntensity();
	Root["Tiling"] = Material->GetTiling();
	Root["Compack packing"] = Material->IsCompackPacking();

	return Root;
}

FEMaterial* FEResourceManager::LoadMaterialFromJSON(Json::Value& Root)
{
	FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["FEObjectData"]);

	FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial(LoadedObjectData.Name, LoadedObjectData.ID);
	RESOURCE_MANAGER.SetTag(NewMaterial, LoadedObjectData.Tag);
	NewMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

	std::vector<Json::String> MembersList = Root.getMemberNames();
	for (size_t i = 0; i < MembersList.size(); i++)
	{
		if (MembersList[i] == "Textures")
		{
			for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
			{
				if (Root["Textures"].isMember(std::to_string(j).c_str()))
				{
					std::string TextureID = Root["Textures"][std::to_string(j).c_str()].asCString();
					NewMaterial->Textures[j] = RESOURCE_MANAGER.GetTexture(TextureID);
					if (NewMaterial->Textures[j] == nullptr)
						NewMaterial->Textures[j] = RESOURCE_MANAGER.NoTexture;
				}
			}
		}

		if (MembersList[i] == "Texture bindings")
		{
			for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
			{
				if (Root["Texture bindings"].isMember(std::to_string(j).c_str()))
				{
					int Binding = Root["Texture bindings"][std::to_string(j).c_str()].asInt();
					NewMaterial->TextureBindings[j] = Binding;
				}
			}
		}

		if (MembersList[i] == "Texture channels")
		{
			for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
			{
				if (Root["Texture channels"].isMember(std::to_string(j).c_str()))
				{
					int binding = Root["Texture channels"][std::to_string(j).c_str()].asInt();
					NewMaterial->TextureChannels[j] = binding;
				}
			}
		}
	}

	NewMaterial->SetMetalness(Root["Metalness"].asFloat());
	NewMaterial->SetRoughness(Root["Roughness"].asFloat());
	NewMaterial->SetNormalMapIntensity(Root["NormalMap intensity"].asFloat());
	NewMaterial->SetAmbientOcclusionIntensity(Root["AmbientOcclusion intensity"].asFloat());
	NewMaterial->SetAmbientOcclusionMapIntensity(Root["AmbientOcclusionMap intensity"].asFloat());
	NewMaterial->SetRoughnessMapIntensity(Root["RoughnessMap intensity"].asFloat());
	NewMaterial->SetMetalnessMapIntensity(Root["MetalnessMap intensity"].asFloat());

	if (Root.isMember("Tiling"))
		NewMaterial->SetTiling(Root["Tiling"].asFloat());
	NewMaterial->SetCompackPacking(Root["Compack packing"].asBool());

	return NewMaterial;
}

std::vector<std::string> FEResourceManager::GetMaterialIDList()
{
	FE_MAP_TO_STR_VECTOR(Materials)
}

std::vector<std::string> FEResourceManager::GetEnginePrivateMaterialIDList()
{
	return GetResourceIDListByTag(Materials, ENGINE_RESOURCE_TAG);
}

FEMaterial* FEResourceManager::GetMaterial(const std::string ID)
{
	if (Materials.find(ID) == Materials.end())
		return nullptr;

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

std::vector<std::string> FEResourceManager::GetMeshIDList()
{
	FE_MAP_TO_STR_VECTOR(Meshes)
}

std::vector<std::string> FEResourceManager::GetEnginePrivateMeshIDList()
{
	return GetResourceIDListByTag(Meshes, ENGINE_RESOURCE_TAG);
}

FEMesh* FEResourceManager::GetMesh(const std::string ID)
{
	if (Meshes.find(ID) == Meshes.end())
		return nullptr;

	return Meshes[ID];
}

std::vector<FEMesh*> FEResourceManager::GetMeshByName(const std::string Name)
{
	std::vector<FEMesh*> Result;
	auto MeshIterator = Meshes.begin();
	while (MeshIterator != Meshes.end())
	{
		if (MeshIterator->second->GetName() == Name)
			Result.push_back(MeshIterator->second);

		MeshIterator++;
	}

	return Result;
}

void FEResourceManager::LoadStandardMaterial()
{
	FEMaterial* NewMaterial = CreateMaterial("SolidColorMaterial", "18251A5E0F08013Z3939317U");
	NewMaterial->SetTag(ENGINE_RESOURCE_TAG);
	NewMaterial->Shader = CreateShader("FESolidColorShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//SolidColorMaterial//FE_SolidColor_FS.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"6917497A5E0C05454876186F");
	NewMaterial->Shader->SetTag(ENGINE_RESOURCE_TAG);

	FEShaderUniformValue Color("baseColor", glm::vec3(1.0f, 0.4f, 0.6f));
	NewMaterial->SetUniformVariation(Color);
	NewMaterial->SetTag(ENGINE_RESOURCE_TAG);

	FEShader* FEPhongShader = CreateShader("FEPhongShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PhongMaterial//FE_Phong_FS.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"4C41665B5E125C2A07456E44");
	FEPhongShader->SetTag(ENGINE_RESOURCE_TAG);

	// ****************************** PBR SHADER ******************************
	FEShader* PBRShader = CreateShader("FEPBRShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS_GBUFFER.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"0800253C242B05321A332D09");

	PBRShader->SetTag(ENGINE_RESOURCE_TAG);

	FEShader* PBRShaderForward = CreateShader("FEPBRShaderForward", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"5E45017E664A62273E191500");

	PBRShaderForward->SetTag(ENGINE_RESOURCE_TAG);

	FEShader* PBRGBufferShader = CreateShader("FEPBRGBufferShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"670B01496E202658377A4576");

	PBRGBufferShader->SetTag(ENGINE_RESOURCE_TAG);


	FEShader* PBRInstancedShader = CreateShader("FEPBRInstancedShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_DEFERRED.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"7C80085C184442155D0F3C7B");

	PBRInstancedShader->SetTag(ENGINE_RESOURCE_TAG);

	FEShader* PBRInstancedGBufferShader = CreateShader("FEPBRInstancedGBufferShader", LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_INSTANCED_VS.glsl").c_str()).c_str(),
		LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//PBRMaterial//FE_PBR_FS_GBUFFER.glsl").c_str()).c_str(),
		nullptr, nullptr, nullptr, nullptr,
		"613830232E12602D6A1D2C17");

	PBRInstancedGBufferShader->SetTag(ENGINE_RESOURCE_TAG);

	NewMaterial = CreateMaterial("FEPBRBaseMaterial", "61649B9E0F08013Q3939316C" /*"FEPBRBaseMaterial"*/);
	NewMaterial->SetTag(ENGINE_RESOURCE_TAG);
	NewMaterial->Shader = GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	NewMaterial->SetAlbedoMap(NoTexture);
	NewMaterial->SetTag(ENGINE_RESOURCE_TAG);
	// ****************************** PBR SHADER END ******************************

	// same as FERenderer::updateFogInShaders()
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogDensity", 0.007f);
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogGradient", 2.5f);
	GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("shadowBlurFactor", 1.0f);

	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogDensity", 0.007f);
	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogGradient", 2.5f);
	GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("shadowBlurFactor", 1.0f);
}

void FEResourceManager::LoadStandardGameModels()
{
	FEGameModel* NewGameModel = new FEGameModel(GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/), GetMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/), "standardGameModel");
	GameModels.erase(NewGameModel->GetObjectID());
	NewGameModel->SetID("67251E393508013ZV579315F");
	NewGameModel->SetTag(ENGINE_RESOURCE_TAG);
	GameModels[NewGameModel->GetObjectID()] = NewGameModel;
}

void FEResourceManager::Clear()
{
	ClearResource(Materials);
	ClearResource(Meshes);
	ClearResource(Textures);
	ClearResource(GameModels);
	ClearResource(Prefabs);
}

void FEResourceManager::SaveFEMesh(FEMesh* Mesh, const char* FileName)
{
	std::fstream File;
	File.open(FileName, std::ios::out | std::ios::binary);

	// Version of FEMesh File type.
	float Version = FE_MESH_VERSION;
	File.write((char*)&Version, sizeof(float));

	OBJECT_MANAGER.SaveFEObjectPart(File, Mesh);

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

std::vector<std::string> FEResourceManager::GetTextureIDList()
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

std::vector<std::string> FEResourceManager::GetGameModelIDList()
{
	FE_MAP_TO_STR_VECTOR(GameModels)
}

std::vector<std::string> FEResourceManager::GetEnginePrivateGameModelIDList()
{
	return GetResourceIDListByTag(GameModels, ENGINE_RESOURCE_TAG);
}

FEGameModel* FEResourceManager::GetGameModel(const std::string ID)
{
	if (GameModels.find(ID) == GameModels.end())
		return nullptr;

	return GameModels[ID];
}

std::vector<FEGameModel*> FEResourceManager::GetGameModelByName(const std::string Name)
{
	std::vector<FEGameModel*> Result;

	auto GameModelIterator = GameModels.begin();
	while (GameModelIterator != GameModels.end())
	{
		if (GameModelIterator->second->GetName() == Name)
		{
			Result.push_back(GameModelIterator->second);
		}

		GameModelIterator++;
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

Json::Value FEResourceManager::SaveGameModelToJSON(FEGameModel* GameModel)
{
	Json::Value Root;

	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(GameModel);
	Root["Mesh"] = GameModel->Mesh->GetObjectID();
	Root["Material"] = GameModel->Material->GetObjectID();
	Root["ScaleFactor"] = GameModel->GetScaleFactor();

	Root["LODs"]["HaveLODlevels"] = GameModel->IsUsingLOD();
	if (GameModel->IsUsingLOD())
	{
		Root["LODs"]["CullDistance"] = GameModel->GetCullDistance();
		Root["LODs"]["Billboard zero rotaion"] = GameModel->GetBillboardZeroRotaion();
		Root["LODs"]["LODCount"] = GameModel->GetLODCount();
		for (size_t i = 0; i < GameModel->GetLODCount(); i++)
		{
			Root["LODs"][std::to_string(i)]["Mesh"] = GameModel->GetLODMesh(i)->GetObjectID();
			Root["LODs"][std::to_string(i)]["Max draw distance"] = GameModel->GetLODMaxDrawDistance(i);
			Root["LODs"][std::to_string(i)]["IsBillboard"] = GameModel->IsLODBillboard(i);
			if (GameModel->IsLODBillboard(i))
				Root["LODs"][std::to_string(i)]["Billboard material"] = GameModel->GetBillboardMaterial()->GetObjectID();
		}
	}

	return Root;
}

FEGameModel* FEResourceManager::LoadGameModelFromJSON(Json::Value& Root)
{
	FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["FEObjectData"]);

	FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh(Root["Mesh"].asCString()),
		RESOURCE_MANAGER.GetMaterial(Root["Material"].asCString()),
		LoadedObjectData.Name, LoadedObjectData.ID);

	if (NewGameModel == nullptr)
	{
		LOG.Add("Can't load game model from JSON in function FEResourceManager::LoadGameModelFromJSON.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	RESOURCE_MANAGER.SetTag(NewGameModel, LoadedObjectData.Tag);

	NewGameModel->SetScaleFactor(Root["ScaleFactor"].asFloat());

	bool bHaveLODLevels = Root["LODs"]["HaveLODlevels"].asBool();
	NewGameModel->SetUsingLOD(bHaveLODLevels);
	if (bHaveLODLevels)
	{
		NewGameModel->SetCullDistance(Root["LODs"]["CullDistance"].asFloat());
		NewGameModel->SetBillboardZeroRotaion(Root["LODs"]["Billboard zero rotaion"].asFloat());

		size_t LODCount = Root["LODs"]["LODCount"].asInt();
		for (size_t i = 0; i < LODCount; i++)
		{
			NewGameModel->SetLODMesh(i, RESOURCE_MANAGER.GetMesh(Root["LODs"][std::to_string(i)]["Mesh"].asString()));
			NewGameModel->SetLODMaxDrawDistance(i, Root["LODs"][std::to_string(i)]["Max draw distance"].asFloat());

			bool bLODBillboard = Root["LODs"][std::to_string(i)]["IsBillboard"].asBool();
			NewGameModel->SetIsLODBillboard(i, bLODBillboard);
			if (bLODBillboard)
				NewGameModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Root["LODs"][std::to_string(i)]["Billboard material"].asString()));
		}
	}

	return NewGameModel;
}

void FEResourceManager::DeleteGameModel(const FEGameModel* GameModel)
{
	GameModels.erase(GameModel->GetObjectID());
	delete GameModel;
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

FEShader* FEResourceManager::GetShader(const std::string ShaderID)
{
	if (Shaders.find(ShaderID) == Shaders.end())
		return nullptr;

	return Shaders[ShaderID];
}

std::vector<FEShader*> FEResourceManager::GetShaderByName(const std::string Name)
{
	std::vector<FEShader*> Result;

	auto ShaderIterator = Shaders.begin();
	while (ShaderIterator != Shaders.end())
	{
		if (ShaderIterator->second->GetName() == Name)
		{
			Result.push_back(ShaderIterator->second);
		}

		ShaderIterator++;
	}

	return Result;
}

std::vector<std::string> FEResourceManager::GetShaderIDList()
{
	FE_MAP_TO_STR_VECTOR(Shaders)
}

std::vector<std::string> FEResourceManager::GetEnginePrivateShaderIDList()
{
	return GetResourceIDListByTag(Shaders, ENGINE_RESOURCE_TAG);
}

void FEResourceManager::DeleteShader(const FEShader* Shader)
{
	if (Shader == nullptr)
		return;

	if (Shader->GetTag() == ENGINE_RESOURCE_TAG)
	{
		LOG.Add("Attempt to delete engine private shader in function FEResourceManager::DeleteShader.", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	auto MaterialsIterator = Materials.begin();
	while (MaterialsIterator != Materials.end())
	{
		if (MaterialsIterator->second->Shader->GetNameHash() == Shader->GetNameHash())
			MaterialsIterator->second->Shader = GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);

		MaterialsIterator++;
	}

	Shaders.erase(Shader->GetObjectID());
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
		Shaders[OldShaderID]->ReCompile(NewShader->GetName(), NewShader->GetVertexShaderText(), NewShader->GetFragmentShaderText(),
			NewShader->GetTessControlShaderText(), NewShader->GetTessEvalShaderText(),
			NewShader->GetGeometryShaderText(), NewShader->GetComputeShaderText());
	}

	return true;
}

// TO-DO: That function should be in TERRAIN_SYSTEM and FEResourceManager should just exepct general settings to create texture.
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

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

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
		const size_t NextID = Shaders.size();
		size_t Index = 0;
		Result = "Shader_" + std::to_string(NextID + Index);
		while (Shaders.find(Result) != Shaders.end())
		{
			Index++;
			Result = "Shader_" + std::to_string(NextID + Index);
		}

		return Result;
	}
	case FocalEngine::FE_TEXTURE:
	{
		const size_t NextID = Textures.size();
		size_t Index = 0;
		Result = "Texture_" + std::to_string(NextID + Index);
		while (Textures.find(Result) != Textures.end())
		{
			Index++;
			Result = "Texture_" + std::to_string(NextID + Index);
		}

		return Result;
	}
	case FocalEngine::FE_MESH:
	{
		const size_t NextID = Meshes.size();
		size_t Index = 0;
		Result = "Mesh_" + std::to_string(NextID + Index);
		while (Meshes.find(Result) != Meshes.end())
		{
			Index++;
			Result = "Mesh_" + std::to_string(NextID + Index);
		}

		return Result;
	}
	case FocalEngine::FE_MATERIAL:
	{
		const size_t NextID = Materials.size();
		size_t Index = 0;
		Result = "Material_" + std::to_string(NextID + Index);
		while (Materials.find(Result) != Materials.end())
		{
			Index++;
			Result = "Material_" + std::to_string(NextID + Index);
		}

		return Result;
	}
	case FocalEngine::FE_GAMEMODEL:
	{
		const size_t NextID = GameModels.size();
		size_t Index = 0;
		Result = "GameModel_" + std::to_string(NextID + Index);
		while (GameModels.find(Result) != GameModels.end())
		{
			Index++;
			Result = "GameModel_" + std::to_string(NextID + Index);
		}

		return Result;
	}
	case FocalEngine::FE_ENTITY:
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
	auto MeshIterator = Meshes.begin();
	while (MeshIterator != Meshes.end())
	{
		if (MeshIterator->second->GetTag() == ENGINE_RESOURCE_TAG)
			SaveFEMesh(MeshIterator->second, (ResourcesFolder + MeshIterator->second->GetObjectID() + std::string(".model")).c_str());
		MeshIterator++;
	}
}

void FEResourceManager::ReSaveEnginePrivateTextures()
{
	auto TextureIterator = Textures.begin();
	while (TextureIterator != Textures.end())
	{
		if (TextureIterator->second->GetTag() == ENGINE_RESOURCE_TAG)
			SaveFETexture(TextureIterator->second, (ResourcesFolder + TextureIterator->second->GetObjectID() + std::string(".texture")).c_str());
		TextureIterator++;
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
		TextureToExport->InternalFormat != GL_RGBA16F &&
		TextureToExport->InternalFormat != GL_RG16F)
	{
		LOG.Add("FEResourceManager::ExportFETextureToPNG InternalFormat of TextureToExport is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	auto HalfFloatToFloat = [](unsigned short HalfFloat) -> float {
		int Sign = (HalfFloat >> 15) & 0x00000001;
		int Exponent = (HalfFloat >> 10) & 0x0000001F;
		int Mantissa = HalfFloat & 0x000003FF;

		Exponent = Exponent + (127 - 15);
		int FloatValue = (Sign << 31) | (Exponent << 23) | (Mantissa << 13);

		return *reinterpret_cast<float*>(&FloatValue);
	};

	std::vector<unsigned char> RawData;
	if (TextureToExport->InternalFormat == GL_RGBA16F)
	{
		const unsigned char* TextureData = TextureToExport->GetRawData();
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 4);

		size_t RawDataIndex = 0;
		for (size_t i = 0; i < RawData.size() * sizeof(unsigned short); i += 2)
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
	else if (TextureToExport->InternalFormat == GL_RG16F)
	{
		const unsigned char* TextureData = TextureToExport->GetRawData();
		// Two channels per pixel (R and G) – output 8 bits per channel.
		RawData.resize(TextureToExport->GetWidth() * TextureToExport->GetHeight() * 2);

		size_t RawDataIndex = 0;
		// Total half-floats = width * height * 2; each half-float is 2 bytes.
		for (size_t i = 0; i < RawData.size() * sizeof(unsigned short); i += 2)
		{
			unsigned short Half = (TextureData[i + 1] << 8) | TextureData[i];
			float Value = HalfFloatToFloat(Half);
			unsigned char ByteValue = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, Value)) * 255.0f);
			RawData[RawDataIndex++] = ByteValue;
		}

		// Flip image vertically.
		const size_t RowBytes = TextureToExport->GetWidth() * 2;  // 2 bytes per pixel row
		unsigned char* RowBuffer = new unsigned char[RowBytes];
		for (size_t y = 0; y < TextureToExport->GetHeight() / 2; y++)
		{
			std::memcpy(RowBuffer, RawData.data() + y * RowBytes, RowBytes);
			std::memcpy(RawData.data() + y * RowBytes, RawData.data() + (TextureToExport->GetHeight() - 1 - y) * RowBytes, RowBytes);
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

		for (size_t i = 0; i < RawData.size(); i += 2)
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
	if (TextureToExport->InternalFormat == GL_RG16F)
	{
		Error = lodepng::encode(FilePath, RawData, TextureToExport->GetWidth(), TextureToExport->GetHeight(), LCT_GREY_ALPHA);
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
		for (size_t i = 0; i < RedChannel.size() * 4; i += 4)
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
		return GetTexture("48271F005A73241F5D7E7134"); // "noTexture"
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
	const FEObjLoader& OBJLoader = FEObjLoader::GetInstance();

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

	if (!FILE_SYSTEM.DoesFileExist(FileName))
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

std::vector<std::string> FEResourceManager::GetPrefabIDList()
{
	FE_MAP_TO_STR_VECTOR(Prefabs)
}

std::vector<std::string> FEResourceManager::GetEnginePrivatePrefabIDList()
{
	return GetResourceIDListByTag(Prefabs, ENGINE_RESOURCE_TAG);
}

FEPrefab* FEResourceManager::GetPrefab(const std::string ID)
{
	if (Prefabs.find(ID) == Prefabs.end())
		return nullptr;

	return Prefabs[ID];
}

std::vector<FEPrefab*> FEResourceManager::GetPrefabByName(const std::string Name)
{
	std::vector<FEPrefab*> Result;

	auto PrefabsIterator = Prefabs.begin();
	while (PrefabsIterator != Prefabs.end())
	{
		if (PrefabsIterator->second->GetName() == Name)
		{
			Result.push_back(PrefabsIterator->second);
		}

		PrefabsIterator++;
	}

	return Result;
}

FEPrefab* FEResourceManager::CreatePrefab(std::string Name, const std::string ForceObjectID, FEScene* SceneDescription)
{
	if (Name.empty())
		Name = "Unnamed prefab";

	FEPrefab* NewPrefab = new FEPrefab(Name, SceneDescription == nullptr);
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
	if (SceneDescription != nullptr)
	{
		SceneDescription->SetFlag(FESceneFlag::PrefabDescription, true);
		Prefabs[NewPrefab->ID]->Scene = SceneDescription;
	}

	return Prefabs[NewPrefab->ID];
}

Json::Value FEResourceManager::SavePrefabToJSON(FEPrefab* Prefab)
{
	Json::Value Root;

	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Prefab);
	if (Prefab->GetScene() == nullptr)
	{
		LOG.Add("FEResourceManager::SavePrefabToJSON: Prefab scene is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		Root["SceneID"] = "";
		return Root;
	}

	Root["SceneID"] = Prefab->GetScene()->GetObjectID();
	return Root;
}

FEPrefab* FEResourceManager::LoadPrefabFromJSON(Json::Value& Root)
{
	FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["FEObjectData"]);

	std::string SceneID;
	if (Root.isMember("Scene"))
	{
		SceneID = Root["Scene"]["ID"].asCString();
	}
	else
	{
		SceneID = Root["SceneID"].asCString();
	}

	FEScene* Scene = SCENE_MANAGER.GetScene(SceneID);
	if (Scene == nullptr)
	{
		LOG.Add("FEResourceManager::LoadPrefabFromJSON: Prefab scene is missing!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(LoadedObjectData.Name, LoadedObjectData.ID, Scene);
	RESOURCE_MANAGER.SetTag(NewPrefab, LoadedObjectData.Tag);

	return NewPrefab;
}

void FEResourceManager::DeletePrefab(const FEPrefab* Prefab)
{
	Prefabs.erase(Prefab->GetObjectID());
	delete Prefab;
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

Json::Value FEResourceManager::SaveFEObjectPart(FEObject* Object)
{
	Json::Value Root;
	Root["ID"] = Object->GetObjectID();
	Root["Tag"] = Object->GetTag();
	Root["Name"] = Object->GetName();
	Root["Type"] = Object->GetType();

	return Root;
}

FEObjectLoadedData FEResourceManager::LoadFEObjectPart(Json::Value Root)
{
	FEObjectLoadedData Result;

	if (Root.isMember("ID") && Root["ID"].isString())
		Result.ID = Root["ID"].asString();

	if (Root.isMember("Tag") && Root["Tag"].isString())
		Result.Tag = Root["Tag"].asString();

	if (Root.isMember("Name") && Root["Name"].isString())
		Result.Name = Root["Name"].asString();

	if (Root.isMember("Type") && Root["Type"].isInt())
		Result.Type = FE_OBJECT_TYPE(Root["Type"].asInt());

	return Result;
}

std::vector<std::string> FEResourceManager::GetTagsThatWillPreventDeletion()
{
	std::vector<std::string> Result;
	for (size_t i = 0; i < TagsThatWillPreventDeletion.size(); i++)
		Result.push_back(TagsThatWillPreventDeletion[i]);

	return Result;
}

void FEResourceManager::AddTagThatWillPreventDeletion(std::string Tag)
{
	if (std::find(TagsThatWillPreventDeletion.begin(), TagsThatWillPreventDeletion.end(), Tag) == TagsThatWillPreventDeletion.end())
		TagsThatWillPreventDeletion.push_back(Tag);
}

void FEResourceManager::RemoveTagThatWillPreventDeletion(std::string Tag)
{
	if (Tag == ENGINE_RESOURCE_TAG)
		return;

	for (size_t i = 0; i < TagsThatWillPreventDeletion.size(); i++)
	{
		if (TagsThatWillPreventDeletion[i] == Tag)
		{
			TagsThatWillPreventDeletion.erase(TagsThatWillPreventDeletion.begin() + i);
			return;
		}
	}
}

std::vector<std::string> FEResourceManager::GetNativeScriptModuleIDList()
{
	FE_MAP_TO_STR_VECTOR(NativeScriptModules);
}

std::vector<std::string> FEResourceManager::GetEnginePrivateNativeScriptModuleIDList()
{
	return GetResourceIDListByTag(NativeScriptModules, ENGINE_RESOURCE_TAG);
}

FENativeScriptModule* FEResourceManager::GetNativeScriptModule(std::string ID)
{
	if (NativeScriptModules.find(ID) == NativeScriptModules.end())
		return nullptr;

	return NativeScriptModules[ID];
}

std::vector<FENativeScriptModule*> FEResourceManager::GetNativeScriptModuleByName(std::string Name)
{
	std::vector<FENativeScriptModule*> Result;

	auto NativeScriptModulesIterator = NativeScriptModules.begin();
	while (NativeScriptModulesIterator != NativeScriptModules.end())
	{
		if (NativeScriptModulesIterator->second->GetName() == Name)
		{
			Result.push_back(NativeScriptModulesIterator->second);
		}

		NativeScriptModulesIterator++;
	}

	return Result;
}

std::string FEResourceManager::ReadDLLModuleID(std::string DLLFilePath)
{
	if (DLLFilePath.empty())
	{
		LOG.Add("call of FEResourceManager::ReadDLLModuleID with empty DLLFilePath", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return "";
	}

	if (!FILE_SYSTEM.DoesFileExist(DLLFilePath))
	{
		LOG.Add("can't locate file: " + DLLFilePath + " in FEResourceManager::ReadDLLModuleID", "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	HMODULE DLLHandle = LoadLibraryA(DLLFilePath.c_str());
	if (!DLLHandle)
	{
		LOG.Add("FEResourceManager::ReadDLLModuleID failed to load DLL: " + DLLFilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	typedef char* (*Get_ModuleID_Function)(void);
	Get_ModuleID_Function GetModuleID = (Get_ModuleID_Function)GetProcAddress(DLLHandle, "GetModuleID");
	if (!GetModuleID)
	{
		LOG.Add("FEResourceManager::ReadDLLModuleID failed to get GetModuleID function from DLL: " + DLLFilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	std::string DLLModuleID = GetModuleID();
	if (DLLModuleID.empty() || DLLModuleID.size() != 24)
	{
		LOG.Add("FEResourceManager::ReadDLLModuleID failed to get proper DLLModuleID from DLL: " + DLLFilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	FreeLibrary(DLLHandle);
	return DLLModuleID;
}

FENativeScriptModule* FEResourceManager::CreateNativeScriptModule(std::string Name, std::string ForceObjectID)
{
	if (Name.empty())
		Name = "Unnamed NativeScriptModule";

	FENativeScriptModule* NewNativeScriptModule = new FENativeScriptModule();
	if (!ForceObjectID.empty())
	{
		NativeScriptModules[ForceObjectID] = NewNativeScriptModule;
		NativeScriptModules[ForceObjectID]->SetID(ForceObjectID);
	}
	else
	{
		NativeScriptModules[NewNativeScriptModule->ID] = NewNativeScriptModule;
	}

	NativeScriptModules[NewNativeScriptModule->ID]->SetName(Name);
	return NativeScriptModules[NewNativeScriptModule->ID];
}

FENativeScriptModule* FEResourceManager::CreateNativeScriptModule(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles, std::string Name, std::string ForceObjectID)
{
	if (DebugDLLFilePath.empty())
	{
		LOG.Add("call of FEResourceManager::CreateNativeScriptModule with empty DebugDLLFilePath", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (DebugPDBFilePath.empty())
	{
		LOG.Add("call of FEResourceManager::CreateNativeScriptModule with empty DebugPDBFilePath", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (ReleaseDLLFilePath.empty())
	{
		LOG.Add("call of FEResourceManager::CreateNativeScriptModule with empty ReleaseDLLFilePath", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return nullptr;
	}

	if (Name.empty())
		Name = "Unnamed NativeScriptModule";

	// First we need to check if files are valid.
	if (!FILE_SYSTEM.DoesFileExist(DebugDLLFilePath))
	{
		LOG.Add("can't locate file: " + DebugDLLFilePath + " in FEResourceManager::CreateNativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	if (!FILE_SYSTEM.DoesFileExist(DebugPDBFilePath))
	{
		LOG.Add("can't locate file: " + DebugPDBFilePath + " in FEResourceManager::CreateNativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	if (!FILE_SYSTEM.DoesFileExist(ReleaseDLLFilePath))
	{
		LOG.Add("can't locate file: " + ReleaseDLLFilePath + " in FEResourceManager::CreateNativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	// We also need to retrieve DLL module ID from debug and release DLLs.
	std::string DebugDLLModuleID = ReadDLLModuleID(DebugDLLFilePath);
	if (DebugDLLModuleID.empty())
	{
		LOG.Add("FEResourceManager::CreateNativeScriptModule failed to get DLLModuleID from DLL: " + DebugDLLFilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	std::string ReleaseDLLModuleID = ReadDLLModuleID(DebugDLLFilePath);
	if (ReleaseDLLModuleID.empty())
	{
		LOG.Add("FEResourceManager::CreateNativeScriptModule failed to get DLLModuleID from DLL: " + DebugDLLFilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	if (DebugDLLModuleID != ReleaseDLLModuleID)
	{
		LOG.Add("FEResourceManager::CreateNativeScriptModule DLLModuleID mismatch between debug and release DLLs", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	FENativeScriptModule* NewNativeScriptModule = new FENativeScriptModule(DebugDLLFilePath, DebugPDBFilePath, ReleaseDLLFilePath, ScriptFiles);
	if (!ForceObjectID.empty())
	{
		NativeScriptModules[ForceObjectID] = NewNativeScriptModule;
		NativeScriptModules[ForceObjectID]->SetID(ForceObjectID);
	}
	else
	{
		NativeScriptModules[NewNativeScriptModule->ID] = NewNativeScriptModule;
	}

	NativeScriptModules[NewNativeScriptModule->ID]->SetName(Name);
	return NativeScriptModules[NewNativeScriptModule->ID];
}

FENativeScriptModule* FEResourceManager::LoadFENativeScriptModule(std::string FileName)
{
	if (FileName.empty())
	{
		LOG.Add("call of FEResourceManager::LoadFENativeScriptModule with empty FileName", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	if (!FILE_SYSTEM.DoesFileExist(FileName))
	{
		LOG.Add("can't locate file: " + FileName + " in FEResourceManager::LoadFENativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	std::fstream File;
	File.open(FileName, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("can't open file: " + FileName + " in FEResourceManager::LoadFENativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	float Version;
	File.read((char*)&Version, sizeof(float));
	if (Version != FE_NATIVE_SCRIPT_MODULE_VERSION)
	{
		LOG.Add("version mismatch in FEResourceManager::LoadFENativeScriptModule", "FE_LOG_LOADING", FE_LOG_ERROR);
		File.close();
		return nullptr;
	}

	FENativeScriptModule* NewNativeScriptModule = new FENativeScriptModule();
	FEObjectLoadedData ObjectData = OBJECT_MANAGER.LoadFEObjectPart(File);
	NewNativeScriptModule->SetID(ObjectData.ID);
	NewNativeScriptModule->SetTag(ObjectData.Tag);
	NewNativeScriptModule->SetName(ObjectData.Name);
	NewNativeScriptModule->SetType(ObjectData.Type);

	// Load DebugDLLAssetID.
	size_t DebugDllAssetIDSize = 0;
	File.read((char*)&DebugDllAssetIDSize, sizeof(size_t));
	char* DebugDllAssetID = new char[DebugDllAssetIDSize];
	File.read(DebugDllAssetID, DebugDllAssetIDSize);
	NewNativeScriptModule->DebugDLLAssetID = std::string(DebugDllAssetID, DebugDllAssetIDSize);
	delete[] DebugDllAssetID;

	// Load DebugPDBAssetID.
	size_t DebugPdbAssetIDSize = 0;
	File.read((char*)&DebugPdbAssetIDSize, sizeof(size_t));
	char* DebugPdbAssetID = new char[DebugPdbAssetIDSize];
	File.read(DebugPdbAssetID, DebugPdbAssetIDSize);
	NewNativeScriptModule->DebugPDBAssetID = std::string(DebugPdbAssetID, DebugPdbAssetIDSize);
	delete[] DebugPdbAssetID;

	// Load ReleaseDLLAssetID.
	size_t ReleaseDllAssetIDSize = 0;
	File.read((char*)&ReleaseDllAssetIDSize, sizeof(size_t));
	char* ReleaseDllAssetID = new char[ReleaseDllAssetIDSize];
	File.read(ReleaseDllAssetID, ReleaseDllAssetIDSize);
	NewNativeScriptModule->ReleaseDLLAssetID = std::string(ReleaseDllAssetID, ReleaseDllAssetIDSize);
	delete[] ReleaseDllAssetID;

	// Load CMakeFileAssetID.
	size_t CMakeFileAssetIDSize = 0;
	File.read((char*)&CMakeFileAssetIDSize, sizeof(size_t));
	char* CMakeFileAssetID = new char[CMakeFileAssetIDSize];
	File.read(CMakeFileAssetID, CMakeFileAssetIDSize);
	NewNativeScriptModule->CMakeFileAssetID = std::string(CMakeFileAssetID, CMakeFileAssetIDSize);
	delete[] CMakeFileAssetID;

	// Load ScriptAssetIDs.
	size_t ScriptAssetIDsSize = 0;
	File.read((char*)&ScriptAssetIDsSize, sizeof(size_t));
	for (size_t i = 0; i < ScriptAssetIDsSize; i++)
	{
		size_t ScriptAssetIDSize = 0;
		File.read((char*)&ScriptAssetIDSize, sizeof(size_t));
		char* ScriptAssetID = new char[ScriptAssetIDSize];
		File.read(ScriptAssetID, ScriptAssetIDSize);
		NewNativeScriptModule->ScriptAssetIDs.push_back(std::string(ScriptAssetID, ScriptAssetIDSize));
		delete[] ScriptAssetID;
	}

	// Load ScriptAssetPackage.
	size_t PackageFullCopySize = 0;
	File.read((char*)&PackageFullCopySize, sizeof(size_t));
	unsigned char* PackageFullCopy = new unsigned char[PackageFullCopySize];
	File.read((char*)PackageFullCopy, PackageFullCopySize);
	NewNativeScriptModule->ScriptAssetPackage = new FEAssetPackage();
	NewNativeScriptModule->ScriptAssetPackage->LoadFromMemory(PackageFullCopy, PackageFullCopySize);
	delete[] PackageFullCopy;

	// Load project data.
	size_t ProjectPackageFullCopySize = 0;
	File.read((char*)&ProjectPackageFullCopySize, sizeof(size_t));
	if (ProjectPackageFullCopySize > 0)
	{
		unsigned char* ProjectPackageFullCopy = new unsigned char[ProjectPackageFullCopySize];
		File.read((char*)ProjectPackageFullCopy, ProjectPackageFullCopySize);

		FEAssetPackage* ProjectData = new FEAssetPackage();
		ProjectData->LoadFromMemory(ProjectPackageFullCopy, ProjectPackageFullCopySize);
		NewNativeScriptModule->Project->Initialize(ProjectData);
		delete[] ProjectPackageFullCopy;
	}

	File.close();

	NativeScriptModules[NewNativeScriptModule->ID] = NewNativeScriptModule;
	return NativeScriptModules[NewNativeScriptModule->ID];
}

void FEResourceManager::SaveFENativeScriptModule(FENativeScriptModule* NativeScriptModule, std::string FileName)
{
	if (NativeScriptModule == nullptr)
	{
		LOG.Add("call of FEResourceManager::SaveFENativeScriptModule with nullptr NativeScriptModule", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	if (FileName.empty())
	{
		LOG.Add("call of FEResourceManager::SaveFENativeScriptModule with empty FileName", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	std::fstream File;
	File.open(FileName, std::ios::out | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("can't open file: " + FileName + " in FEResourceManager::SaveFENativeScriptModule", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	// Version of FENativeScriptModule file.
	float Version = FE_NATIVE_SCRIPT_MODULE_VERSION;
	File.write((char*)&Version, sizeof(float));

	OBJECT_MANAGER.SaveFEObjectPart(File, NativeScriptModule);

	// Save DebugDLLAssetID.
	size_t DebugDllAssetIDSize = NativeScriptModule->DebugDLLAssetID.size();
	File.write((char*)&DebugDllAssetIDSize, sizeof(size_t));
	File.write(NativeScriptModule->DebugDLLAssetID.c_str(), DebugDllAssetIDSize);

	// Save DebugPDBAssetID.
	size_t DebugPdbAssetIDSize = NativeScriptModule->DebugPDBAssetID.size();
	File.write((char*)&DebugPdbAssetIDSize, sizeof(size_t));
	File.write(NativeScriptModule->DebugPDBAssetID.c_str(), DebugPdbAssetIDSize);

	// Save ReleaseDLLAssetID.
	size_t ReleaseDllAssetIDSize = NativeScriptModule->ReleaseDLLAssetID.size();
	File.write((char*)&ReleaseDllAssetIDSize, sizeof(size_t));
	File.write(NativeScriptModule->ReleaseDLLAssetID.c_str(), ReleaseDllAssetIDSize);

	// Save CMakeFileAssetID.
	size_t CMakeFileAssetIDSize = NativeScriptModule->CMakeFileAssetID.size();
	File.write((char*)&CMakeFileAssetIDSize, sizeof(size_t));
	File.write(NativeScriptModule->CMakeFileAssetID.c_str(), CMakeFileAssetIDSize);

	// Save ScriptAssetIDs.
	size_t ScriptAssetIDsSize = NativeScriptModule->ScriptAssetIDs.size();
	File.write((char*)&ScriptAssetIDsSize, sizeof(size_t));
	for (size_t i = 0; i < NativeScriptModule->ScriptAssetIDs.size(); i++)
	{
		size_t ScriptAssetIDSize = NativeScriptModule->ScriptAssetIDs[i].size();
		File.write((char*)&ScriptAssetIDSize, sizeof(size_t));
		File.write(NativeScriptModule->ScriptAssetIDs[i].c_str(), ScriptAssetIDSize);
	}

	// Save ScriptAssetPackage.
	size_t PackageFullCopySize = 0;
	unsigned char* PackageFullCopy = NativeScriptModule->ScriptAssetPackage->ExportAsRawData(PackageFullCopySize);
	File.write((char*)&PackageFullCopySize, sizeof(size_t));
	File.write((char*)PackageFullCopy, PackageFullCopySize);

	// Saveing project data.
	size_t ProjectPackageFullCopySize = 0;
	if (NativeScriptModule->Project != nullptr)
	{
		NativeScriptModule->Project->UpdateDataToRecoverVSProject();
		if (NativeScriptModule->Project->DataToRecoverVSProject == nullptr)
		{
			LOG.Add("NativeScriptModule->Project->DataToRecoverVSProject is nullptr in FEResourceManager::SaveFENativeScriptModule", "FE_LOG_SAVING", FE_LOG_WARNING);
			File.write((char*)&ProjectPackageFullCopySize, sizeof(size_t));
		}
		else
		{
			unsigned char* ProjectPackageFullCopy = NativeScriptModule->Project->DataToRecoverVSProject->ExportAsRawData(ProjectPackageFullCopySize);
			File.write((char*)&ProjectPackageFullCopySize, sizeof(size_t));
			File.write((char*)ProjectPackageFullCopy, ProjectPackageFullCopySize);
		}
	}
	else
	{
		LOG.Add("NativeScriptModule->Project is nullptr in FEResourceManager::SaveFENativeScriptModule", "FE_LOG_SAVING", FE_LOG_WARNING);
		File.write((char*)&ProjectPackageFullCopySize, sizeof(size_t));
	}

	File.close();
}

bool FEResourceManager::DeleteNativeScriptModuleInternal(FENativeScriptModule* Module)
{
	if (Module == nullptr)
	{
		LOG.Add("call of FEResourceManager::DeleteNativeScriptModuleInternal with nullptr Module", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (Module->GetTag() == ENGINE_RESOURCE_TAG)
	{
		LOG.Add("can't delete Engine Private NativeScriptModule", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (NativeScriptModules.find(Module->GetObjectID()) == NativeScriptModules.end())
	{
		LOG.Add("can't find Module in NativeScriptModules in FEResourceManager::DeleteNativeScriptModuleInternal", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	NativeScriptModules.erase(Module->GetObjectID());
	delete Module;

	return true;
}

FEAssetPackage* FEResourceManager::CreateEngineHeadersAssetPackage()
{
	FEAssetPackage* EngineHeadersAssetPackage = new FEAssetPackage();
	EngineHeadersAssetPackage->SetName("EngineHeaders");
	if (EngineHeadersAssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::CreateEngineHeadersAssetPackage: Error creating asset package", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FEResourceManager::CreateEngineHeadersAssetPackage: Engine folder does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);
	// After having all files in the engine folder, we need to filter out only the header files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 2) == ".h" || AllFiles[i].substr(AllFiles[i].size() - 4) == ".hpp" || AllFiles[i].substr(AllFiles[i].size() - 4) == ".inl")
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(EnginePath.size());
			EntryData.Type = "Text";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "Engine header file";

			EngineHeadersAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineHeadersAssetPackage;
}

bool FEResourceManager::UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Asset package is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Destination path is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Destination path does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Asset package is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		std::string LocalPath = std::filesystem::path(AssetPackageContent[i].Name).parent_path().string();
		// Since we are not using folders in FEAssetPackage, we need to create all folders in the file path.
		// First we need to get chain of folders.
		std::vector<std::string> FolderChain;
		try
		{
			std::filesystem::path Directory(LocalPath);
			while (!Directory.string().empty())
			{
				if (!FolderChain.empty())
				{
					if (FolderChain.back() == Directory.string())
						break;
				}
				FolderChain.push_back(Directory.string());
				Directory = Directory.parent_path();
			}

			std::reverse(FolderChain.begin(), FolderChain.end());
		}
		catch (const std::exception& Exception)
		{
			LOG.Add("Error in FEResourceManager::UnPackEngineHeadersAssetPackage: " + std::string(Exception.what()), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		// Then we will go from the root folder to the last folder and create them if they do not exist.
		for (size_t i = 0; i < FolderChain.size(); i++)
		{
			std::string FinalPath = Path + FolderChain[i];
			if (!FILE_SYSTEM.DoesDirectoryExist(FinalPath))
			{
				if (!FILE_SYSTEM.CreateDirectory(FinalPath))
				{
					LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Error creating directory " + FinalPath, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
					return false;
				}
			}
		}

		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

FEAssetPackage* FEResourceManager::CreateEngineSourceFilesAssetPackage()
{
	FEAssetPackage* EngineSourceFilesAssetPackage = new FEAssetPackage();
	EngineSourceFilesAssetPackage->SetName("EngineSourceFiles");
	if (EngineSourceFilesAssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::CreateEngineSourceFilesAssetPackage: Error creating asset package", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FEResourceManager::CreateEngineSourceFilesAssetPackage: Engine folder does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);
	// After having all files in the engine folder, we need to filter out only the source files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 4) == ".cpp" || AllFiles[i].substr(AllFiles[i].size() - 2) == ".c")
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(EnginePath.size());
			EntryData.Type = "Text";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "Engine source file";

			EngineSourceFilesAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineSourceFilesAssetPackage;
}

bool FEResourceManager::UnPackEngineSourceFilesAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Asset package is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Destination path is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Destination path does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Asset package is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		std::string LocalPath = std::filesystem::path(AssetPackageContent[i].Name).parent_path().string();
		// Since we are not using folders in FEAssetPackage, we need to create all folders in the file path.
		// First we need to get chain of folders.
		std::vector<std::string> FolderChain;
		try
		{
			std::filesystem::path Directory(LocalPath);
			while (!Directory.string().empty())
			{
				if (!FolderChain.empty())
				{
					if (FolderChain.back() == Directory.string())
						break;
				}
				FolderChain.push_back(Directory.string());
				Directory = Directory.parent_path();
			}

			std::reverse(FolderChain.begin(), FolderChain.end());
		}
		catch (const std::exception& Exception)
		{
			LOG.Add("Error in FEResourceManager::UnPackEngineSourceFilesAssetPackage: " + std::string(Exception.what()), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		// Then we will go from the root folder to the last folder and create them if they do not exist.
		for (size_t j = 0; j < FolderChain.size(); j++)
		{
			std::string FinalPath = Path + FolderChain[j];
			if (!FILE_SYSTEM.DoesDirectoryExist(FinalPath))
			{
				if (!FILE_SYSTEM.CreateDirectory(FinalPath))
				{
					LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Error creating directory " + FinalPath, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
					return false;
				}
			}
		}

		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FEResourceManager::UnPackEngineSourceFilesAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

FEAssetPackage* FEResourceManager::CreateEngineLIBAssetPackage()
{
	FEAssetPackage* EngineLIBAssetPackage = new FEAssetPackage();
	EngineLIBAssetPackage->SetName("EngineLib");
	if (EngineLIBAssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::CreateEngineLIBAssetPackage: Error creating asset package", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FEResourceManager::CreateEngineLIBAssetPackage: Engine folder does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);

	std::vector<std::string> DebugStrings;
	// After having all files in the engine folder, we need to filter out only the lib files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 4) == ".lib")
		{
			// FIX ME! Currently projects would need only debug lib files. Is it correct?
			// It will grab either Debug or Release lib files. It should be more deterministic.
			// And only FocalEngine.lib and FEBasicApplication.lib are needed.
			if (AllFiles[i].find("FocalEngine.lib") == std::string::npos && AllFiles[i].find("FEBasicApplication.lib") == std::string::npos)
				continue;

			FEAssetPackageEntryIntializeData EntryData;
			EntryData.Name = FILE_SYSTEM.GetFileName(AllFiles[i]);
			DebugStrings.push_back(EntryData.Name);
			EntryData.Type = "BINARY";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "Engine lib file";

			EngineLIBAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineLIBAssetPackage;
}

bool FEResourceManager::UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::UnPackEngineLIBAssetPackage: Asset package is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineLIBAssetPackage: Destination path is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FEResourceManager::UnPackEngineLIBAssetPackage: Destination path does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FEResourceManager::UnPackEngineLIBAssetPackage: Asset package is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FEResourceManager::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool FEResourceManager::CopyEngineFiles(bool bCopyEngineHeaders, bool bCopyEngineSourceFiles, bool bCopyEngineLIBs, std::string DestinationDirectory)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(DestinationDirectory))
	{
		LOG.Add("FEResourceManager::CopyEngineFiles: DestinationDirectory does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (bCopyEngineHeaders)
	{
		FEAssetPackage* EngineHeadersPackage = RESOURCE_MANAGER.CreateEngineHeadersAssetPackage();
		if (EngineHeadersPackage == nullptr)
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error creating engine headers asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!RESOURCE_MANAGER.UnPackEngineHeadersAssetPackage(EngineHeadersPackage, DestinationDirectory + "SubSystems/FocalEngine/"))
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error unpacking engine headers asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	if (bCopyEngineSourceFiles)
	{
		FEAssetPackage* EngineSourcePackage = RESOURCE_MANAGER.CreateEngineSourceFilesAssetPackage();
		if (EngineSourcePackage == nullptr)
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error creating engine source files asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!RESOURCE_MANAGER.UnPackEngineSourceFilesAssetPackage(EngineSourcePackage, DestinationDirectory + "SubSystems/FocalEngine/"))
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error unpacking engine source files asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	if (bCopyEngineLIBs)
	{
		FEAssetPackage* EngineLIBPackage = RESOURCE_MANAGER.CreateEngineLIBAssetPackage();
		if (EngineLIBPackage == nullptr)
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error creating engine lib asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!RESOURCE_MANAGER.UnPackEngineLIBAssetPackage(EngineLIBPackage, DestinationDirectory))
		{
			LOG.Add("FEResourceManager::CopyEngineFiles: Error unpacking engine lib asset package.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

FEAssetPackage* FEResourceManager::CreatePrivateEngineAssetPackage()
{
	FEAssetPackage* PrivateEngineAssetPackage = new FEAssetPackage();
	PrivateEngineAssetPackage->SetName("PrivateEngineResources");
	if (PrivateEngineAssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::CreatePrivateEngineAssetPackage: Error creating asset package", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FEResourceManager::CreatePrivateEngineAssetPackage: Engine folder does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return nullptr;
	}

	// FIX ME! It is temporary solution.
	// I will just dump all related files in the engine folder to the asset package.
	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath + "/Resources/", false);
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 8) == ".texture" ||
			AllFiles[i].substr(AllFiles[i].size() - 6) == ".model" ||
			AllFiles[i].substr(AllFiles[i].size() - 19) == ".nativescriptmodule")
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(FILE_SYSTEM.GetCurrentWorkingPath().size());
			EntryData.Type = AllFiles[i].substr(AllFiles[i].size() - 8) == ".texture" ? "FE_TEXTURE" : AllFiles[i].substr(AllFiles[i].size() - 19) == ".nativescriptmodule" ? "FE_NATIVE_SCRIPT_MODULE" : "FE_GAME_MODEL";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "";

			PrivateEngineAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	// And files from CoreExtensions.
	AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath + "/CoreExtensions/", true);
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 5) == ".glsl")
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(FILE_SYSTEM.GetCurrentWorkingPath().size());
			EntryData.Type = "GLSL";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "";

			PrivateEngineAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	//PrivateEngineAssetPackage->ImportAsset(RESOURCE_MANAGER.NoTexture);
	return PrivateEngineAssetPackage;
}

bool FEResourceManager::UnPackPrivateEngineAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Asset package is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Destination path is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Destination path does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Asset package is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		std::string LocalPath = std::filesystem::path(AssetPackageContent[i].Name).parent_path().string();
		// Since we are not using folders in FEAssetPackage, we need to create all folders in the file path.
		// First we need to get chain of folders.
		std::vector<std::string> FolderChain;
		try
		{
			std::filesystem::path Directory(LocalPath);
			while (!Directory.string().empty())
			{
				if (!FolderChain.empty())
				{
					if (FolderChain.back() == Directory.string())
						break;
				}
				FolderChain.push_back(Directory.string());
				Directory = Directory.parent_path();
			}

			std::reverse(FolderChain.begin(), FolderChain.end());
		}
		catch (const std::exception& Exception)
		{
			LOG.Add("Error in FEResourceManager::UnPackPrivateEngineAssetPackage: " + std::string(Exception.what()), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		// Then we will go from the root folder to the last folder and create them if they do not exist.
		for (size_t j = 0; j < FolderChain.size(); j++)
		{
			std::string FinalPath = Path + FolderChain[j];
			if (!FILE_SYSTEM.DoesDirectoryExist(FinalPath))
			{
				if (!FILE_SYSTEM.CreateDirectory(FinalPath))
				{
					LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Error creating directory " + FinalPath, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
					return false;
				}
			}
		}

		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FEResourceManager::UnPackPrivateEngineAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}