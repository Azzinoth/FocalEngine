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

FEResourceManager::FEResourceManager()
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);

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
	FETexture::MarkAsPersistent(NoTexture->GetTextureID());

	FEShader* NewShader = CreateShader("FECombineFrameBuffers", LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																LoadGLSL((EngineFolder + "CoreExtensions//PostProcessEffects//FE_CombineFrameBuffers_FS.glsl").c_str()).c_str(),
																nullptr, nullptr, nullptr, nullptr,
																"5C267A01466A545E7D1A2E66");
	NewShader->SetTag(ENGINE_RESOURCE_TAG);

	LoadStandardMaterial();
	LoadStandardMeshes();
	LoadStandardGameModels();

	// Load all standard script modules.
	std::vector<std::string> PotentialScriptModuleFiles = FILE_SYSTEM.GetFileNamesInDirectory(ResourcesFolder);
	for (size_t i = 0; i < PotentialScriptModuleFiles.size(); i++)
	{
		if (PotentialScriptModuleFiles[i].substr(PotentialScriptModuleFiles[i].size() - 19, 19) == ".nativescriptmodule")
		{
			LoadFENativeScriptModule((ResourcesFolder + PotentialScriptModuleFiles[i]).c_str());
		}
	}

	if (laszip_load_dll() != 0)
	{
		LOG.Add("DLL ERROR: loading LASzip DLL failed.", "FE_LOG_LOADING", FE_LOG_ERROR);
		bIsLasLazFilesEnabled = false;
		LOG.Add("Loading LAS/LAZ files is disabled.", "FE_LOG_LOADING", FE_LOG_WARNING);
	}
	else
	{
		bIsLasLazFilesEnabled = true;
	}
}

FEResourceManager::~FEResourceManager()
{
	Clear();
}

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

	SetTagInternal(Object, NewTag);
	return true;
}

void FEResourceManager::SetTagInternal(FEObject* Object, std::string NewTag)
{
	if (Object == nullptr)
	{
		LOG.Add("Object is nullptr in function FEResourceManager::SetTagInternal.", "FE_LOG_GENERAL", FE_LOG_ERROR);
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
		FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RED, GL_UNSIGNED_SHORT, RawExtractedData.data());
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
		FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawExtractedData.data());

		if (NewTexture->MipEnabled)
		{
			FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
			// TO-DO: make it configurable.
			FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
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
	FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, Format, GL_UNSIGNED_BYTE, TextureData);

	if (NewTexture->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		// TO-DO: make it configurable.
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
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
	if (FILE_SYSTEM.DoesFileExist(FileName) == false)
	{
		LOG.Add("File does not exist: " + std::string(FileName) + " in function FEResourceManager::LoadFETexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return this->NoTexture;
	}

	std::fstream File;
	File.open(FileName, std::ios::in | std::ios::binary | std::ios::ate);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function FEResourceManager::LoadFETexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
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
		// TO-DO: make it configurable.
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
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

FEMesh* FEResourceManager::RawDataToMesh(float* Positions, const int PositionsCount,
										 float* UV, const int UVCount,
										 float* Normals, const int NormalsCount,
										 float* Tangents, const int TangentsCount,
										 int* Indices, const int IndicesCount,
										 float* Colors, int ColorsCount,
										 float* MaterialIndices, const int MaterialIndicesCount, const int MaterialCount,
										 const std::string Name)
{
	if (PositionsCount == 0)
	{
		LOG.Add("FEResourceManager::RawDataToMesh: PositionsCount is 0, can't create mesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	if (IndicesCount == 0)
	{
		LOG.Add("FEResourceManager::RawDataToMesh: IndicesCount is 0, can't create mesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	int VertexType = FE_POSITION | FE_INDEX;

	GLuint VaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &VaoID));
	FE_GL_ERROR(glBindVertexArray(VaoID));

	GLuint IndicesBufferID;
	// Index
	FE_GL_ERROR(glGenBuffers(1, &IndicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * IndicesCount, Indices, GL_STATIC_DRAW));

	GLuint PositionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &PositionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PositionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PositionsCount, Positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint ColorsBufferID = 0;
	if (Colors != nullptr && ColorsCount != 0)
	{
		VertexType |= FE_COLOR;
		// colors
		FE_GL_ERROR(glGenBuffers(1, &ColorsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ColorsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ColorsCount, Colors, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint NormalsBufferID = 0;
	if (Normals != nullptr && NormalsCount != 0)
	{
		VertexType |= FE_NORMAL;
		// normals
		FE_GL_ERROR(glGenBuffers(1, &NormalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, NormalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NormalsCount, Normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint TangentsBufferID = 0;
	if (Tangents != nullptr && TangentsCount != 0)
	{
		VertexType |= FE_TANGENTS;
		// tangents
		FE_GL_ERROR(glGenBuffers(1, &TangentsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, TangentsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TangentsCount, Tangents, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr && UVCount != 0)
	{
		VertexType |= FE_UV;
		// UV
		FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVCount, UV, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint MaterialsIndicesBufferID = -1;
	if (MaterialIndices != nullptr && MaterialIndicesCount > 1)
	{
		// Material ID
		FE_GL_ERROR(glGenBuffers(1, &MaterialsIndicesBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, MaterialsIndicesBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MaterialIndicesCount, MaterialIndices, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(5/*FE_MATINDEX*/, 1, GL_FLOAT, false, 0, nullptr));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	//int vertexType = FE_POSITION | FE_UV | FE_NORMAL | FE_TANGENTS | FE_INDEX;
	if (MaterialCount > 1)
	{
		VertexType |= FE_MATINDEX;
	}

	FEMesh* NewMesh = CreateMesh(VaoID, IndicesCount, VertexType, FEAABB(Positions, PositionsCount), Name);
	NewMesh->IndicesCount = IndicesCount;
	NewMesh->IndicesBufferID = IndicesBufferID;

	NewMesh->PositionsCount = PositionsCount;
	NewMesh->PositionsBufferID = PositionsBufferID;

	NewMesh->ColorCount = ColorsCount;
	NewMesh->ColorBufferID = ColorsBufferID;

	NewMesh->NormalsCount = NormalsCount;
	NewMesh->NormalsBufferID = NormalsBufferID;

	NewMesh->TangentsCount = TangentsCount;
	NewMesh->TangentsBufferID = TangentsBufferID;

	NewMesh->UVCount = UVCount;
	NewMesh->UVBufferID = UVBufferID;

	NewMesh->MaterialsIndicesCount = MaterialIndicesCount;
	NewMesh->MaterialsIndicesBufferID = MaterialsIndicesBufferID;

	NewMesh->MaterialsCount = MaterialCount;

	return NewMesh;
}

FEMesh* FEResourceManager::RawPLYDataToFEMesh(FERawPLYData* PLYData, std::string Name, std::string ForceObjectID)
{
	FEMesh* NewMesh = nullptr;
	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return NewMesh;
	}

	if (Name.empty())
		Name = "unnamedMesh";

	bool bIsMesh = DoesPLYContainMesh(PLYData);
	if (!bIsMesh)
	{
		LOG.Add("PLYData is not a mesh in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return NewMesh;
	}

	//std::vector<glm::vec3> Vertices;
	std::vector<float> PositionsRawData;
	std::variant<std::vector<glm::vec3>, std::vector<glm::dvec3>> Positions = ExtractPositionsFromPLYData(PLYData);
	if (auto* FloatPositionVector = std::get_if<std::vector<glm::vec3>>(&Positions))
	{
		if (FloatPositionVector->empty())
		{
			LOG.Add("Can't extract positions from PLYData in function FEResourceManager::RawPLYDataToFEMesh.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return NewMesh;
		}

		for (size_t i = 0; i < FloatPositionVector->size(); i++)
		{
			PositionsRawData.push_back(FloatPositionVector->at(i).x);
			PositionsRawData.push_back(FloatPositionVector->at(i).y);
			PositionsRawData.push_back(FloatPositionVector->at(i).z);
		}
	}
	else if (auto* DoublePositionVector = std::get_if<std::vector<glm::dvec3>>(&Positions))
	{
		if (DoublePositionVector->empty())
		{
			LOG.Add("Can't extract positions from PLYData in function FEResourceManager::RawPLYDataToFEMesh.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return NewMesh;
		}

		// Right now we are not supporting double precision positions.
		// So we will convert them to float.
		for (size_t i = 0; i < DoublePositionVector->size(); i++)
		{
			PositionsRawData.push_back(static_cast<float>(DoublePositionVector->at(i).x));
			PositionsRawData.push_back(static_cast<float>(DoublePositionVector->at(i).y));
			PositionsRawData.push_back(static_cast<float>(DoublePositionVector->at(i).z));
		}
	}
	else
	{
		LOG.Add("Can't extract positions from PLYData in function FEResourceManager::RawPLYDataToFEMesh.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return NewMesh;
	}

	std::vector<int> Indices = ExtractIndicesFromPLYData(PLYData);
	if (Indices.empty())
	{
		LOG.Add("Can't extract indices from PLYData in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return NewMesh;
	}

	std::vector<std::vector<unsigned char>> Color = ExtractColorsFromPLYData(PLYData);
	std::vector<float> ConvertedColors;
	for (size_t i = 0; i < Color.size(); i++)
	{
		ConvertedColors.push_back(static_cast<float>(Color[i][0]) / 255.0f);
		ConvertedColors.push_back(static_cast<float>(Color[i][1]) / 255.0f);
		ConvertedColors.push_back(static_cast<float>(Color[i][2]) / 255.0f);
	}

	bool bTextureCoordinatesArePartOfVertex = false;
	std::vector<glm::vec2> UV = ExtractUVsFromPLYData(PLYData, bTextureCoordinatesArePartOfVertex);
	std::vector<float> ConvertedUV;
	for (size_t i = 0; i < UV.size(); i++)
	{
		ConvertedUV.push_back(UV[i].x);
		ConvertedUV.push_back(1.0f - UV[i].y);
	}
	
	std::vector<glm::vec3> Normals = ExtractNormalsFromPLYData(PLYData);
	std::vector<float> ConvertedNormals;
	if (Normals.empty())
	{
		ConvertedNormals.resize(PositionsRawData.size());
		GEOMETRY.CalculateNormals(Indices, PositionsRawData, ConvertedNormals);
	}
	else
	{
		for (size_t i = 0; i < Normals.size(); i++)
		{
			ConvertedNormals.push_back(Normals[i].x);
			ConvertedNormals.push_back(Normals[i].y);
			ConvertedNormals.push_back(Normals[i].z);
		}
	}

	std::vector<float> Tangents;
	if (ConvertedUV.size() > 0 && ConvertedNormals.size() > 0)
	{
		Tangents.resize(ConvertedNormals.size());
		GEOMETRY.CalculateTangents(Indices, PositionsRawData, ConvertedUV, ConvertedNormals, Tangents);
	}

	return RawDataToMesh(PositionsRawData.data(), static_cast<int>(PositionsRawData.size()),
						 ConvertedUV.data(), static_cast<int>(ConvertedUV.size()),
						 ConvertedNormals.data(), static_cast<int>(ConvertedNormals.size()),
						 Tangents.data(), static_cast<int>(Tangents.size()),
						 Indices.data(), static_cast<int>(Indices.size()),
						 ConvertedColors.data(), static_cast<int>(ConvertedColors.size()),
						 nullptr, 0, 0, Name);
}

bool FEResourceManager::ExportFEMeshToOBJ(FEMesh* MeshToExport, const char* FileName)
{
	if (MeshToExport == nullptr)
	{
		LOG.Add("MeshToExport is nullptr in function FEResourceManager::ExportFEMeshToOBJ.", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	if (FileName == nullptr)
	{
		LOG.Add("FileName is nullptr in function FEResourceManager::ExportFEMeshToOBJ.", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	FEObjLoader& OBJLoader = FEObjLoader::GetInstance();
	// Create a raw OBJ data object to hold the mesh data
	FERawOBJData Data;

	// Extract data from the mesh
	const size_t VertexCount = MeshToExport->GetPositionsCount() / 3;
	const size_t NormalCount = MeshToExport->GetNormalsCount() / 3;
	const size_t UVCount = MeshToExport->GetUVCount() / 2;
	const size_t IndexCount = MeshToExport->GetIndicesCount();
	const size_t ColorCount = MeshToExport->GetColorCount() / 3;

	OBJLoader.bHaveNormalCoord = NormalCount > 0 ? true : false;
	OBJLoader.bHaveTextureCoord = UVCount > 0 ? true : false;
	OBJLoader.bHaveColors = ColorCount > 0 ? true : false;

	// Get buffer data from GPU
	float* Positions = new float[MeshToExport->GetPositionsCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetPositionsBufferID(), 0, sizeof(float) * MeshToExport->GetPositionsCount(), Positions));

	float* Normals = nullptr;
	if (NormalCount > 0)
	{
		Normals = new float[MeshToExport->GetNormalsCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetNormalsBufferID(), 0, sizeof(float) * MeshToExport->GetNormalsCount(), Normals));
	}

	float* UVs = nullptr;
	if (UVCount > 0)
	{
		UVs = new float[MeshToExport->GetUVCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetUVBufferID(), 0, sizeof(float) * MeshToExport->GetUVCount(), UVs));
	}

	int* Indices = new int[MeshToExport->GetIndicesCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetIndicesBufferID(), 0, sizeof(int) * MeshToExport->GetIndicesCount(), Indices));

	float* Colors = nullptr;
	if (ColorCount > 0)
	{
		Colors = new float[MeshToExport->GetColorCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetColorBufferID(), 0, sizeof(float) * MeshToExport->GetColorCount(), Colors));
	}

	float* MaterialIndices = nullptr;
	if (MeshToExport->GetMaterialsIndicesCount() > 0)
	{
		MaterialIndices = new float[MeshToExport->GetMaterialsIndicesCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetMaterialsIndicesBufferID(), 0, sizeof(float) * MeshToExport->GetMaterialsIndicesCount(), MaterialIndices));
	}

	// Fill RawOBJData with vertex positions
	for (size_t i = 0; i < VertexCount; i++)
	{
		glm::vec3 VertexPositions(Positions[i * 3], Positions[i * 3 + 1], Positions[i * 3 + 2]);
		Data.RawVertexCoordinates.push_back(VertexPositions);

		if (Colors != nullptr)
		{
			glm::vec3 Color(Colors[i * 3], Colors[i * 3 + 1], Colors[i * 3 + 2]);
			Data.RawVertexColors.push_back(Color);
		}
	}

	// Fill RawOBJData with texture coordinates (UVs)
	if (UVs != nullptr)
	{
		for (size_t i = 0; i < UVCount; i++)
		{
			glm::vec2 UV(UVs[i * 2], UVs[i * 2 + 1]);
			// Flip V coordinate to follow OBJ convention
			//UV.y = 1.0f - UV.y;
			Data.RawTextureCoordinates.push_back(UV);
		}
	}

	// Fill RawOBJData with normal coordinates
	if (Normals != nullptr)
	{
		for (size_t i = 0; i < NormalCount; i++)
		{
			glm::vec3 NormalVector(Normals[i * 3], Normals[i * 3 + 1], Normals[i * 3 + 2]);
			Data.RawNormalCoordinates.push_back(NormalVector);
		}
	}

	// Create material record if needed
	if (MeshToExport->MaterialsCount > 0 && MaterialIndices != nullptr)
	{
		// Default material record as placeholder
		MaterialRecord MaterialRecord;
		MaterialRecord.Name = MeshToExport->GetName() + "_material";
		Data.MaterialRecords.push_back(MaterialRecord);
	}

	// Convert indices to the OBJ format (1-based, not 0-based)
	// OBJ format requires indices for vertex/uv/normal for each face
	for (size_t i = 0; i < IndexCount; i += 3)
	{
		// For each triangle's vertex
		for (size_t j = 0; j < 3; j++)
		{
			int CurrentIndex = Indices[i + j] + 1; // +1 because OBJ indices start at 1

			// In OBJ, each face vertex is defined as v/vt/vn
			Data.RawIndices.push_back(CurrentIndex); // vertex position index

			if (UVCount > 0)
				Data.UVIndices.push_back(CurrentIndex); // texture coordinate index

			if (NormalCount > 0)
				Data.NormalIndices.push_back(CurrentIndex); // normal vector index
		}
	}

	if (Colors != nullptr)
	{
		for (size_t i = 0; i < ColorCount; i += 3)
		{
			glm::vec3 Color(Colors[i], Colors[i + 1], Colors[i + 2]);
			Data.RawVertexColors.push_back(Color);
		}
	}

	// Use FEObjLoader to save the file
	bool Result = OBJLoader.SaveToOBJ(FileName, &Data);

	// Clean up allocated memory
	delete[] Positions;
	if (Normals) delete[] Normals;
	if (UVs) delete[] UVs;
	delete[] Indices;
	if (Colors) delete[] Colors;
	if (MaterialIndices) delete[] MaterialIndices;

	if (Result)
	{
		LOG.Add(std::string("Successfully exported mesh to: ") + FileName, "FE_LOG_SAVING", FE_LOG_INFO);
	}
	else
	{
		LOG.Add(std::string("Failed to export mesh to: ") + FileName, "FE_LOG_SAVING", FE_LOG_ERROR);
	}

	return Result;
}

bool FEResourceManager::ExportFEMeshToPLY(FEMesh* MeshToExport, std::string FileName)
{
	bool bResult = false;

	if (MeshToExport == nullptr)
	{
		LOG.Add("MeshToExport is nullptr in function FEResourceManager::ExportFEMeshToPLY.", "FE_LOG_SAVING", FE_LOG_ERROR);
		return bResult;
	}

	if (FileName.empty())
	{
		LOG.Add("FileName is empty in function FEResourceManager::ExportFEMeshToPLY.", "FE_LOG_SAVING", FE_LOG_ERROR);
		return bResult;
	}

	// Extract data from the mesh
	const size_t VertexCount = MeshToExport->GetPositionsCount() / 3;
	const size_t NormalCount = MeshToExport->GetNormalsCount() / 3;
	const size_t UVCount = MeshToExport->GetUVCount() / 2;
	const size_t IndexCount = MeshToExport->GetIndicesCount();
	const size_t ColorCount = MeshToExport->GetColorCount() / 3;

	// Get buffer data from GPU
	float* Positions = new float[MeshToExport->GetPositionsCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetPositionsBufferID(), 0, sizeof(float) * MeshToExport->GetPositionsCount(), Positions));

	float* Normals = nullptr;
	if (NormalCount > 0)
	{
		Normals = new float[MeshToExport->GetNormalsCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetNormalsBufferID(), 0, sizeof(float) * MeshToExport->GetNormalsCount(), Normals));
	}

	float* UVs = nullptr;
	if (UVCount > 0)
	{
		UVs = new float[MeshToExport->GetUVCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetUVBufferID(), 0, sizeof(float) * MeshToExport->GetUVCount(), UVs));
	}

	int* Indices = new int[MeshToExport->GetIndicesCount()];
	FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetIndicesBufferID(), 0, sizeof(int) * MeshToExport->GetIndicesCount(), Indices));

	float* Colors = nullptr;
	if (ColorCount > 0)
	{
		Colors = new float[MeshToExport->GetColorCount()];
		FE_GL_ERROR(glGetNamedBufferSubData(MeshToExport->GetColorBufferID(), 0, sizeof(float) * MeshToExport->GetColorCount(), Colors));
	}

	FERawPLYData* PLYData = new FERawPLYData();
	PLYData->Header = new FEPLYHeader();
	PLYData->Header->StorageType = PLYFileType::BINARY_LITTLE_ENDIAN;
	PLYData->Header->Comments.push_back("Generated by Focal Engine");
	PLYData->Header->ElementSchemas.push_back(PLYElementSchema());
	PLYData->Header->ElementSchemas[0].Name = "vertex";
	size_t PLYVertexCount = MeshToExport->GetPositionsCount() / 3;
	PLYData->Header->ElementSchemas[0].Count = PLYVertexCount;

	size_t PropertyIndex = 0;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "x";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
	PropertyIndex++;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "y";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
	PropertyIndex++;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "z";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
	PropertyIndex++;

	if (NormalCount > 0)
	{
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "nx";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "ny";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "nz";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
		PropertyIndex++;
	}

	if (UVCount > 0)
	{
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "u";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "v";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::FLOAT;
		PropertyIndex++;
	}

	if (ColorCount > 0)
	{
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "red";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::UCHAR;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "green";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::UCHAR;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "blue";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::UCHAR;
		PropertyIndex++;
		PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Name = "alpha";
		PLYData->Header->ElementSchemas[0].PropertyDefinitions[PropertyIndex].Type = PLYPropertyType::UCHAR;
		PropertyIndex++;
	}

	PLYData->Elements.push_back(PLYElementData());
	PLYData->Elements[0].Description = PLYData->Header->ElementSchemas[0];
	PLYData->Elements[0].Entries.resize(PLYVertexCount);
	
	for (size_t i = 0; i < PLYVertexCount; i++)
	{
		PLYData->Elements[0].Entries[i].PropertyValues.resize(PLYData->Elements[0].Description.PropertyDefinitions.size());
		PropertyIndex = 0;

		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Positions[i * 3];
		PropertyIndex++;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Positions[i * 3 + 1];
		PropertyIndex++;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Positions[i * 3 + 2];
		PropertyIndex++;

		if (NormalCount > 0)
		{
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Normals[i * 3];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Normals[i * 3 + 1];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Normals[i * 3 + 2];
			PropertyIndex++;
		}

		if (UVCount > 0)
		{
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = UVs[i * 2];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = 1.0f - UVs[i * 2 + 1];
			PropertyIndex++;
		}

		if (ColorCount > 0)
		{
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Colors[i * 3];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Colors[i * 3 + 1];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = Colors[i * 3 + 2];
			PropertyIndex++;
			std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[PropertyIndex]) = 255;
			PropertyIndex++;
		}
	}

	PLYData->Header->ElementSchemas.push_back(PLYElementSchema());
	PLYData->Header->ElementSchemas[1].Name = "face";
	size_t PLYIndecesCount = MeshToExport->GetIndicesCount() / 3;
	PLYData->Header->ElementSchemas[1].Count = PLYIndecesCount;

	PLYData->Header->ElementSchemas[1].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[1].PropertyDefinitions[0].Name = "vertex_indices";
	PLYData->Header->ElementSchemas[1].PropertyDefinitions[0].bIsList = true;
	PLYData->Header->ElementSchemas[1].PropertyDefinitions[0].SizeType = PLYPropertyType::UCHAR;
	PLYData->Header->ElementSchemas[1].PropertyDefinitions[0].Type = PLYPropertyType::UINT;

	PLYData->Elements.push_back(PLYElementData());
	PLYData->Elements[1].Description = PLYData->Header->ElementSchemas[1];
	PLYData->Elements[1].Entries.resize(PLYIndecesCount);

	PropertyIndex = 0;
	for (size_t i = 0; i < PLYIndecesCount; i++)
	{
		PLYData->Elements[1].Entries[i].PropertyValues.resize(1);
		PLYData->Elements[1].Entries[i].PropertyValues[PropertyIndex] = PLYListValue();
		PLYListValue& ListValue = std::get<PLYListValue>(PLYData->Elements[1].Entries[i].PropertyValues[PropertyIndex]);

		ListValue.push_back(Indices[i * 3]);
		ListValue.push_back(Indices[i * 3 + 1]);
		ListValue.push_back(Indices[i * 3 + 2]);
	}

	bResult = PLY_MANAGER.SaveToPLY(FileName, PLYData, PLYFileType::BINARY_LITTLE_ENDIAN);
	return bResult;
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

	NewMesh = LoadFEMesh((ResourcesFolder + "Generic_VR_Controller.model").c_str(), "Generic_VR_Controller");
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
			OBJLoader.LoadedObjects[i]->MaterialIDs.data(), static_cast<int>(OBJLoader.LoadedObjects[i]->MaterialIDs.size()), static_cast<int>(OBJLoader.LoadedObjects[i]->MaterialRecords.size()), name));


		// in rawDataToMesh() hidden FEMesh allocation and TextureIterator will go to hash table so we need to use setMeshName() not setName.
		Result.back()->SetName(name);
		Meshes[Result.back()->GetObjectID()] = reinterpret_cast<FEMesh*>(Result.back());
	}

	CreateMaterialsFromOBJData(Result);

	return Result;
}

FEMesh* FEResourceManager::LoadFEMesh(const char* FileName, const std::string Name)
{
	if (FILE_SYSTEM.DoesFileExist(FileName) == false)
	{
		LOG.Add("File does not exist: " + std::string(FileName) + " in function FEResourceManager::LoadFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return GetMesh("84251E6E0D0801363579317R"/*"cube"*/);
	}

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
	Root["Compack packing"] = Material->IsCompactPacking();

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
	NewMaterial->SetCompactPacking(Root["Compack packing"].asBool());

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

	FEShaderUniformValue Color("BaseColor", glm::vec3(1.0f, 0.4f, 0.6f));
	NewMaterial->SetUniformVariation(Color);

	FEShaderUniformValue BrightnessFactor("BrightnessFactor", 1.0f);
	NewMaterial->SetUniformVariation(BrightnessFactor);

	// Generic VR controller material
	NewMaterial = CreateMaterial("FEGenericVRControllerMaterial", "6F381A367E2D683A753C2A79");
	NewMaterial->SetTag(ENGINE_RESOURCE_TAG);
	NewMaterial->Shader = GetShader("6917497A5E0C05454876186F");

	Color.SetValue(glm::vec3(0.1f, 0.1f, 0.1f));
	NewMaterial->SetUniformVariation(Color);
	NewMaterial->SetUniformVariation(BrightnessFactor);

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

	// ****************************** POINT CLOUD SHADERS ******************************

	FEShader* StandardPointCloudShader = CreateShader("StandardPointCloudShader", LoadGLSL((EngineFolder + "CoreExtensions//PointCloudShaders//FE_PointCloud_VS.glsl").c_str()).c_str(),
																				  LoadGLSL((EngineFolder + "CoreExtensions//PointCloudShaders//FE_PointCloud_FS.glsl").c_str()).c_str(),
																				  nullptr, nullptr, nullptr, nullptr);

	// ****************************** POINT CLOUD SHADERS END **************************
}

void FEResourceManager::LoadStandardGameModels()
{
	FEGameModel* NewGameModel = new FEGameModel(GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/), GetMaterial("18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/), "standardGameModel");
	GameModels.erase(NewGameModel->GetObjectID());
	NewGameModel->SetID("67251E393508013ZV579315F");
	NewGameModel->SetTag(ENGINE_RESOURCE_TAG);
	GameModels[NewGameModel->GetObjectID()] = NewGameModel;

	// Generic VR controller game model
	NewGameModel = new FEGameModel(GetMesh("7F784407607A39545A65033A"), GetMaterial("6F381A367E2D683A753C2A79"), "FEGenericVRControllerGameModel");
	GameModels.erase(NewGameModel->GetObjectID());
	NewGameModel->SetID("504029555848336725615C49");
	NewGameModel->SetTag(ENGINE_RESOURCE_TAG);
	NewGameModel->SetScaleFactor(20.0f);
	GameModels[NewGameModel->GetObjectID()] = NewGameModel;
}

void FEResourceManager::Clear()
{
	ClearResource(Materials);
	ClearResource(Meshes);
	ClearResource(LineCollections);
	ClearResource(PointClouds);
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
		Root["LODs"]["Billboard zero rotaion"] = GameModel->GetBillboardZeroRotation();
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
		NewGameModel->SetBillboardZeroRotation(Root["LODs"]["Billboard zero rotaion"].asFloat());

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
FETexture* FEResourceManager::CreateBlankHeightMapTexture(int Width, int Height, std::string Name)
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

	FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, NewTexture->InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RED, GL_UNSIGNED_SHORT, (unsigned char*)RawPixels);
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
		std::string CurrentLine;
		while (getline(File, CurrentLine))
		{
			ShaderData += CurrentLine;
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

	if (Width <= 0 || Height <= 0)
	{
		LOG.Add("Invalid texture dimensions in function FEResourceManager::CreateTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	FETexture* NewTexture = new FETexture(InternalFormat, Format, Width, Height, Name);
	if (!bUnManaged)
		Textures[NewTexture->GetObjectID()] = NewTexture;

	return NewTexture;
}

FEFramebuffer* FEResourceManager::CreateFramebuffer(const int Attachments, const int Width, const int Height, const bool bHDR)
{
	if (Width <= 0 || Height <= 0)
	{
		LOG.Add("Invalid framebuffer dimensions in function FEResourceManager::CreateFramebuffer.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

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

FEFramebuffer* FEResourceManager::CreateFramebuffer()
{
	FEFramebuffer* NewFramebuffer = new FEFramebuffer();
	NewFramebuffer->ColorAttachments.resize(MaxColorAttachments);
	for (size_t i = 0; i < static_cast<size_t>(MaxColorAttachments); i++)
	{
		NewFramebuffer->ColorAttachments[i] = nullptr;
	}

	FE_GL_ERROR(glGenFramebuffers(1, &NewFramebuffer->FBO));
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
	FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, Result->InternalFormat, Result->Width, Result->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, ReferenceRawData);

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
	FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, RawData);
	delete RawData;
	NewTexture->InternalFormat = InternalFormat;

	if (NewTexture->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		// TO-DO: make it configurable.
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
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
	FETexture::GPUAllocateTexture(GL_TEXTURE_2D, 0, InternalFormat, Result->Width, Result->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawData);
	Result->InternalFormat = InternalFormat;

	if (Result->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		// TO-DO: make it configurable.
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
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

void FEResourceManager::SetUserDataVertexAttributeActive(FEMesh* Mesh)
{
	if (Mesh == nullptr)
		return;

	Mesh->VertexAttributes |= FE_VERTEX_ATTRIBUTES::FE_USER_DEFINED_DATA;
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
			FEAssetPackageEntryInitializeData EntryData;
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
			FEAssetPackageEntryInitializeData EntryData;
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
			// FIXME: Currently projects only need debug lib files. Is this correct?
			// The code will grab either Debug or Release lib files. We should make this more deterministic.
			// Note: Only FocalEngine.lib and FEBasicApplication.lib are needed.
			if (AllFiles[i].find("FocalEngine.lib") == std::string::npos && AllFiles[i].find("FEBasicApplication.lib") == std::string::npos)
				continue;

			FEAssetPackageEntryInitializeData EntryData;
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

	// TODO: Check if we need to add more files to the asset package.
	// Currently dumping all related files from the engine folder into the asset package.
	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath + "/Resources/", false);
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].substr(AllFiles[i].size() - 8) == ".texture" ||
			AllFiles[i].substr(AllFiles[i].size() - 6) == ".model" ||
			AllFiles[i].substr(AllFiles[i].size() - 19) == ".nativescriptmodule")
		{
			FEAssetPackageEntryInitializeData EntryData;
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
			FEAssetPackageEntryInitializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(FILE_SYSTEM.GetCurrentWorkingPath().size());
			EntryData.Type = "GLSL";
			EntryData.Tag = ENGINE_RESOURCE_TAG;
			EntryData.Comment = "";

			PrivateEngineAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

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

std::vector<std::string> FEResourceManager::GetPointCloudIDList()
{
	FE_MAP_TO_STR_VECTOR(PointClouds)
}

std::vector<std::string> FEResourceManager::GetEnginePrivatePointCloudIDList()
{
	return GetResourceIDListByTag(PointClouds, ENGINE_RESOURCE_TAG);
}

FEPointCloud* FEResourceManager::GetPointCloud(std::string ID)
{
	if (PointClouds.find(ID) == PointClouds.end())
		return nullptr;

	return PointClouds[ID];
}

std::vector<FEPointCloud*> FEResourceManager::GetPointCloudByName(const std::string Name)
{
	std::vector<FEPointCloud*> Result;
	auto PointCloudIterator = PointClouds.begin();
	while (PointCloudIterator != PointClouds.end())
	{
		if (PointCloudIterator->second->GetName() == Name)
			Result.push_back(PointCloudIterator->second);

		PointCloudIterator++;
	}

	return Result;
}

FEPointCloud* FEResourceManager::RawDataToFEPointCloud(std::vector<FEPointCloudVertexDouble>& RawPointCloudDataDouble, std::string Name, std::string ForceObjectID, bool bCenterPositions, bool bAdvancedRendering, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor)
{
	if (RawPointCloudDataDouble.empty())
	{
		LOG.Add("FEResourceManager::RawDataToFEPointCloud: RawPointCloudDataDouble is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return nullptr;
	}

	FEAABB PointCloudAABB;
	// Before converting to float, we need to center the point cloud using 64 bit precision.
	if (!RawPointCloudDataDouble.empty())
	{
		glm::dvec3 Min = glm::dvec3(DBL_MAX);
		glm::dvec3 Max = glm::dvec3(-DBL_MAX);

		for (size_t i = 0; i < RawPointCloudDataDouble.size(); i++)
		{
			if (RawPointCloudDataDouble[i].X < Min.x)
				Min.x = RawPointCloudDataDouble[i].X;

			if (RawPointCloudDataDouble[i].X > Max.x)
				Max.x = RawPointCloudDataDouble[i].X;

			if (RawPointCloudDataDouble[i].Y < Min.y)
				Min.y = RawPointCloudDataDouble[i].Y;

			if (RawPointCloudDataDouble[i].Y > Max.y)
				Max.y = RawPointCloudDataDouble[i].Y;

			if (RawPointCloudDataDouble[i].Z < Min.z)
				Min.z = RawPointCloudDataDouble[i].Z;

			if (RawPointCloudDataDouble[i].Z > Max.z)
				Max.z = RawPointCloudDataDouble[i].Z;
		}

		glm::dvec3 Extent = Max - Min;
		glm::dvec3 Center = Min + Extent / 2.0;

		if (bCenterPositions)
		{
			for (size_t i = 0; i < RawPointCloudDataDouble.size(); i++)
			{
				RawPointCloudDataDouble[i].X = RawPointCloudDataDouble[i].X - Center.x;
				RawPointCloudDataDouble[i].Y = RawPointCloudDataDouble[i].Y - Center.y;
				RawPointCloudDataDouble[i].Z = RawPointCloudDataDouble[i].Z - Center.z;
			}

			PointCloudAABB = FEAABB(Min - Center, Max - Center);
		}
		else
		{
			PointCloudAABB = FEAABB(Min, Max);
		}
	}

	std::vector<FEPointCloudVertex> RawPointCloudData;
	RawPointCloudData.resize(RawPointCloudDataDouble.size());
	for (size_t i = 0; i < RawPointCloudDataDouble.size(); i++)
	{
		RawPointCloudData[i].X = static_cast<float>(RawPointCloudDataDouble[i].X);
		RawPointCloudData[i].Y = static_cast<float>(RawPointCloudDataDouble[i].Y);
		RawPointCloudData[i].Z = static_cast<float>(RawPointCloudDataDouble[i].Z);
		RawPointCloudData[i].R = RawPointCloudDataDouble[i].R;
		RawPointCloudData[i].G = RawPointCloudDataDouble[i].G;
		RawPointCloudData[i].B = RawPointCloudDataDouble[i].B;
		RawPointCloudData[i].A = RawPointCloudDataDouble[i].A;
	}

	if (UserDataProcessor)
	{
		UserDataProcessor(RawPointCloudData);

		// Points might have been removed or changed in the user data processor.
		glm::dvec3 Min = glm::dvec3(DBL_MAX);
		glm::dvec3 Max = glm::dvec3(-DBL_MAX);

		for (size_t i = 0; i < RawPointCloudData.size(); i++)
		{
			if (RawPointCloudData[i].X < Min.x)
				Min.x = RawPointCloudData[i].X;

			if (RawPointCloudData[i].X > Max.x)
				Max.x = RawPointCloudData[i].X;

			if (RawPointCloudData[i].Y < Min.y)
				Min.y = RawPointCloudData[i].Y;

			if (RawPointCloudData[i].Y > Max.y)
				Max.y = RawPointCloudData[i].Y;

			if (RawPointCloudData[i].Z < Min.z)
				Min.z = RawPointCloudData[i].Z;

			if (RawPointCloudData[i].Z > Max.z)
				Max.z = RawPointCloudData[i].Z;
		}

		PointCloudAABB = FEAABB(Min, Max);
	}

	FEPointCloud* LoadedPointCloud = RawDataToFEPointCloud(RawPointCloudData, Name, ForceObjectID, false, bAdvancedRendering);
	LoadedPointCloud->AABB = PointCloudAABB;
	return LoadedPointCloud;
}

bool FEResourceManager::SetUpPointCloudGPUBuffers(FEPointCloud* PointCloud, std::vector<FEPointCloudVertex>& RawPointCloudData)
{
	if (PointCloud == nullptr)
	{
		LOG.Add("FEResourceManager::SetUpPointCloudGPUBuffers: PointCloud is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (RawPointCloudData.empty())
	{
		LOG.Add("FEResourceManager::SetUpPointCloudGPUBuffers: RawPointCloudData is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (RawPointCloudData.size() >= FEPointCloud::MaxPointsPerBuffer)
	{
		LOG.Add("FEResourceManager::SetUpPointCloudGPUBuffers: Point cloud has too many points for rendering. Forcing advanced rendering instead.", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		PointCloud->bUseAdvancedRendering = true;
	}

	if (PointCloud->VaoID != GLuint(-1))
	{
		FE_GL_ERROR(glDeleteVertexArrays(1, &PointCloud->VaoID));
		PointCloud->VaoID = GLuint(-1);
	}

	if (PointCloud->VboID != GLuint(-1))
	{
		FE_GL_ERROR(glDeleteBuffers(1, &PointCloud->VboID));
		PointCloud->VboID = GLuint(-1);
	}

	if (PointCloud->ComputeShaderBuffer != GLuint(-1))
	{
		FE_GL_ERROR(glDeleteBuffers(1, &PointCloud->ComputeShaderBuffer));
		PointCloud->ComputeShaderBuffer = GLuint(-1);
	}

	if (!PointCloud->IsAdvancedRenderingEnabled())
	{
		FE_GL_ERROR(glGenBuffers(1, &PointCloud->VboID));

		// Bind and upload vertex data to the VBO.
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->VboID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(FEPointCloudVertex) * PointCloud->PointCount, RawPointCloudData.data(), GL_STATIC_DRAW));

		FE_GL_ERROR(glGenVertexArrays(1, &PointCloud->VaoID));

		// Bind and link VAO and VBO.
		FE_GL_ERROR(glBindVertexArray(PointCloud->VaoID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->VboID));

		FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FEPointCloudVertex), (void*)0));
		FE_GL_ERROR(glEnableVertexAttribArray(0));

		FE_GL_ERROR(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(FEPointCloudVertex), (void*)(3 * sizeof(float))));
		FE_GL_ERROR(glEnableVertexAttribArray(1));
	}
	else
	{
		FE_GL_ERROR(glGenBuffers(1, &PointCloud->ComputeShaderBuffer));
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointCloud->ComputeShaderBuffer));
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointCloud->ComputeShaderBuffer));
		// If we have more points than the maximum points per buffer, we will split the data into multiple buffers.
		if (PointCloud->PointCount > FEPointCloud::MaxPointsPerBuffer)
		{
			FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FEPointCloudVertex) * FEPointCloud::MaxPointsPerBuffer, RawPointCloudData.data(), GL_DYNAMIC_DRAW));

			for (size_t i = FEPointCloud::MaxPointsPerBuffer; i < PointCloud->PointCount; i += FEPointCloud::MaxPointsPerBuffer)
			{
				PointCloud->ComputeShaderBuffers.resize(PointCloud->ComputeShaderBuffers.size() + 1);
				FE_GL_ERROR(glGenBuffers(1, &PointCloud->ComputeShaderBuffers.back()));
				FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointCloud->ComputeShaderBuffers.back()));
				FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointCloud->ComputeShaderBuffers.back()));

				// Calculate the number of points for the current buffer
				size_t NumberOfPoints = std::min(FEPointCloud::MaxPointsPerBuffer, RawPointCloudData.size() - i);
				FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FEPointCloudVertex) * NumberOfPoints, RawPointCloudData.data() + i, GL_DYNAMIC_DRAW));
			}
		}
		else
		{
			FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FEPointCloudVertex) * PointCloud->PointCount, RawPointCloudData.data(), GL_DYNAMIC_DRAW));
		}
	}

	PointCloud->PointCount = RawPointCloudData.size();
	return true;
}

FEPointCloud* FEResourceManager::RawDataToFEPointCloud(std::vector<FEPointCloudVertex>& RawPointCloudData, std::string Name, std::string ForceObjectID, bool bCenterPositions, bool bAdvancedRendering, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor)
{
	FEPointCloud* NewPointCloud = new FEPointCloud();
	NewPointCloud->SetName(Name);

	if (!ForceObjectID.empty())	
		NewPointCloud->SetID(ForceObjectID);

	if (RawPointCloudData.empty())
	{
		LOG.Add("FEResourceManager::RawDataToFEPointCloud: RawPointCloudData is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return NewPointCloud;
	}

	if (!RawPointCloudData.empty())
	{
		glm::vec3 Min = glm::vec3(FLT_MAX);
		glm::vec3 Max = glm::vec3(-FLT_MAX);

		for (size_t i = 0; i < RawPointCloudData.size(); i++)
		{
			if (RawPointCloudData[i].X < Min.x)
				Min.x = RawPointCloudData[i].X;

			if (RawPointCloudData[i].X > Max.x)
				Max.x = RawPointCloudData[i].X;

			if (RawPointCloudData[i].Y < Min.y)
				Min.y = RawPointCloudData[i].Y;

			if (RawPointCloudData[i].Y > Max.y)
				Max.y = RawPointCloudData[i].Y;

			if (RawPointCloudData[i].Z < Min.z)
				Min.z = RawPointCloudData[i].Z;

			if (RawPointCloudData[i].Z > Max.z)
				Max.z = RawPointCloudData[i].Z;
		}

		NewPointCloud->AABB = FEAABB(Min, Max);
		if (bCenterPositions)
		{
			glm::vec3 Extent = Max - Min;
			glm::vec3 Center = Min + Extent / 2.0f;

			for (size_t i = 0; i < RawPointCloudData.size(); i++)
			{
				RawPointCloudData[i].X = RawPointCloudData[i].X - Center.x;
				RawPointCloudData[i].Y = RawPointCloudData[i].Y - Center.y;
				RawPointCloudData[i].Z = RawPointCloudData[i].Z - Center.z;
			}

			NewPointCloud->AABB = FEAABB(Min - Center, Max - Center);
		}
	}

	if (UserDataProcessor)
		UserDataProcessor(RawPointCloudData);

	PointClouds[NewPointCloud->GetObjectID()] = NewPointCloud;
	NewPointCloud->PointCount = RawPointCloudData.size();
	NewPointCloud->bUseAdvancedRendering = bAdvancedRendering;

	if (!SetUpPointCloudGPUBuffers(NewPointCloud, RawPointCloudData))
	{
		LOG.Add("FEResourceManager::RawDataToFEPointCloud: Failed to set up GPU buffers for point cloud.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		delete NewPointCloud;
		return nullptr;
	}

	return NewPointCloud;
}

FEPointCloud* FEResourceManager::RawPLYDataToFEPointCloud(FERawPLYData* PLYData, std::string Name, std::string ForceObjectID, bool bCenterPositions, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor)
{
	FEPointCloud* LoadedPointCloud = nullptr;

	if (PLYData == nullptr)
	{
		LOG.Add("FEResourceManager::RawPLYDataToFEPointCloud: PLYData is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	bool bIsPointCloud = DoesPLYContainPointCloud(PLYData);
	if (!bIsPointCloud)
	{
		LOG.Add("FEResourceManager::RawPLYDataToFEPointCloud: PLY file does not contain point cloud data.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	std::vector<FEPointCloudVertex> Vertices;
	std::variant<std::vector<glm::vec3>, std::vector<glm::dvec3>> Positions = ExtractPositionsFromPLYData(PLYData);
	if (auto* FloatPositionVector = std::get_if<std::vector<glm::vec3>>(&Positions))
	{
		if (FloatPositionVector->empty())
		{
			LOG.Add("FEResourceManager::RawPLYDataToFEPointCloud: Error extracting positions from PLY data.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return LoadedPointCloud;
		}

		Vertices.resize(FloatPositionVector->size());
		for (size_t i = 0; i < FloatPositionVector->size(); i++)
		{
			Vertices[i].X = FloatPositionVector->at(i).x;
			Vertices[i].Y = FloatPositionVector->at(i).y;
			Vertices[i].Z = FloatPositionVector->at(i).z;
		}

		FloatPositionVector->clear();
	}
	else if (auto* DoublePositionVector = std::get_if<std::vector<glm::dvec3>>(&Positions))
	{
		if (DoublePositionVector->empty())
		{
			LOG.Add("FEResourceManager::RawPLYDataToFEPointCloud: Error extracting positions from PLY data.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return LoadedPointCloud;
		}

		// Right now we are not supporting double precision positions.
		// So we will convert them to float.
		Vertices.resize(DoublePositionVector->size());
		for (size_t i = 0; i < DoublePositionVector->size(); i++)
		{
			Vertices[i].X = static_cast<float>(DoublePositionVector->at(i).x);
			Vertices[i].Y = static_cast<float>(DoublePositionVector->at(i).y);
			Vertices[i].Z = static_cast<float>(DoublePositionVector->at(i).z);
		}

		DoublePositionVector->clear();
	}
	else
	{
		LOG.Add("FEResourceManager::RawPLYDataToFEPointCloud: Error extracting positions from PLY data.", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	std::vector<std::vector<unsigned char>> Colors = ExtractColorsFromPLYData(PLYData);
	if (!Colors.empty() && Vertices.size() == Colors.size())
	{
		for (size_t i = 0; i < Colors.size(); i++)
		{
			if (Colors[i].size() == 4)
			{
				Vertices[i].R = Colors[i][0];
				Vertices[i].G = Colors[i][1];
				Vertices[i].B = Colors[i][2];
				Vertices[i].A = Colors[i][3];
			}
		}

		Colors.clear();
	}

	LoadedPointCloud = RawDataToFEPointCloud(Vertices, Name, ForceObjectID, bCenterPositions, false, UserDataProcessor);
	return LoadedPointCloud;
}

FEPointCloud* FEResourceManager::LasOrLazToFEPointCloud(std::string FilePath, std::string Name, std::string ForceObjectID, bool bCenterPositions, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor, laszip_header* OutHeaderCopy)
{
	FEPointCloud* LoadedPointCloud = nullptr;
	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: FilePath is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return LoadedPointCloud;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: File does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return LoadedPointCloud;
	}

	if (!bIsLasLazFilesEnabled)
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: LAS/LAZ files are not enabled", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return LoadedPointCloud;
	}

	laszip_POINTER LaszipReader;
	laszip_I32 Error = laszip_create(&LaszipReader);
	if (Error)
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: Creating laszip reader failed with error: " + std::to_string(Error), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	laszip_BOOL bIsCompressed = 0;
	bool bIsLASOrLAZFile = !laszip_open_reader(LaszipReader, FilePath.c_str(), &bIsCompressed);
	if (!bIsLASOrLAZFile)
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: File is not a valid LAS/LAZ file", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		if (laszip_destroy(LaszipReader))
		{
			LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: destroying laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		}
		return LoadedPointCloud;
	}

	laszip_header* FileHeader;
	if (laszip_get_header_pointer(LaszipReader, &FileHeader))
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: getting header pointer from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	if (OutHeaderCopy != nullptr)
		std::memcpy(OutHeaderCopy, FileHeader, sizeof(laszip_header));

	laszip_point* CurrentPointPointer;
	if (laszip_get_point_pointer(LaszipReader, &CurrentPointPointer))
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: getting point pointer from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	laszip_U64 PointCount = (FileHeader->number_of_point_records ? FileHeader->number_of_point_records : FileHeader->extended_number_of_point_records);
	if (PointCount == 0)
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: Point count is zero", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	std::vector<FEPointCloudVertexDouble> RawDataDouble;
	RawDataDouble.resize(PointCount);

	laszip_U64 PointIndex = 0;
	while (PointIndex < PointCount)
	{
		if (laszip_read_point(LaszipReader))
		{
			LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: reading point from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return LoadedPointCloud;
		}

		RawDataDouble[PointIndex].X = CurrentPointPointer->X * FileHeader->x_scale_factor;
		RawDataDouble[PointIndex].Y = CurrentPointPointer->Y * FileHeader->y_scale_factor;
		RawDataDouble[PointIndex].Z = CurrentPointPointer->Z * FileHeader->z_scale_factor;

		RawDataDouble[PointIndex].R = unsigned char(CurrentPointPointer->rgb[0] / float(1 << 16) * 255);
		RawDataDouble[PointIndex].G = unsigned char(CurrentPointPointer->rgb[1] / float(1 << 16) * 255);
		RawDataDouble[PointIndex].B = unsigned char(CurrentPointPointer->rgb[2] / float(1 << 16) * 255);

		PointIndex++;
	}

	if (laszip_close_reader(LaszipReader))
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: closing laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
	}

	if (laszip_destroy(LaszipReader))
	{
		LOG.Add("FEResourceManager::LasOrLazToFEPointCloud: destroying laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
	}

	return RawDataToFEPointCloud(RawDataDouble, Name, ForceObjectID, bCenterPositions, false, UserDataProcessor);
}

bool FEResourceManager::ReadLasOrLaz(std::string FilePath, std::vector<FEPointCloudVertexDouble>& RawData, laszip_header* OutHeaderCopy)
{
	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: FilePath is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: File does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!bIsLasLazFilesEnabled)
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: LAS/LAZ files are not enabled", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	laszip_POINTER LaszipReader;
	laszip_I32 Error = laszip_create(&LaszipReader);
	if (Error)
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: Creating laszip reader failed with error: " + std::to_string(Error), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	laszip_BOOL bIsCompressed = 0;
	bool bIsLASOrLAZFile = !laszip_open_reader(LaszipReader, FilePath.c_str(), &bIsCompressed);
	if (!bIsLASOrLAZFile)
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: File is not a valid LAS/LAZ file", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		if (laszip_destroy(LaszipReader))
		{
			LOG.Add("FEResourceManager::ReadLasOrLaz: destroying laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		}
		return false;
	}

	laszip_header* FileHeader;
	if (laszip_get_header_pointer(LaszipReader, &FileHeader))
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: getting header pointer from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (OutHeaderCopy != nullptr)
		std::memcpy(OutHeaderCopy, FileHeader, sizeof(laszip_header));

	laszip_point* CurrentPointPointer;
	if (laszip_get_point_pointer(LaszipReader, &CurrentPointPointer))
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: getting point pointer from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	laszip_U64 PointCount = (FileHeader->number_of_point_records ? FileHeader->number_of_point_records : FileHeader->extended_number_of_point_records);
	if (PointCount == 0)
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: Point count is zero", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	//std::vector<FEPointCloudVertexDouble> RawDataDouble;
	RawData.resize(PointCount);

	laszip_U64 PointIndex = 0;
	while (PointIndex < PointCount)
	{
		if (laszip_read_point(LaszipReader))
		{
			LOG.Add("FEResourceManager::ReadLasOrLaz: reading point from laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}

		RawData[PointIndex].X = CurrentPointPointer->X * FileHeader->x_scale_factor;
		RawData[PointIndex].Y = CurrentPointPointer->Y * FileHeader->y_scale_factor;
		RawData[PointIndex].Z = CurrentPointPointer->Z * FileHeader->z_scale_factor;

		RawData[PointIndex].R = unsigned char(CurrentPointPointer->rgb[0] / float(1 << 16) * 255);
		RawData[PointIndex].G = unsigned char(CurrentPointPointer->rgb[1] / float(1 << 16) * 255);
		RawData[PointIndex].B = unsigned char(CurrentPointPointer->rgb[2] / float(1 << 16) * 255);

		PointIndex++;
	}

	if (laszip_close_reader(LaszipReader))
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: closing laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
	}

	if (laszip_destroy(LaszipReader))
	{
		LOG.Add("FEResourceManager::ReadLasOrLaz: destroying laszip reader failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
	}

	return true;
}

FEPointCloud* FEResourceManager::ImportPointCloud(std::string FilePath, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor)
{
	FEPointCloud* LoadedPointCloud = nullptr;
	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ImportPointCloud: FileName is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return LoadedPointCloud;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEResourceManager::ImportPointCloud: File does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return LoadedPointCloud;
	}

	bool bIsPLYFile = PLY_MANAGER.IsPLYFile(FilePath);
	if (bIsPLYFile)
	{
		FERawPLYData* PLYData = PLY_MANAGER.ParseFile(FilePath);
		LoadedPointCloud = RawPLYDataToFEPointCloud(PLYData, FILE_SYSTEM.GetFileName(FilePath), "", true, UserDataProcessor);
	}
	else
	{
		LoadedPointCloud = LasOrLazToFEPointCloud(FilePath, FILE_SYSTEM.GetFileName(FilePath), "", true, UserDataProcessor);
	}

	if (LoadedPointCloud == nullptr)
	{
		LOG.Add("FEResourceManager::ImportPointCloud: Error creating point cloud from file: " + FilePath, "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedPointCloud;
	}

	return LoadedPointCloud;
}

struct LoadPointCloudAsyncInfo
{
	std::string FilePath;
	std::function<void(FEPointCloud*)> UserCallBack;
	std::function<void(std::vector<FEPointCloudVertexDouble>&)> UserDataProcessor;
	std::vector<FEPointCloudVertex> RawData;
	bool bSuccess = false;
	bool bCenterPositions = true;
	FEAABB AABB;
	laszip_header* OutHeaderCopy = nullptr;
};

void LoadPointCloudFileAsync(void* InputData, void* OutputData)
{
	auto* Input = reinterpret_cast<LoadPointCloudAsyncInfo*>(InputData);
	auto* Output = reinterpret_cast<LoadPointCloudAsyncInfo*>(OutputData);

	std::vector<FEPointCloudVertexDouble> TempRawData;
	Output->bSuccess = RESOURCE_MANAGER.ReadLasOrLaz(Input->FilePath, TempRawData, Input->OutHeaderCopy);

	if (Output->bSuccess)
	{
		FEAABB PointCloudAABB;
		// Before converting to float, we need to center the point cloud using 64 bit precision.
		if (!TempRawData.empty())
		{
			glm::dvec3 Min = glm::dvec3(DBL_MAX);
			glm::dvec3 Max = glm::dvec3(-DBL_MAX);

			for (size_t i = 0; i < TempRawData.size(); i++)
			{
				if (TempRawData[i].X < Min.x)
					Min.x = TempRawData[i].X;

				if (TempRawData[i].X > Max.x)
					Max.x = TempRawData[i].X;

				if (TempRawData[i].Y < Min.y)
					Min.y = TempRawData[i].Y;

				if (TempRawData[i].Y > Max.y)
					Max.y = TempRawData[i].Y;

				if (TempRawData[i].Z < Min.z)
					Min.z = TempRawData[i].Z;

				if (TempRawData[i].Z > Max.z)
					Max.z = TempRawData[i].Z;
			}

			glm::dvec3 Extent = Max - Min;
			glm::dvec3 Center = Min + Extent / 2.0;

			if (Input->bCenterPositions)
			{
				for (size_t i = 0; i < TempRawData.size(); i++)
				{
					TempRawData[i].X = TempRawData[i].X - Center.x;
					TempRawData[i].Y = TempRawData[i].Y - Center.y;
					TempRawData[i].Z = TempRawData[i].Z - Center.z;
				}

				PointCloudAABB = FEAABB(Min - Center, Max - Center);
			}
			else
			{
				PointCloudAABB = FEAABB(Min, Max);
			}
		}

		if (Input->UserDataProcessor)
		{
			Input->UserDataProcessor(TempRawData);

			// Points might have been removed or changed in the user data processor.
			glm::dvec3 Min = glm::dvec3(DBL_MAX);
			glm::dvec3 Max = glm::dvec3(-DBL_MAX);

			for (size_t i = 0; i < TempRawData.size(); i++)
			{
				if (TempRawData[i].X < Min.x)
					Min.x = TempRawData[i].X;

				if (TempRawData[i].X > Max.x)
					Max.x = TempRawData[i].X;

				if (TempRawData[i].Y < Min.y)
					Min.y = TempRawData[i].Y;

				if (TempRawData[i].Y > Max.y)
					Max.y = TempRawData[i].Y;

				if (TempRawData[i].Z < Min.z)
					Min.z = TempRawData[i].Z;

				if (TempRawData[i].Z > Max.z)
					Max.z = TempRawData[i].Z;
			}

			PointCloudAABB = FEAABB(Min, Max);
		}

		Output->RawData.resize(TempRawData.size());
		for (size_t i = 0; i < TempRawData.size(); i++)
		{
			Output->RawData[i].X = static_cast<float>(TempRawData[i].X);
			Output->RawData[i].Y = static_cast<float>(TempRawData[i].Y);
			Output->RawData[i].Z = static_cast<float>(TempRawData[i].Z);
			Output->RawData[i].R = TempRawData[i].R;
			Output->RawData[i].G = TempRawData[i].G;
			Output->RawData[i].B = TempRawData[i].B;
			Output->RawData[i].A = TempRawData[i].A;
		}
		TempRawData.clear();

		Output->AABB = PointCloudAABB;
	}

	Output->UserCallBack = Input->UserCallBack;
	delete Input;
}

void FEResourceManager::LoadPointCloudFileAsyncCallBack(void* OutputData)
{
	FEPointCloud* LoadedPointCloud = nullptr;
	auto* ResultInfo = reinterpret_cast<LoadPointCloudAsyncInfo*>(OutputData);

	if (ResultInfo->bSuccess)
	{
		LoadedPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(ResultInfo->RawData, FILE_SYSTEM.GetFileName(ResultInfo->FilePath), "", false, false, nullptr);
		if (LoadedPointCloud)
			LoadedPointCloud->AABB = ResultInfo->AABB;
	}

	if (ResultInfo->UserCallBack)
		ResultInfo->UserCallBack(LoadedPointCloud);

	delete ResultInfo;
}

void FEResourceManager::ImportLasOrLazPointCloudAsync(std::string FilePath, std::function<void(FEPointCloud*)> CallBack, bool bCenterPositions, std::function<void(std::vector<FEPointCloudVertexDouble>& RawData)> UserDataProcessor, laszip_header* OutHeaderCopy)
{
	FEPointCloud* LoadedPointCloud = nullptr;
	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ImportLasOrLazPointCloudAsync: FileName is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEResourceManager::ImportLasOrLazPointCloudAsync: File does not exist", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	LoadPointCloudAsyncInfo* InputData = new LoadPointCloudAsyncInfo();
	InputData->FilePath = FilePath;
	InputData->UserCallBack = CallBack;
	InputData->UserDataProcessor = UserDataProcessor;
	InputData->bCenterPositions = bCenterPositions;
	InputData->OutHeaderCopy = OutHeaderCopy;

	LoadPointCloudAsyncInfo* OutputData = new LoadPointCloudAsyncInfo();
	THREAD_POOL.Execute(LoadPointCloudFileAsync, InputData, OutputData, &LoadPointCloudFileAsyncCallBack);
}

FEObject* FEResourceManager::ImportPLYFile(std::string FileName)
{
	FEObject* LoadedObject = nullptr;

	FERawPLYData* PLYData = PLY_MANAGER.ParseFile(FileName);
	if (PLYData == nullptr)
	{
		LOG.Add("FEResourceManager::ImportPLYFile: Error parsing PLY file", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return LoadedObject;
	}

	FEPointCloud* LoadedPointCloud = RawPLYDataToFEPointCloud(PLYData, FILE_SYSTEM.GetFileName(FileName));
	if (LoadedPointCloud != nullptr)
		LoadedObject = LoadedPointCloud;
	
	if (LoadedObject == nullptr)
	{
		FEMesh* LoadedMesh = RawPLYDataToFEMesh(PLYData, FILE_SYSTEM.GetFileName(FileName));
		LoadedObject = LoadedMesh;
	}
	else
	{
		LOG.Add("FEResourceManager::ImportPLYFile: Error creating object from PLY file", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
	}

	return LoadedObject;
}

FEPointCloud* FEResourceManager::LoadFEPointCloud(std::string FilePath, std::string Name)
{
	std::fstream File;

	File.open(FilePath, std::ios::in | std::ios::binary);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("can't load file: ") + FilePath + " in function FEResourceManager::LoadFEPointCloud.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	char* Buffer = new char[4];

	// Version of FEPointCloud File type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;

	std::string LoadedObjectID;
	std::string LoadedName;
	if (Version != FE_POINT_CLOUD_VERSION)
	{
		LOG.Add(std::string("can't load file: ") + FilePath + " in function FEResourceManager::LoadFEPointCloud. File was created in different version of engine!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	FEObjectLoadedData ObjectData = OBJECT_MANAGER.LoadFEObjectPart(File);
	LoadedObjectID = ObjectData.ID;
	LoadedName = ObjectData.Name;

	char* Buffer_8Byte = new char[8];
	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t VertexCout = *(size_t*)Buffer_8Byte;
	char* VertexBuffer = new char[VertexCout * sizeof(float)];
	File.read(VertexBuffer, VertexCout * sizeof(float));

	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t ColorCout = *(size_t*)Buffer_8Byte;
	char* ColorBuffer = new char[ColorCout * sizeof(unsigned char)];
	File.read(ColorBuffer, ColorCout * sizeof(unsigned char));

	std::vector<FEPointCloudVertex> PointCloudData;
	for (size_t i = 0; i < VertexCout / 3; i++)
	{
		PointCloudData.push_back(FEPointCloudVertex());
		PointCloudData[i].X = *(float*)(VertexBuffer + i * 3 * sizeof(float));
		PointCloudData[i].Y = *(float*)(VertexBuffer + i * 3 * sizeof(float) + sizeof(float));
		PointCloudData[i].Z = *(float*)(VertexBuffer + i * 3 * sizeof(float) + sizeof(float) * 2);

		PointCloudData[i].R = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char));
		PointCloudData[i].G = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char));
		PointCloudData[i].B = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 2);
		PointCloudData[i].A = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 3);
	}

	char BoolBuffer[sizeof(bool)];
	File.read(BoolBuffer, sizeof(bool));
	bool bAdvancedRendering = *reinterpret_cast<bool*>(BoolBuffer);
	
	FEAABB PointCloudAABB;
	for (int i = 0; i <= 2; i++)
	{
		File.read(Buffer, 4);
		PointCloudAABB.Min[i] = *(float*)Buffer;
	}

	for (int i = 0; i <= 2; i++)
	{
		File.read(Buffer, 4);
		PointCloudAABB.Max[i] = *(float*)Buffer;
	}

	File.close();

	FEPointCloud* NewPointCloud = RawDataToFEPointCloud(PointCloudData, Name, LoadedObjectID, false, bAdvancedRendering);

	delete[] Buffer;
	delete[] Buffer_8Byte;
	delete[] VertexBuffer;
	delete[] ColorBuffer;
	
	NewPointCloud->AABB = PointCloudAABB;
	NewPointCloud->SetName(Name);
	NewPointCloud->Tag = ObjectData.Tag;

	return NewPointCloud;
}

void FEResourceManager::SaveFEPointCloud(FEPointCloud* PointCloud, std::string FilePath)
{
	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);

	// Version of FEPointCloud File type.
	float Version = FE_POINT_CLOUD_VERSION;
	File.write((char*)&Version, sizeof(float));

	OBJECT_MANAGER.SaveFEObjectPart(File, PointCloud);

	std::vector<FEPointCloudVertex> Data = PointCloud->GetRawData();

	size_t Count = PointCloud->GetPointCount() * 3;
	float* Positions = new float[Count];
	for (size_t i = 0; i < PointCloud->GetPointCount(); i++)
	{
		Positions[i * 3] = Data[i].X;
		Positions[i * 3 + 1] = Data[i].Y;
		Positions[i * 3 + 2] = Data[i].Z;
	}
	
	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Positions, sizeof(float) * Count);

	Count = PointCloud->GetPointCount() * 4;
	unsigned char* Colors = new unsigned char[PointCloud->GetPointCount() * 4];
	for (size_t i = 0; i < PointCloud->GetPointCount(); i++)
	{
		Colors[i * 4] = Data[i].R;
		Colors[i * 4 + 1] = Data[i].G;
		Colors[i * 4 + 2] = Data[i].B;
		Colors[i * 4 + 3] = Data[i].A;
	}

	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Colors, sizeof(unsigned char) * Count);

	bool bAdvancedRendering = PointCloud->IsAdvancedRenderingEnabled();
	File.write((char*)&bAdvancedRendering, sizeof(bool));

	File.write((char*)&PointCloud->AABB.Min[0], sizeof(float));
	File.write((char*)&PointCloud->AABB.Min[1], sizeof(float));
	File.write((char*)&PointCloud->AABB.Min[2], sizeof(float));

	File.write((char*)&PointCloud->AABB.Max[0], sizeof(float));
	File.write((char*)&PointCloud->AABB.Max[1], sizeof(float));
	File.write((char*)&PointCloud->AABB.Max[2], sizeof(float));

	File.close();

	delete[] Positions;
}

bool FEResourceManager::DoesPLYContainMesh(FERawPLYData* PLYData)
{
	if (PLYData == nullptr)
	{
		LOG.Add("FEResourceManager::DoesPLYContainMesh: PLYData is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (PLYData->Header == nullptr)
	{
		LOG.Add("FEResourceManager::DoesPLYContainMesh: PLYData->Header is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (PLYData->Header->ElementSchemas.empty())
	{
		LOG.Add("FEResourceManager::DoesPLYContainMesh: PLYData->Header->Elements is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	bool bHasVertex = false;
	for (size_t i = 0; i < PLYData->Header->ElementSchemas.size(); i++)
	{
		if (PLYData->Header->ElementSchemas[i].Name == "vertex")
		{
			if (PLYData->Header->ElementSchemas[i].PropertyDefinitions.size() < 3)
				break;
			
			bool bHasX = false;
			bool bHasY = false;
			bool bHasZ = false;

			for (size_t j = 0; j < PLYData->Header->ElementSchemas[i].PropertyDefinitions.size(); j++)
			{
				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "x")
					bHasX = true;

				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "y")
					bHasY = true;

				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "z")
					bHasZ = true;
			}

			bHasVertex = bHasX && bHasY && bHasZ;
		}
	}

	bool bHasFace = false;
	for (size_t i = 0; i < PLYData->Header->ElementSchemas.size(); i++)
	{
		if (PLYData->Header->ElementSchemas[i].Name == "face")
		{
			if (PLYData->Header->ElementSchemas[i].PropertyDefinitions.size() == 0)
				break;

			for (size_t j = 0; j < PLYData->Header->ElementSchemas[i].PropertyDefinitions.size(); j++)
			{
				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "vertex_indices" ||
					PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "vertex_index" ||
					PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "indices")
				{
					bHasFace = true;
				}
			}
		}
	}
	
	return bHasFace && bHasVertex;
}

bool FEResourceManager::DoesPLYContainPointCloud(FERawPLYData* PLYData)
{
	if (PLYData == nullptr)
	{
		LOG.Add("FEResourceManager::DoesPLYContainPointCloud: PLYData is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (PLYData->Header == nullptr)
	{
		LOG.Add("FEResourceManager::DoesPLYContainPointCloud: PLYData->Header is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (PLYData->Header->ElementSchemas.empty())
	{
		LOG.Add("FEResourceManager::DoesPLYContainPointCloud: PLYData->Header->Elements is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (DoesPLYContainMesh(PLYData))
		return false;
	
	bool bHasVertex = false;
	for (size_t i = 0; i < PLYData->Header->ElementSchemas.size(); i++)
	{
		if (PLYData->Header->ElementSchemas[i].Name == "vertex")
		{
			if (PLYData->Header->ElementSchemas[i].PropertyDefinitions.size() < 3)
				break;

			bool bHasX = false;
			bool bHasY = false;
			bool bHasZ = false;

			for (size_t j = 0; j < PLYData->Header->ElementSchemas[i].PropertyDefinitions.size(); j++)
			{
				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "x")
					bHasX = true;

				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "y")
					bHasY = true;

				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "z")
					bHasZ = true;
			}

			bHasVertex = bHasX && bHasY && bHasZ;
		}
	}

	bool bHasFace = false;
	for (size_t i = 0; i < PLYData->Header->ElementSchemas.size(); i++)
	{
		if (PLYData->Header->ElementSchemas[i].Name == "face")
		{
			if (PLYData->Header->ElementSchemas[i].PropertyDefinitions.size() == 0)
				break;

			for (size_t j = 0; j < PLYData->Header->ElementSchemas[i].PropertyDefinitions.size(); j++)
			{
				if (PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "vertex_indices" ||
					PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "vertex_index" ||
					PLYData->Header->ElementSchemas[i].PropertyDefinitions[j].Name == "indices")
				{
					bHasFace = true;
				}
			}
		}
	}

	return !bHasFace && bHasVertex;
}

void FEResourceManager::DeleteFEPointCloud(FEPointCloud* PointCloud)
{
	if (PointCloud == nullptr)
	{
		LOG.Add("FEResourceManager::DeleteFEPointCloud: PointCloud is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	if (PointClouds.find(PointCloud->GetObjectID()) == PointClouds.end())
	{
		LOG.Add("FEResourceManager::DeleteFEPointCloud: PointCloud does not exist in the resource manager", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	PointClouds.erase(PointCloud->GetObjectID());
	delete PointCloud;
}

bool FEResourceManager::SaveRawDataToPLY(std::vector<FEPointCloudVertex>& RawData, std::string FilePath)
{
	bool bResult = false;

	if (RawData.empty())
	{
		LOG.Add("FEResourceManager::SaveRawDataToPLY: RawData is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return bResult;
	}

	FERawPLYData* PLYData = new FERawPLYData();
	PLYData->Header = new FEPLYHeader();
	PLYData->Header->StorageType = PLYFileType::BINARY_LITTLE_ENDIAN;
	PLYData->Header->Comments.push_back("Generated by Focal Engine");
	PLYData->Header->ElementSchemas.push_back(PLYElementSchema());
	PLYData->Header->ElementSchemas[0].Name = "vertex";
	PLYData->Header->ElementSchemas[0].Count = RawData.size();

	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[0].Name = "x";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[0].Type = PLYPropertyType::FLOAT;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[1].Name = "y";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[1].Type = PLYPropertyType::FLOAT;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[2].Name = "z";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[2].Type = PLYPropertyType::FLOAT;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());

	PLYData->Header->ElementSchemas[0].PropertyDefinitions[3].Name = "red";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[3].Type = PLYPropertyType::UCHAR;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[4].Name = "green";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[4].Type = PLYPropertyType::UCHAR;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[5].Name = "blue";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[5].Type = PLYPropertyType::UCHAR;
	PLYData->Header->ElementSchemas[0].PropertyDefinitions.push_back(PLYPropertyDefinition());
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[6].Name = "alpha";
	PLYData->Header->ElementSchemas[0].PropertyDefinitions[6].Type = PLYPropertyType::UCHAR;

	PLYData->Elements.push_back(PLYElementData());
	PLYData->Elements[0].Description = PLYData->Header->ElementSchemas[0];
	PLYData->Elements[0].Entries.resize(RawData.size());
	for (size_t i = 0; i < RawData.size(); i++)
	{
		PLYData->Elements[0].Entries[i].PropertyValues.resize(7);

		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[0]) = RawData[i].X;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[1]) = RawData[i].Y;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[2]) = RawData[i].Z;

		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[3]) = RawData[i].R;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[4]) = RawData[i].G;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[5]) = RawData[i].B;
		std::get<PLYScalarValue>(PLYData->Elements[0].Entries[i].PropertyValues[6]) = RawData[i].A;
	}

	bResult = PLY_MANAGER.SaveToPLY(FilePath, PLYData, PLYFileType::BINARY_LITTLE_ENDIAN);
	return bResult;
}

bool FEResourceManager::ExportFEPointCloudToPLY(FEPointCloud* PointCloudToExport, std::string FilePath)
{
	bool bResult = false;

	if (PointCloudToExport == nullptr)
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToPLY: PointCloudToExport is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return bResult;
	}

	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToPLY: FilePath is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return bResult;
	}

	std::vector<FEPointCloudVertex> PointCloudData;
	PointCloudData = PointCloudToExport->GetRawData();

	return SaveRawDataToPLY(PointCloudData, FilePath);
}

bool FEResourceManager::SaveRawDataToLASOrLAZ(std::vector<FEPointCloudVertex>& RawData, std::string FilePath, bool bIsCompressed, double ScaleFactor)
{
	if (RawData.empty())
	{
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: RawData is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	laszip_POINTER LaszipWriter;
	if (laszip_create(&LaszipWriter))
	{
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Creating laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	laszip_header* FileHeader;
	if (laszip_get_header_pointer(LaszipWriter, &FileHeader))
	{
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Getting header for a file failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	// Indicate that we have RGB color data in our point format
	FileHeader->point_data_format = 2;  // Format 2 includes RGB values

	// Update the point data record length to account for RGB data
	// Standard record length is 20 bytes, plus 6 bytes for RGB (2 bytes per color channel)
	FileHeader->point_data_record_length = 26;

	// Set appropriate scale factors for data
	// The LAS format stores coordinates as integers internally,
	// but these integers represent real-world coordinates (typically in meters)
	// Scale factors convert between floating-point and integer representations:
	//   integer_value = real_world_value / scale_factor
	//   real_world_value = integer_value * scale_factor
	// 
	// For example, with scale_factor = 0.001:
	// - A real-world coordinate of 10.523 meters becomes integer 10523
	// - An integer value of 8421 represents 8.421 meters in real-world space
	//
	FileHeader->x_scale_factor = ScaleFactor;
	FileHeader->y_scale_factor = ScaleFactor;
	FileHeader->z_scale_factor = ScaleFactor;

	FileHeader->number_of_point_records = static_cast<laszip_U32>(RawData.size());

	if (laszip_open_writer(LaszipWriter, FilePath.c_str(), bIsCompressed))
	{
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Opening laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

		laszip_CHAR* Error;
		if (laszip_get_error(LaszipWriter, &Error))
			LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Getting laszip error failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: " + std::string(Error), "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
		return false;
	}

	laszip_point_struct* CurrentPoint = nullptr;
	CurrentPoint = new laszip_point_struct();
	for (size_t i = 0; i < RawData.size(); i++)
	{
		CurrentPoint->X = static_cast<laszip_I32>(static_cast<double>(RawData[i].X) / FileHeader->x_scale_factor);
		CurrentPoint->Y = static_cast<laszip_I32>(static_cast<double>(RawData[i].Y) / FileHeader->y_scale_factor);
		CurrentPoint->Z = static_cast<laszip_I32>(static_cast<double>(RawData[i].Z) / FileHeader->z_scale_factor);

		// Convert 8-bit colors (0-255) to 16-bit colors (0-65535)
		int CoefficientToConvertTo16Bit = UINT16_MAX / UINT8_MAX;
		CurrentPoint->rgb[0] = static_cast<laszip_U16>(RawData[i].R * CoefficientToConvertTo16Bit);
		CurrentPoint->rgb[1] = static_cast<laszip_U16>(RawData[i].G * CoefficientToConvertTo16Bit);
		CurrentPoint->rgb[2] = static_cast<laszip_U16>(RawData[i].B * CoefficientToConvertTo16Bit);

		if (laszip_set_point(LaszipWriter, CurrentPoint))
		{
			LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Setting point with index " + std::to_string(i) + " failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			delete CurrentPoint;

			if (laszip_close_writer(LaszipWriter))
				LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Closing laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

			if (laszip_destroy(LaszipWriter))
				LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

			return false;
		}

		if (laszip_write_point(LaszipWriter))
		{
			LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Writing point with index " + std::to_string(i) + " failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	if (laszip_close_writer(LaszipWriter))
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Closing laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

	if (laszip_destroy(LaszipWriter))
		LOG.Add("FEResourceManager::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_RESOURCE_MANAGER", FE_LOG_ERROR);

	delete CurrentPoint;
	return true;
}

bool FEResourceManager::ExportFEPointCloudToLAS(FEPointCloud* PointCloudToExport, std::string FilePath)
{
	if (PointCloudToExport == nullptr)
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAS: PointCloudToExport is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAS: FilePath is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!bIsLasLazFilesEnabled)
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAS: LAS/LAZ files are not enabled", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEPointCloudVertex> PointCloudData = PointCloudToExport->GetRawData();
	return SaveRawDataToLASOrLAZ(PointCloudData, FilePath, false);
}

bool FEResourceManager::ExportFEPointCloudToLAZ(FEPointCloud* PointCloudToExport, std::string FilePath)
{
	if (PointCloudToExport == nullptr)
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAZ: PointCloudToExport is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (FilePath.empty())
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAZ: FilePath is empty", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!bIsLasLazFilesEnabled)
	{
		LOG.Add("FEResourceManager::ExportFEPointCloudToLAZ: LAS/LAZ files are not enabled", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEPointCloudVertex> PointCloudData = PointCloudToExport->GetRawData();
	return SaveRawDataToLASOrLAZ(PointCloudData, FilePath, true);
}

std::variant<std::vector<glm::vec3>, std::vector<glm::dvec3>> FEResourceManager::ExtractPositionsFromPLYData(FERawPLYData* PLYData)
{
	std::variant<std::vector<glm::vec3>, std::vector<glm::dvec3>> Positions;

	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Positions;
	}

	std::string ElementName = "vertex";

	bool bHaveFloatPositions = false;
	bool bHaveDoublePositions = false;
	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == ElementName)
		{
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if (Property.Name == "x" || Property.Name == "y" || Property.Name == "z" ||
							Property.Name == "X" || Property.Name == "Y" || Property.Name == "Z")
						{
							if (Property.Type == PLYPropertyType::FLOAT)
							{
								bHaveFloatPositions = true;
								break;
							}
							else if (Property.Type == PLYPropertyType::DOUBLE)
							{
								bHaveDoublePositions = true;
								break;
							}
						}
					}

					if (bHaveFloatPositions || bHaveDoublePositions)
						break;
				}

				if (bHaveFloatPositions || bHaveDoublePositions)
					break;
			}
		}
	}

	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == ElementName)
		{
			if (bHaveFloatPositions)
			{
				Positions = std::vector<glm::vec3>();
				std::get<std::vector<glm::vec3>>(Positions).reserve(PLYData->Elements[i].Entries.size());
			}
			else if (bHaveDoublePositions)
			{
				Positions = std::vector<glm::dvec3>();
				std::get<std::vector<glm::dvec3>>(Positions).reserve(PLYData->Elements[i].Entries.size());
			}

			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				glm::vec3 CurrentVertex = glm::vec3(0.0f);
				glm::dvec3 CurrentVertexDouble = glm::dvec3(0.0);
				bool bReadSomeData = false;

				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);

						if ((Property.Name == "x" || Property.Name == "X"))
						{
							if (bHaveFloatPositions)
								CurrentVertex.x = std::get<float>(CurrentValue);
							else if (bHaveDoublePositions)
								CurrentVertexDouble.x = std::get<double>(CurrentValue);

							bReadSomeData = true;
						}
						else if ((Property.Name == "y" || Property.Name == "Y"))
						{
							if (bHaveFloatPositions)
								CurrentVertex.y = std::get<float>(CurrentValue);
							else if (bHaveDoublePositions)
								CurrentVertexDouble.y = std::get<double>(CurrentValue);

							bReadSomeData = true;
						}
						else if ((Property.Name == "z" || Property.Name == "Z"))
						{
							if (bHaveFloatPositions)
								CurrentVertex.z = std::get<float>(CurrentValue);
							else if (bHaveDoublePositions)
								CurrentVertexDouble.z = std::get<double>(CurrentValue);

							bReadSomeData = true;
						}
					}
				}

				if (bReadSomeData)
				{
					if (bHaveFloatPositions)
						std::get<std::vector<glm::vec3>>(Positions).push_back(CurrentVertex);
					else if (bHaveDoublePositions)
						std::get<std::vector<glm::dvec3>>(Positions).push_back(CurrentVertexDouble);
				}
			}

			break;
		}
	}

	return Positions;
}

std::vector<std::vector<unsigned char>> FEResourceManager::ExtractColorsFromPLYData(FERawPLYData* PLYData)
{
	std::vector<std::vector<unsigned char>> Colors;

	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Colors;
	}

	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == "vertex")
		{
			bool bCorrectElement = false;
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "red" || Property.Name == "r" ||
							(Property.Name == "green" || Property.Name == "g") ||
							(Property.Name == "blue" || Property.Name == "b") ||
							(Property.Name == "alpha" || Property.Name == "a")) && Property.Type == PLYPropertyType::UCHAR)
						{
							bCorrectElement = true;
						}
					}

					if (bCorrectElement)
						break;
				}

				if (bCorrectElement)
					break;
			}

			if (!bCorrectElement)
				continue;

			Colors.reserve(PLYData->Elements[i].Entries.size());

			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				std::vector<unsigned char> CurrentVertexColor;
				CurrentVertexColor.resize(4);
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);

						if ((Property.Name == "red" || Property.Name == "r") && Property.Type == PLYPropertyType::UCHAR) CurrentVertexColor[0] = std::get<unsigned char>(CurrentValue);
						else if ((Property.Name == "green" || Property.Name == "g") && Property.Type == PLYPropertyType::UCHAR) CurrentVertexColor[1] = std::get<unsigned char>(CurrentValue);
						else if ((Property.Name == "blue" || Property.Name == "b") && Property.Type == PLYPropertyType::UCHAR) CurrentVertexColor[2] = std::get<unsigned char>(CurrentValue);
						else if ((Property.Name == "alpha" || Property.Name == "a") && Property.Type == PLYPropertyType::UCHAR) CurrentVertexColor[3] = std::get<unsigned char>(CurrentValue);
					}
				}

				Colors.push_back(CurrentVertexColor);
			}

			break;
		}
	}

	return Colors;
}

std::vector<int> FEResourceManager::ExtractIndicesFromPLYData(FERawPLYData* PLYData)
{
	std::vector<int> Indices;

	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Indices;
	}

	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == "face")
		{
			Indices.reserve(PLYData->Elements[i].Entries.size());
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYListValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYListValue& CurrentValue = std::get<PLYListValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if (CurrentValue.size() != 3)
						{
							LOG.Add("FEResourceManager::ExtractIndicesFromPLYData: Indices size is not 3", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
							return std::vector<int>();
						}
						
						if ((Property.Name == "vertex_indices" || Property.Name == "vertex_index" || Property.Name == "indices") && Property.bIsList)
						{
							if (Property.Type == PLYPropertyType::INT)
							{
								for (size_t l = 0; l < CurrentValue.size(); l++)
								{
									Indices.push_back(std::get<int>(CurrentValue[l]));
								}
							}
							else if (Property.Type == PLYPropertyType::UINT)
							{
								for (size_t l = 0; l < CurrentValue.size(); l++)
								{
									Indices.push_back(std::get<unsigned int>(CurrentValue[l]));
								}
							}
						}
					}
				}
			}
			break;
		}
	}

	return Indices;
}

std::vector<glm::vec2> FEResourceManager::ExtractUVsFromPLYData(FERawPLYData* PLYData, bool& bTextureCoordinatesArePartOfVertex)
{
	std::vector<glm::vec2> UVs;
	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return UVs;
	}

	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == "vertex")
		{
			bool bCorrectElement = false;
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "u" || Property.Name == "U" ||
							Property.Name == "s" || Property.Name == "S" ||
							Property.Name == "texture_u") && Property.Type == PLYPropertyType::FLOAT)
						{
							bCorrectElement = true;
						}
							
						if ((Property.Name == "v" || Property.Name == "V" ||
							Property.Name == "t" || Property.Name == "T" ||
							Property.Name == "texture_v") && Property.Type == PLYPropertyType::FLOAT)
						{
							bCorrectElement = true;
						}	
					}

					if (bCorrectElement)
						break;
				}

				if (bCorrectElement)
					break;
			}

			if (!bCorrectElement)
				continue;

			bTextureCoordinatesArePartOfVertex = true;

			UVs.reserve(PLYData->Elements[i].Entries.size());
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				glm::vec2 CurrentUV;
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "u" || Property.Name == "U" ||
							Property.Name == "s" || Property.Name == "S" ||
							Property.Name == "texture_u") && Property.Type == PLYPropertyType::FLOAT)
						{
							CurrentUV.x = std::get<float>(CurrentValue);
						}
							
						if ((Property.Name == "v" || Property.Name == "V" ||
							Property.Name == "t" || Property.Name == "T" ||
							Property.Name == "texture_v") && Property.Type == PLYPropertyType::FLOAT)
						{
							CurrentUV.y = std::get<float>(CurrentValue);
						}
					}
				}
				UVs.push_back(CurrentUV);
			}
			break;
		}
		else if (PLYData->Elements[i].Description.Name.find("texture") != std::string::npos)
		{
			bool bCorrectElement = false;
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "u" || Property.Name == "U") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;

						if ((Property.Name == "v" || Property.Name == "V") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
					}

					if (bCorrectElement)
						break;
				}

				if (bCorrectElement)
					break;
			}

			if (!bCorrectElement)
				continue;

			bTextureCoordinatesArePartOfVertex = false;

			UVs.reserve(PLYData->Elements[i].Entries.size());
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				glm::vec2 CurrentUV;
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if (Property.Name == "u" && Property.Type == PLYPropertyType::FLOAT) CurrentUV.x = std::get<float>(CurrentValue);
						else if (Property.Name == "v" && Property.Type == PLYPropertyType::FLOAT) CurrentUV.y = std::get<float>(CurrentValue);
					}
				}
				UVs.push_back(CurrentUV);
			}
			break;
		}
	}

	return UVs;
}

std::vector<glm::vec3> FEResourceManager::ExtractNormalsFromPLYData(FERawPLYData* PLYData)
{
	std::vector<glm::vec3> Normals;
	if (PLYData == nullptr)
	{
		LOG.Add("PLYData is nullptr in function FEResourceManager::RawPLYDataToFEMesh.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Normals;
	}

	for (size_t i = 0; i < PLYData->Elements.size(); i++)
	{
		if (PLYData->Elements[i].Description.Name == "vertex")
		{
			bool bCorrectElement = false;
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "nx" || Property.Name == "NX") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
						if ((Property.Name == "ny" || Property.Name == "NY") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
						if ((Property.Name == "nz" || Property.Name == "NZ") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
					}
					if (bCorrectElement)
						break;
				}
				if (bCorrectElement)
					break;
			}

			if (!bCorrectElement)
				continue;

			Normals.reserve(PLYData->Elements[i].Entries.size());
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				glm::vec3 CurrentNormal;
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "nx" || Property.Name == "NX") && Property.Type == PLYPropertyType::FLOAT) CurrentNormal.x = std::get<float>(CurrentValue);
						else if ((Property.Name == "ny" || Property.Name == "NY") && Property.Type == PLYPropertyType::FLOAT) CurrentNormal.y = std::get<float>(CurrentValue);
						else if ((Property.Name == "nz" || Property.Name == "NZ") && Property.Type == PLYPropertyType::FLOAT) CurrentNormal.z = std::get<float>(CurrentValue);
					}
				}
				Normals.push_back(CurrentNormal);
			}
			break;
		}
		else if (PLYData->Elements[i].Description.Name.find("normal") != std::string::npos)
		{
			bool bCorrectElement = false;
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "nx" || Property.Name == "NX") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
						if ((Property.Name == "ny" || Property.Name == "NY") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
						if ((Property.Name == "nz" || Property.Name == "NZ") && Property.Type == PLYPropertyType::FLOAT)
							bCorrectElement = true;
					}
					if (bCorrectElement)
						break;
				}
				if (bCorrectElement)
					break;
			}

			if (!bCorrectElement)
				continue;

			Normals.reserve(PLYData->Elements[i].Entries.size());
			for (size_t j = 0; j < PLYData->Elements[i].Entries.size(); j++)
			{
				glm::vec3 CurrentNormal;
				for (size_t k = 0; k < PLYData->Elements[i].Entries[j].PropertyValues.size(); k++)
				{
					auto Property = PLYData->Header->ElementSchemas[i].PropertyDefinitions[k];
					if (std::holds_alternative<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]))
					{
						const PLYScalarValue& CurrentValue = std::get<PLYScalarValue>(PLYData->Elements[i].Entries[j].PropertyValues[k]);
						if ((Property.Name == "nx" || Property.Name == "NX") && Property.Type == PLYPropertyType::FLOAT)
							CurrentNormal.x = std::get<float>(CurrentValue);
						else if ((Property.Name == "ny" || Property.Name == "NY") && Property.Type == PLYPropertyType::FLOAT)
							CurrentNormal.y = std::get<float>(CurrentValue);
						else if ((Property.Name == "nz" || Property.Name == "NZ") && Property.Type == PLYPropertyType::FLOAT)
							CurrentNormal.z = std::get<float>(CurrentValue);
					}
				}
				Normals.push_back(CurrentNormal);
			}
		}
	}

	return Normals;
}

std::string FEResourceManager::GetEngineFolder()
{
	return EngineFolder;
}

FELineCollection* FEResourceManager::RawDataToFELineCollection(std::vector<FELine> Lines, std::string Name)
{
	FELineCollection* NewLineCollection = new FELineCollection(Lines);
	if (!Name.empty())
		NewLineCollection->SetName(Name);

	LineCollections[NewLineCollection->GetObjectID()] = NewLineCollection;
	return NewLineCollection;
}

void FEResourceManager::DeleteFELineCollection(const FELineCollection* LineCollection)
{
	if (LineCollection == nullptr)
	{
		LOG.Add("FEResourceManager::DeleteFELineCollection: LineCollection is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	auto LineCollectionIterator = LineCollections.find(LineCollection->GetObjectID());
	if (LineCollectionIterator != LineCollections.end())
	{
		delete LineCollectionIterator->second;
		LineCollections.erase(LineCollectionIterator);
	}
	else
	{
		LOG.Add("FEResourceManager::DeleteFELineCollection: LineCollection not found in LineCollections map", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
	}
}

std::vector<std::string> FEResourceManager::GetFELineCollectionIDList()
{
	FE_MAP_TO_STR_VECTOR(LineCollections)
}

std::vector<std::string> FEResourceManager::GetEnginePrivateFELineCollectionIDList()
{
	return GetResourceIDListByTag(LineCollections, ENGINE_RESOURCE_TAG);
}

FELineCollection* FEResourceManager::GetLineCollection(std::string ID)
{
	if (LineCollections.find(ID) == LineCollections.end())
		return nullptr;

	return LineCollections[ID];
}

std::vector<FELineCollection*> FEResourceManager::GetLineCollectionByName(std::string Name)
{
	std::vector<FELineCollection*> Result;
	auto LineCollectionIterator = LineCollections.begin();
	while (LineCollectionIterator != LineCollections.end())
	{
		if (LineCollectionIterator->second->GetName() == Name)
			Result.push_back(LineCollectionIterator->second);

		LineCollectionIterator++;
	}

	return Result;
}

FELineCollection* FEResourceManager::LoadFELineCollection(std::string FilePath, std::string Name)
{
	if (FILE_SYSTEM.DoesFileExist(FilePath) == false)
	{
		LOG.Add(std::string("can't load file: ") + FilePath + " in function FEResourceManager::LoadFELineCollection.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add("Can't load file: " + FilePath + " in function FEResourceManager::LoadFELineCollection.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	char* Buffer = new char[4];
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version != FE_LINE_COLLECTION_VERSION)
	{
		LOG.Add("Can't load file: " + FilePath + " in function FEResourceManager::LoadFELineCollection. File was created in different version of engine!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	std::string LoadedObjectID;
	std::string LoadedName;
	FEObjectLoadedData ObjectData = OBJECT_MANAGER.LoadFEObjectPart(File);
	LoadedObjectID = ObjectData.ID;
	LoadedName = ObjectData.Name;

	std::vector<FELine> RawData;
	char* BigBuffer = new char[8];
	File.read(BigBuffer, 8);
	const size_t LineCount = *(size_t*)BigBuffer;
	size_t FELineSize = sizeof(FELine);

	char* LineBuffer = new char[LineCount * FELineSize];
	File.read(LineBuffer, LineCount * FELineSize);
	RawData.resize(LineCount);
	for (size_t i = 0; i < LineCount; i++)
	{
		FELine* CurrentLine = reinterpret_cast<FELine*>(LineBuffer + i * FELineSize);
		RawData[i] = *CurrentLine;
	}
		
	File.close();

	FELineCollection* NewLineCollection = new FELineCollection(RawData);
	const std::string OldID = NewLineCollection->ID;
	// Overwrite ID with Loaded ID.
	if (!LoadedObjectID.empty())
	{
		NewLineCollection->SetID(LoadedObjectID);
		LineCollections.erase(OldID);
		LineCollections[NewLineCollection->GetObjectID()] = NewLineCollection;
	}

	NewLineCollection->SetName(Name);
	NewLineCollection->Tag = ObjectData.Tag;

	delete[] Buffer;
	delete[] BigBuffer;
	delete[] LineBuffer;

	return NewLineCollection;
}

void FEResourceManager::SaveFELineCollection(FELineCollection* LineCollection, std::string FilePath)
{
	if (LineCollection == nullptr)
	{
		LOG.Add("FEResourceManager::SaveFELineCollection: LineCollection is nullptr", "FE_RESOURCE_MANAGER", FE_LOG_WARNING);
		return;
	}

	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);

	float Version = FE_LINE_COLLECTION_VERSION;
	File.write((char*)&Version, sizeof(float));

	OBJECT_MANAGER.SaveFEObjectPart(File, LineCollection);

	std::vector<FELine> RawData = LineCollection->GetRawData();
	size_t LineCount = RawData.size();
	File.write((char*)&LineCount, sizeof(size_t));
	File.write((char*)RawData.data(), sizeof(FELine) * LineCount);
	
	File.close();
}