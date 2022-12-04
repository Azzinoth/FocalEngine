#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEVFSDirectory;
struct FEVFSFile
{
	FEVFSFile();
	FEVFSFile(FEObject* Data, FEVFSDirectory* InDirectory);
	FEVFSDirectory* InDirectory;
	FEObject* Data;
	bool bReadOnly = false;
	bool IsReadOnly();
	void SetReadOnly(bool NewValue);
};

class FEVirtualFileSystem;
class FEVFSDirectory : public FEObject
{
	friend FEVirtualFileSystem;

	FEVFSDirectory();
	~FEVFSDirectory();

	FEVFSDirectory* Parent;
	std::vector<FEVFSDirectory*> SubDirectories;
	std::vector<FEVFSFile> Files;

	bool AddFile(FEObject* File);
	bool DeleteFile(const FEObject* File);
	bool HasFile(const FEObject* File);

	bool AddSubDirectory(std::string Name, std::string ForceObjectID = "");
	bool HasSubDirectory(std::string SubDirectory);
	FEVFSDirectory* GetSubDirectory(std::string SubDirectory);
	void Clear();

	bool bReadOnly = false;
	bool IsReadOnly();
	void SetReadOnly(bool NewValue);
};

class FEVirtualFileSystem
{
public:
	SINGLETON_PUBLIC_PART(FEVirtualFileSystem)

	bool IsPathCorrect(std::string Path);
	bool CreateFile(FEObject* Data, std::string Path);
	bool MoveFile(FEObject* Data, std::string OldPath, std::string NewPath);
	bool DeleteFile(const FEObject* Data, std::string Path);
	void SetFileReadOnly(bool NewValue, const FEObject* Data, std::string Path);

	bool CreateDirectory(std::string Name, std::string Path);
	std::string CreateDirectory(std::string Path);
	bool RenameDirectory(std::string NewName, std::string Path);
	bool MoveDirectory(std::string DirectoryPath, std::string NewPath);
	bool DeleteEmptyDirectory(std::string Path);
	int SubDirectoriesCount(std::string Path);
	std::vector<FEObject*> GetDirectoryContent(std::string Path);
	std::string GetDirectoryParent(std::string Path);
	void SetDirectoryReadOnly(bool NewValue, std::string Path);

	void Clear();
	bool AcceptableName(std::string Name);

	std::string GetCurrentPath();
	bool SetCurrentPath(std::string Path);

	std::string LocateFile(FEObject* File);
	void LocateAndDeleteFile(FEObject* File);

	void SaveState(std::string FileName);
	void LoadState(std::string FileName);

	bool IsReadOnly(const FEObject* Data, std::string Path);
private:
	SINGLETON_PRIVATE_PART(FEVirtualFileSystem)
	FEVFSDirectory* Root;
	FEVFSDirectory* PathToDirectory(std::string Path);
	std::string DirectoryToPath(FEVFSDirectory* Directory);
	void DeleteDirectory(FEVFSDirectory* Directory);
	std::string CurrentPath = "/";
	std::string LocateFileRecursive(FEVFSDirectory* Directory, FEObject* File);

	void SaveStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Directory);
	void LoadStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Parent, FEVFSDirectory* Directory, std::string ForceObjectID);
};

#define VIRTUAL_FILE_SYSTEM_VERSION 0.01f
#define VIRTUAL_FILE_SYSTEM FEVirtualFileSystem::getInstance()
