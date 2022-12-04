#include "FEEditorVirtualFileSystem.h"
using namespace FocalEngine;

FEVFSFile::FEVFSFile()
{
	InDirectory = nullptr;
	Data = nullptr;
}

FEVFSFile::FEVFSFile(FEObject* Data, FEVFSDirectory* InDirectory)
{
	this->Data = Data;
	this->InDirectory = InDirectory;
}

bool FEVFSFile::IsReadOnly()
{
	return bReadOnly;
}

void FEVFSFile::SetReadOnly(const bool NewValue)
{
	bReadOnly = NewValue;
}

FEVFSDirectory::FEVFSDirectory() : FEObject(FE_NULL, "")
{
	Parent = nullptr;
}

FEVFSDirectory::~FEVFSDirectory()
{
	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		delete SubDirectories[i];
	}

	SubDirectories.clear();
	Files.clear();
}

bool FEVFSDirectory::HasFile(const FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("file is nullptr in function FEVFSDirectory::hasFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < Files.size(); i++)
	{
		if (Files[i].Data->GetObjectID() == File->GetObjectID())
			return true;
	}

	return false;
}

bool FEVFSDirectory::AddSubDirectory(const std::string Name, const std::string ForceObjectID)
{
	if (HasSubDirectory(Name))
		return false;

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	if (!ForceObjectID.empty())
		NewDirectory->SetIDOfUnTyped(ForceObjectID);
	NewDirectory->SetName(Name);
	NewDirectory->Parent = this;
	SubDirectories.push_back(NewDirectory);

	return true;
}

bool FEVFSDirectory::HasSubDirectory(const std::string SubDirectory)
{
	if (SubDirectory.empty())
		return false;

	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		if (SubDirectories[i]->GetName() == SubDirectory)
			return true;
	}

	return false;
}

FEVFSDirectory* FEVFSDirectory::GetSubDirectory(const std::string SubDirectory)
{
	if (SubDirectory.empty())
		return nullptr;

	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		if (SubDirectories[i]->GetName() == SubDirectory)
			return SubDirectories[i];
	}

	return nullptr;
}

void FEVFSDirectory::Clear()
{
	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		delete SubDirectories[i];
	}

	SubDirectories.clear();
	Files.clear();
}

bool FEVFSDirectory::DeleteFile(const FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("file is nullptr in function FEVFSDirectory::deleteFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < Files.size(); i++)
	{
		if (File->GetObjectID() == Files[i].Data->GetObjectID() && !Files[i].IsReadOnly())
		{
			Files.erase(Files.begin() + i, Files.begin() + i + 1);
			return true;
		}
	}

	return false;
}

bool FEVFSDirectory::AddFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("file is nullptr in function FEVFSDirectory::addFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	Files.push_back(FEVFSFile(File, this));
	return true;
}

bool FEVFSDirectory::IsReadOnly()
{
	return bReadOnly;
}

void FEVFSDirectory::SetReadOnly(const bool NewValue)
{
	bReadOnly = NewValue;
}

FEVirtualFileSystem* FEVirtualFileSystem::Instance = nullptr;
FEVirtualFileSystem::FEVirtualFileSystem()
{
	Root = new FEVFSDirectory();
	Root->Parent = Root;
	Root->SetName("/");
}

FEVirtualFileSystem::~FEVirtualFileSystem()
{
	Clear();
}

bool FEVirtualFileSystem::IsPathCorrect(std::string Path)
{
	if (Path.empty())
		return false;

	if (Path.find('/') == std::string::npos)
		return false;

	if (Path[0] == '/')
		Path.erase(0, 1);

	// it is root directory
	if (Path.empty())
		return true;

	std::vector<std::string> TokenizedPath;
	std::istringstream Iss(Path);
	std::string Token;
	while (std::getline(Iss, Token, '/'))
	{
		TokenizedPath.push_back(Token);
	}

	FEVFSDirectory* CurrentDirectory = Root;
	for (size_t i = 0; i < TokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (CurrentDirectory->GetSubDirectory(TokenizedPath[i]) == nullptr && i == TokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < CurrentDirectory->Files.size(); j++)
			{
				if (CurrentDirectory->Files[j].Data->GetName() == TokenizedPath[i])
					return true;
			}

			return false;
		}

		CurrentDirectory = CurrentDirectory->GetSubDirectory(TokenizedPath[i]);

		if (CurrentDirectory == nullptr)
			return false;
	}
	
	return true;
}

FEVFSDirectory* FEVirtualFileSystem::PathToDirectory(std::string Path)
{
	if (!IsPathCorrect(Path))
		return nullptr;

	if (Path[0] == '/')
		Path.erase(0, 1);

	// it is root directory
	if (Path.empty())
		return Root;

	std::vector<std::string> TokenizedPath;
	std::istringstream Iss(Path);
	std::string Token;
	while (std::getline(Iss, Token, '/'))
	{
		TokenizedPath.push_back(Token);
	}

	FEVFSDirectory* CurrentDirectory = Root;
	for (size_t i = 0; i < TokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (CurrentDirectory->GetSubDirectory(TokenizedPath[i]) == nullptr && i == TokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < CurrentDirectory->Files.size(); j++)
			{
				// If that the case we return last valid directory.
				if (CurrentDirectory->Files[j].Data->GetName() == TokenizedPath[i])
					return CurrentDirectory;
			}
		}

		CurrentDirectory = CurrentDirectory->GetSubDirectory(TokenizedPath[i]);
		if (CurrentDirectory == nullptr)
			return nullptr;
	}

	return CurrentDirectory;
}

bool FEVirtualFileSystem::CreateFile(FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
	{
		LOG.Add("data is nullptr in function FEVirtualFileSystem::createFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (Data->GetType() != FE_SHADER &&
		Data->GetType() != FE_TEXTURE &&
		Data->GetType() != FE_MESH &&
		Data->GetType() != FE_MATERIAL &&
		Data->GetType() != FE_GAMEMODEL &&
		Data->GetType() != FE_PREFAB)
	{
		LOG.Add("data type is not supported in function FEVirtualFileSystem::createFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (!IsPathCorrect(Path))
		return false;

	FEVFSDirectory* directory = PathToDirectory(Path);
	if (directory->HasFile(Data))
		return false;

	directory->Files.push_back(FEVFSFile(Data, directory));

	return true;
}

std::vector<FEObject*> FEVirtualFileSystem::GetDirectoryContent(const std::string Path)
{
	std::vector<FEObject*> Result;
	const FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return Result;

	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		Result.push_back(Directory->SubDirectories[i]);
	}

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		Result.push_back(Directory->Files[i].Data);
	}

	return Result;
}

bool FEVirtualFileSystem::CreateDirectory(const std::string Name, const std::string Path)
{
	if (!AcceptableName(Name))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (Directory->HasSubDirectory(Name))
		return false;

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	NewDirectory->SetName(Name);
	NewDirectory->Parent = Directory;
	Directory->SubDirectories.push_back(NewDirectory);

	return true;
}

std::string FEVirtualFileSystem::CreateDirectory(const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return "";

	int Count = 1;
	const std::string NewDirectoryNameBaseName = "new directory";
	std::string NewDirectoryName = NewDirectoryNameBaseName;
	while (Directory->HasSubDirectory(NewDirectoryName))
	{
		NewDirectoryName = NewDirectoryNameBaseName + "_" + std::to_string(Count);
		Count++;
	}

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	NewDirectory->SetName(NewDirectoryName);
	NewDirectory->Parent = Directory;
	Directory->SubDirectories.push_back(NewDirectory);

	return NewDirectoryName;
}

void FEVirtualFileSystem::Clear()
{
	Root->Clear();
}

bool FEVirtualFileSystem::RenameDirectory(const std::string NewName, const std::string Path)
{
	if (!AcceptableName(NewName))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (Directory->HasSubDirectory(NewName))
		return false;

	if (Directory->IsReadOnly())
		return false;

	Directory->SetName(NewName);
	return true;
}

bool FEVirtualFileSystem::AcceptableName(const std::string Name)
{
	if (Name.empty() || Name.find('/') != std::string::npos)
		return false;

	return true;
}

std::string FEVirtualFileSystem::GetCurrentPath()
{
	return CurrentPath;
}

bool FEVirtualFileSystem::SetCurrentPath(const std::string Path)
{
	if (!IsPathCorrect(Path))
		return false;

	CurrentPath = Path;
	return true;
}

bool FEVirtualFileSystem::MoveFile(FEObject* Data, const std::string OldPath, const std::string NewPath)
{
	if (Data == nullptr)
	{
		LOG.Add("data is nullptr in function FEVirtualFileSystem::moveFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	FEVFSDirectory* OldDirectory = PathToDirectory(OldPath);
	if (OldDirectory == nullptr)
		return false;

	FEVFSDirectory* NewDirectory = PathToDirectory(NewPath);
	if (NewDirectory == nullptr)
		return false;

	if (NewDirectory->HasFile(Data))
		return false;

	if (!OldDirectory->DeleteFile(Data))
		return false;
	NewDirectory->AddFile(Data);

	return true;
}

bool FEVirtualFileSystem::MoveDirectory(const std::string DirectoryPath, const std::string NewPath)
{
	FEVFSDirectory* Directory = PathToDirectory(DirectoryPath);
	if (Directory == nullptr)
		return false;

	FEVFSDirectory* NewDirectory = PathToDirectory(NewPath);
	if (NewDirectory == nullptr)
		return false;

	if (Directory == NewDirectory)
		return false;

	if (NewDirectory->HasSubDirectory(Directory->GetName()))
		return false;

	if (Directory->IsReadOnly())
		return false;

	for (size_t i = 0; i < Directory->Parent->SubDirectories.size(); i++)
	{
		if (Directory->Parent->SubDirectories[i]->GetObjectID() == Directory->GetObjectID())
		{
			Directory->Parent->SubDirectories.erase(Directory->Parent->SubDirectories.begin() + i, Directory->Parent->SubDirectories.begin() + i + 1);
			break;
		}
	}

	Directory->Parent = PathToDirectory(NewPath);
	NewDirectory->SubDirectories.push_back(Directory);

	return true;
}

int FEVirtualFileSystem::SubDirectoriesCount(const std::string Path)
{
	const FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return 0;

	return static_cast<int>(Directory->SubDirectories.size());
}

void FEVirtualFileSystem::DeleteDirectory(FEVFSDirectory* Directory)
{
	if (Directory == nullptr)
	{
		LOG.Add("directory is nullptr in function FEVirtualFileSystem::deleteDirectory.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	if (Directory->IsReadOnly())
		return;

	for (size_t i = 0; i < Directory->Parent->SubDirectories.size(); i++)
	{
		if (Directory->Parent->SubDirectories[i]->GetObjectID() == Directory->GetObjectID())
		{
			Directory->Parent->SubDirectories.erase(Directory->Parent->SubDirectories.begin() + i, Directory->Parent->SubDirectories.begin() + i + 1);
			return;
		}
	}
}

bool FEVirtualFileSystem::DeleteEmptyDirectory(const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (!Directory->SubDirectories.empty() || !Directory->Files.empty())
		return false;

	if (Directory->IsReadOnly())
		return false;

	DeleteDirectory(Directory);
	return true;
}

std::string FEVirtualFileSystem::DirectoryToPath(FEVFSDirectory* Directory)
{
	if (Directory == nullptr)
		return "/";

	std::string Result = Directory->GetName();
	FEVFSDirectory* CurrentDirectory = Directory;

	while (CurrentDirectory->Parent != CurrentDirectory)
	{
		CurrentDirectory = CurrentDirectory->Parent;
		if (CurrentDirectory != Root)
			Result.insert(0, "/");
		
		Result.insert(0, CurrentDirectory->GetName());
		
	}

	return Result;
}

std::string FEVirtualFileSystem::GetDirectoryParent(const std::string Path)
{
	const FEVFSDirectory* directory = PathToDirectory(Path);
	if (directory == nullptr)
		return "/";

	return DirectoryToPath(directory->Parent);
}

bool FEVirtualFileSystem::DeleteFile(const FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
	{
		LOG.Add("data is nullptr in function FEVirtualFileSystem::deleteFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	FEVFSDirectory* directory = PathToDirectory(Path);
	if (directory == nullptr)
		return false;

	if (!directory->HasFile(Data))
		return false;

	if (directory->IsReadOnly())
		return false;

	directory->DeleteFile(Data);

	return true;
}

std::string FEVirtualFileSystem::LocateFileRecursive(FEVFSDirectory* Directory, FEObject* File)
{
	std::string Path = "";

	if (Directory == nullptr)
	{
		LOG.Add("Directory is nullptr in function FEVirtualFileSystem::LocateFileRecursive.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return Path;
	}

	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateFileRecursive.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return Path;
	}

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].Data->GetObjectID() == File->GetObjectID())
		{
			Path = DirectoryToPath(Directory);
			return Path;
		}
	}

	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		Path = LocateFileRecursive(Directory->SubDirectories[i], File);
		if (!Path.empty())
			return Path;
	}

	return Path;
}

std::string FEVirtualFileSystem::LocateFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return "";
	}

	return LocateFileRecursive(Root, File);
}

void FEVirtualFileSystem::LocateAndDeleteFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateAndDeleteFile.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	const std::string Path = LocateFile(File);
	if (Path.empty())
		return;

	if (IsReadOnly(File, Path))
		return;

	PathToDirectory(Path)->DeleteFile(File);
}

void FEVirtualFileSystem::SaveStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Directory)
{
	LocalRoot->operator[](Directory->GetObjectID())["name"] = Directory->GetName();
	
	Json::Value files;
	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		files[Directory->Files[i].Data->GetObjectID()];
	}
	LocalRoot->operator[](Directory->GetObjectID())["files"] = files;

	Json::Value SubDirectories;
	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		SaveStateRecursive(&SubDirectories, Directory->SubDirectories[i]);
	}

	LocalRoot->operator[](Directory->GetObjectID())["subDirectories"] = SubDirectories;
}

void FEVirtualFileSystem::SaveState(const std::string FileName)
{
	Json::Value JsonRoot;
	std::ofstream StateFile;
	StateFile.open(FileName);

	JsonRoot["version"] = VIRTUAL_FILE_SYSTEM_VERSION;
	SaveStateRecursive(&JsonRoot, Root);

	const Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, JsonRoot);

	StateFile << JsonFile;
	StateFile.close();
}

void FEVirtualFileSystem::LoadStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Parent, FEVFSDirectory* Directory, const std::string ForceObjectID)
{
	Directory->SetIDOfUnTyped(ForceObjectID);
	Directory->SetName(LocalRoot->operator[]("name").asCString());
	Directory->Parent = Parent;

	const std::vector<Json::String> files = LocalRoot->operator[]("files").getMemberNames();
	for (size_t j = 0; j < files.size(); j++)
	{
		Directory->AddFile(OBJECT_MANAGER.GetFEObject(files[j]));
	}

	const std::vector<Json::String> SubDirectories = LocalRoot->operator[]("subDirectories").getMemberNames();
	for (size_t j = 0; j < SubDirectories.size(); j++)
	{
		Directory->AddSubDirectory(LocalRoot->operator[]("subDirectories")[SubDirectories[j]]["name"].asCString(), SubDirectories[j]);
		LoadStateRecursive(&LocalRoot->operator[]("subDirectories")[SubDirectories[j]], Directory, Directory->SubDirectories.back(), SubDirectories[j]);
	}
}

void FEVirtualFileSystem::LoadState(std::string FileName)
{
	std::ifstream StateFile;
	StateFile.open(FileName);

	std::string FileData((std::istreambuf_iterator<char>(StateFile)), std::istreambuf_iterator<char>());

	Json::Value JsonRoot;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &JsonRoot, &Err))
		return;

	// read state file version
	float FileVersion = JsonRoot["version"].asFloat();
	if (FileVersion != VIRTUAL_FILE_SYSTEM_VERSION)
	{
		//
	}

	std::vector<Json::String> Values = JsonRoot.getMemberNames();
	for (size_t i = 0; i < Values.size(); i++)
	{
		if (Values[i] != "version")
		{
			LoadStateRecursive(&JsonRoot[Values[i]], Root, Root, Values[i]);
		}
	}
	
	StateFile.close();
}

bool FEVirtualFileSystem::IsReadOnly(const FEObject* Data, const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);

	if (Directory == nullptr)
	{
		for (size_t i = 0; i < Root->Files.size(); i++)
		{
			if (Root->Files[i].Data == Data)
			{
				return Root->Files[i].IsReadOnly();
			}
		}

		return true;
	}

	if (Directory->IsReadOnly())
		return true;

	if (Data == nullptr)
		return true;

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].Data == Data)
		{
			return Directory->Files[i].IsReadOnly();
		}
	}

	return false;
}

void FEVirtualFileSystem::SetDirectoryReadOnly(const bool NewValue, const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);

	if (Directory == nullptr)
		return;

	Directory->SetReadOnly(NewValue);
}

void FEVirtualFileSystem::SetFileReadOnly(const bool NewValue, const FEObject* Data, const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);

	if (Directory == nullptr)
		return;

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].Data == Data)
		{
			Directory->Files[i].SetReadOnly(NewValue);
			return;
		}
	}
}