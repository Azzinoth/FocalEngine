#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEVFSDirectory;
struct FEVFSFile
{
	FEVFSFile();
	FEVFSFile(FEObject* data, FEVFSDirectory* inDirectory);
	FEVFSDirectory* inDirectory;
	FEObject* data;
};

class FEVirtualFileSystem;
class FEVFSDirectory : public FEObject
{
	friend FEVirtualFileSystem;

	FEVFSDirectory();
	~FEVFSDirectory();

	FEVFSDirectory* parent;
	std::vector<FEVFSDirectory*> subDirectories;
	std::vector<FEVFSFile> files;

	bool addFile(FEObject* file);
	bool deleteFile(FEObject* file);
	bool hasFile(FEObject* file);

	bool addSubDirectory(std::string name, std::string forceObjectID = "");
	bool hasSubDirectory(std::string subDirectory);
	FEVFSDirectory* getSubDirectory(std::string subDirectory);
	void clear();
};

class FEVirtualFileSystem
{
public:
	SINGLETON_PUBLIC_PART(FEVirtualFileSystem)

	bool isPathCorrect(std::string path);
	bool createFile(FEObject* data, std::string path);
	bool moveFile(FEObject* data, std::string oldPath, std::string newPath);
	bool deleteFile(FEObject* data, std::string path);

	bool createDirectory(std::string name, std::string path);
	std::string createDirectory(std::string path);
	bool renameDirectory(std::string newName, std::string path);
	bool moveDirectory(std::string directoryPath, std::string newPath);
	bool deleteEmptyDirectory(std::string path);
	int subDirectoriesCount(std::string path);
	std::vector<FEObject*> getDirectoryContent(std::string path);
	std::string getDirectoryParent(std::string path);

	void clear();
	bool acceptableName(std::string name);

	std::string getCurrentPath();
	bool setCurrentPath(std::string path);

	void locateAndDeleteFile(FEObject* file);

	void saveState(std::string fileName);
	void loadState(std::string fileName);
private:
	SINGLETON_PRIVATE_PART(FEVirtualFileSystem)
	FEVFSDirectory* root;
	FEVFSDirectory* pathToDirectory(std::string path);
	std::string directoryToPath(FEVFSDirectory* directory);
	void deleteDirectory(FEVFSDirectory* directory);
	std::string currentPath = "/";
	void locateAndDeleteFileRecursive(FEVFSDirectory* directory, FEObject* file);

	void saveStateRecursive(Json::Value* localRoot, FEVFSDirectory* directory);
	void loadStateRecursive(Json::Value* localRoot, FEVFSDirectory* parent, FEVFSDirectory* directory, std::string forceObjectID);
};

#define VIRTUAL_FILE_SYSTEM_VERSION 0.01f
#define VIRTUAL_FILE_SYSTEM FEVirtualFileSystem::getInstance()
