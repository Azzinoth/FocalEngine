#include "FEEditorVirtualFileSystem.h"
using namespace FocalEngine;

FEVFSFile::FEVFSFile()
{
	inDirectory = nullptr;
	data = nullptr;
}

FEVFSFile::FEVFSFile(FEObject* data, FEVFSDirectory* inDirectory)
{
	this->data = data;
	this->inDirectory = inDirectory;
}

bool FEVFSFile::isReadOnly()
{
	return readOnly;
}

void FEVFSFile::setReadOnly(bool newValue)
{
	readOnly = newValue;
}

FEVFSDirectory::FEVFSDirectory() : FEObject(FE_NULL, "")
{
	parent = nullptr;
}

FEVFSDirectory::~FEVFSDirectory()
{
	for (size_t i = 0; i < subDirectories.size(); i++)
	{
		delete subDirectories[i];
	}

	subDirectories.clear();
	files.clear();
}

bool FEVFSDirectory::hasFile(FEObject* file)
{
	if (file == nullptr)
	{
		LOG.add("file is nullptr in function FEVFSDirectory::hasFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	for (size_t i = 0; i < files.size(); i++)
	{
		if (files[i].data->getObjectID() == file->getObjectID())
			return true;
	}

	return false;
}

bool FEVFSDirectory::addSubDirectory(std::string name, std::string forceObjectID)
{
	if (hasSubDirectory(name))
		return false;

	FEVFSDirectory* newDirectory = new FEVFSDirectory();
	if (forceObjectID != "")
		newDirectory->setIDOfUnTyped(forceObjectID);
	newDirectory->setName(name);
	newDirectory->parent = this;
	subDirectories.push_back(newDirectory);

	return true;
}

bool FEVFSDirectory::hasSubDirectory(std::string subDirectory)
{
	if (subDirectory.size() == 0)
		return false;

	for (size_t i = 0; i < subDirectories.size(); i++)
	{
		if (subDirectories[i]->getName() == subDirectory)
			return true;
	}

	return false;
}

FEVFSDirectory* FEVFSDirectory::getSubDirectory(std::string subDirectory)
{
	if (subDirectory.size() == 0)
		return nullptr;

	for (size_t i = 0; i < subDirectories.size(); i++)
	{
		if (subDirectories[i]->getName() == subDirectory)
			return subDirectories[i];
	}

	return nullptr;
}

void FEVFSDirectory::clear()
{
	for (size_t i = 0; i < subDirectories.size(); i++)
	{
		delete subDirectories[i];
	}

	subDirectories.clear();
	files.clear();
}

bool FEVFSDirectory::deleteFile(FEObject* file)
{
	if (file == nullptr)
	{
		LOG.add("file is nullptr in function FEVFSDirectory::deleteFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	for (size_t i = 0; i < files.size(); i++)
	{
		if (file->getObjectID() == files[i].data->getObjectID() && !files[i].isReadOnly())
		{
			files.erase(files.begin() + i, files.begin() + i + 1);
			return true;
		}
	}

	return false;
}

bool FEVFSDirectory::addFile(FEObject* file)
{
	if (file == nullptr)
	{
		LOG.add("file is nullptr in function FEVFSDirectory::addFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	files.push_back(FEVFSFile(file, this));
	return true;
}

bool FEVFSDirectory::isReadOnly()
{
	return readOnly;
}

void FEVFSDirectory::setReadOnly(bool newValue)
{
	readOnly = newValue;
}

FEVirtualFileSystem* FEVirtualFileSystem::_instance = nullptr;
FEVirtualFileSystem::FEVirtualFileSystem()
{
	root = new FEVFSDirectory();
	root->parent = root;
	root->setName("/");
}

FEVirtualFileSystem::~FEVirtualFileSystem()
{
	clear();
}

bool FEVirtualFileSystem::isPathCorrect(std::string path)
{
	if (path.size() == 0)
		return false;

	if (path.find('/') == std::string::npos)
		return false;

	if (path[0] == '/')
		path.erase(0, 1);

	// it is root directory
	if (path.size() == 0)
		return true;

	std::vector<std::string> tokenizedPath;
	std::istringstream iss(path);
	std::string token;
	while (std::getline(iss, token, '/'))
	{
		tokenizedPath.push_back(token);
	}

	FEVFSDirectory* currentDirectory = root;
	for (size_t i = 0; i < tokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (currentDirectory->getSubDirectory(tokenizedPath[i]) == nullptr && i == tokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < currentDirectory->files.size(); j++)
			{
				if (currentDirectory->files[j].data->getName() == tokenizedPath[i])
					return true;
			}

			return false;
		}

		currentDirectory = currentDirectory->getSubDirectory(tokenizedPath[i]);

		if (currentDirectory == nullptr)
			return false;
	}
	
	return true;
}

FEVFSDirectory* FEVirtualFileSystem::pathToDirectory(std::string path)
{
	if (!isPathCorrect(path))
		return nullptr;

	if (path[0] == '/')
		path.erase(0, 1);

	// it is root directory
	if (path.size() == 0)
		return root;

	std::vector<std::string> tokenizedPath;
	std::istringstream iss(path);
	std::string token;
	while (std::getline(iss, token, '/'))
	{
		tokenizedPath.push_back(token);
	}

	FEVFSDirectory* currentDirectory = root;
	for (size_t i = 0; i < tokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (currentDirectory->getSubDirectory(tokenizedPath[i]) == nullptr && i == tokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < currentDirectory->files.size(); j++)
			{
				// If that the case we return last valid directory.
				if (currentDirectory->files[j].data->getName() == tokenizedPath[i])
					return currentDirectory;
			}
		}

		currentDirectory = currentDirectory->getSubDirectory(tokenizedPath[i]);
		if (currentDirectory == nullptr)
			return nullptr;
	}

	return currentDirectory;
}

bool FEVirtualFileSystem::createFile(FEObject* data, std::string path)
{
	if (data == nullptr)
	{
		LOG.add("data is nullptr in function FEVirtualFileSystem::createFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (data->getType() != FE_SHADER &&
		data->getType() != FE_TEXTURE &&
		data->getType() != FE_MESH &&
		data->getType() != FE_MATERIAL &&
		data->getType() != FE_GAMEMODEL &&
		data->getType() != FE_PREFAB)
	{
		LOG.add("data type is not supported in function FEVirtualFileSystem::createFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	if (!isPathCorrect(path))
		return false;

	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory->hasFile(data))
		return false;

	directory->files.push_back(FEVFSFile(data, directory));

	return true;
}

std::vector<FEObject*> FEVirtualFileSystem::getDirectoryContent(std::string path)
{
	std::vector<FEObject*> result;
	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return result;

	for (size_t i = 0; i < directory->subDirectories.size(); i++)
	{
		result.push_back(directory->subDirectories[i]);
	}

	for (size_t i = 0; i < directory->files.size(); i++)
	{
		result.push_back(directory->files[i].data);
	}

	return result;
}

bool FEVirtualFileSystem::createDirectory(std::string name, std::string path)
{
	if (!acceptableName(name))
		return false;

	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return false;

	if (directory->hasSubDirectory(name))
		return false;

	FEVFSDirectory* newDirectory = new FEVFSDirectory();
	newDirectory->setName(name);
	newDirectory->parent = directory;
	directory->subDirectories.push_back(newDirectory);

	return true;
}

std::string FEVirtualFileSystem::createDirectory(std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return "";

	int count = 1;
	std::string newDirectoryNameBaseName = "new directory";
	std::string newDirectoryName = newDirectoryNameBaseName;
	while (directory->hasSubDirectory(newDirectoryName))
	{
		newDirectoryName = newDirectoryNameBaseName + "_" + std::to_string(count);
		count++;
	}

	FEVFSDirectory* newDirectory = new FEVFSDirectory();
	newDirectory->setName(newDirectoryName);
	newDirectory->parent = directory;
	directory->subDirectories.push_back(newDirectory);

	return newDirectoryName;
}

void FEVirtualFileSystem::clear()
{
	root->clear();
}

bool FEVirtualFileSystem::renameDirectory(std::string newName, std::string path)
{
	if (!acceptableName(newName))
		return false;

	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return false;

	if (directory->hasSubDirectory(newName))
		return false;

	if (directory->isReadOnly())
		return false;

	directory->setName(newName);
	return true;
}

bool FEVirtualFileSystem::acceptableName(std::string name)
{
	if (name == "" || name.find('/') != std::string::npos)
		return false;

	return true;
}

std::string FEVirtualFileSystem::getCurrentPath()
{
	return currentPath;
}

bool FEVirtualFileSystem::setCurrentPath(std::string path)
{
	if (!isPathCorrect(path))
		return false;

	currentPath = path;
	return true;
}

bool FEVirtualFileSystem::moveFile(FEObject* data, std::string oldPath, std::string newPath)
{
	if (data == nullptr)
	{
		LOG.add("data is nullptr in function FEVirtualFileSystem::moveFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	FEVFSDirectory* oldDirectory = pathToDirectory(oldPath);
	if (oldDirectory == nullptr)
		return false;

	FEVFSDirectory* newDirectory = pathToDirectory(newPath);
	if (newDirectory == nullptr)
		return false;

	if (newDirectory->hasFile(data))
		return false;

	if (!oldDirectory->deleteFile(data))
		return false;
	newDirectory->addFile(data);

	return true;
}

bool FEVirtualFileSystem::moveDirectory(std::string directoryPath, std::string newPath)
{
	FEVFSDirectory* directory = pathToDirectory(directoryPath);
	if (directory == nullptr)
		return false;

	FEVFSDirectory* newDirectory = pathToDirectory(newPath);
	if (newDirectory == nullptr)
		return false;

	if (directory == newDirectory)
		return false;

	if (newDirectory->hasSubDirectory(directory->getName()))
		return false;

	if (directory->isReadOnly())
		return false;

	for (size_t i = 0; i < directory->parent->subDirectories.size(); i++)
	{
		if (directory->parent->subDirectories[i]->getObjectID() == directory->getObjectID())
		{
			directory->parent->subDirectories.erase(directory->parent->subDirectories.begin() + i, directory->parent->subDirectories.begin() + i + 1);
			break;
		}
	}

	newDirectory->subDirectories.push_back(directory);

	return true;
}

int FEVirtualFileSystem::subDirectoriesCount(std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return 0;

	return int(directory->subDirectories.size());
}

void FEVirtualFileSystem::deleteDirectory(FEVFSDirectory* directory)
{
	if (directory == nullptr)
	{
		LOG.add("directory is nullptr in function FEVirtualFileSystem::deleteDirectory.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (directory->isReadOnly())
		return;

	for (size_t i = 0; i < directory->parent->subDirectories.size(); i++)
	{
		if (directory->parent->subDirectories[i]->getObjectID() == directory->getObjectID())
		{
			directory->parent->subDirectories.erase(directory->parent->subDirectories.begin() + i, directory->parent->subDirectories.begin() + i + 1);
			return;
		}
	}
}

bool FEVirtualFileSystem::deleteEmptyDirectory(std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return false;

	if (directory->subDirectories.size() != 0 || directory->files.size() != 0)
		return false;

	if (directory->isReadOnly())
		return false;

	deleteDirectory(directory);
	return true;
}

std::string FEVirtualFileSystem::directoryToPath(FEVFSDirectory* directory)
{
	if (directory == nullptr)
		return "/";

	std::string result = directory->getName();
	FEVFSDirectory* currentDirectory = directory;

	while (currentDirectory->parent != currentDirectory)
	{
		currentDirectory = currentDirectory->parent;
		if (currentDirectory != root)
			result.insert(0, "/");
		
		result.insert(0, currentDirectory->getName());
		
	}

	return result;
}

std::string FEVirtualFileSystem::getDirectoryParent(std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return "/";

	return directoryToPath(directory->parent);
}

bool FEVirtualFileSystem::deleteFile(FEObject* data, std::string path)
{
	if (data == nullptr)
	{
		LOG.add("data is nullptr in function FEVirtualFileSystem::deleteFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	FEVFSDirectory* directory = pathToDirectory(path);
	if (directory == nullptr)
		return false;

	if (!directory->hasFile(data))
		return false;

	if (directory->isReadOnly())
		return false;

	directory->deleteFile(data);

	return true;
}

void FEVirtualFileSystem::locateAndDeleteFileRecursive(FEVFSDirectory* directory, FEObject* file)
{
	if (directory == nullptr)
	{
		LOG.add("directory is nullptr in function FEVirtualFileSystem::locateAndDeleteFileRecursive.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (file == nullptr)
	{
		LOG.add("file is nullptr in function FEVirtualFileSystem::locateAndDeleteFileRecursive.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	if (directory->isReadOnly())
		return;

	for (size_t i = 0; i < directory->files.size(); i++)
	{
		if (directory->files[i].data->getObjectID() == file->getObjectID() && !directory->files[i].isReadOnly())
		{
			directory->files.erase(directory->files.begin() + i, directory->files.begin() + i + 1);
			return;
		}
	}

	for (size_t i = 0; i < directory->subDirectories.size(); i++)
	{
		locateAndDeleteFileRecursive(directory->subDirectories[i], file);
	}
}

void FEVirtualFileSystem::locateAndDeleteFile(FEObject* file)
{
	if (file == nullptr)
	{
		LOG.add("file is nullptr in function FEVirtualFileSystem::locateAndDeleteFile.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	locateAndDeleteFileRecursive(root, file);
}

void FEVirtualFileSystem::saveStateRecursive(Json::Value* localRoot, FEVFSDirectory* directory)
{
	localRoot->operator[](directory->getObjectID())["name"] = directory->getName();
	
	Json::Value files;
	for (size_t i = 0; i < directory->files.size(); i++)
	{
		files[directory->files[i].data->getObjectID()];
	}
	localRoot->operator[](directory->getObjectID())["files"] = files;

	Json::Value subDirectories;
	for (size_t i = 0; i < directory->subDirectories.size(); i++)
	{
		saveStateRecursive(&subDirectories, directory->subDirectories[i]);
	}

	localRoot->operator[](directory->getObjectID())["subDirectories"] = subDirectories;
}

void FEVirtualFileSystem::saveState(std::string fileName)
{
	Json::Value JsonRoot;
	std::ofstream stateFile;
	stateFile.open(fileName);

	JsonRoot["version"] = VIRTUAL_FILE_SYSTEM_VERSION;
	saveStateRecursive(&JsonRoot, root);

	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, JsonRoot);

	stateFile << json_file;
	stateFile.close();
}

void FEVirtualFileSystem::loadStateRecursive(Json::Value* localRoot, FEVFSDirectory* parent, FEVFSDirectory* directory, std::string forceObjectID)
{
	directory->setIDOfUnTyped(forceObjectID);
	directory->setName(localRoot->operator[]("name").asCString());
	directory->parent = parent;

	std::vector<Json::String> files = localRoot->operator[]("files").getMemberNames();
	for (size_t j = 0; j < files.size(); j++)
	{
		directory->addFile(OBJECT_MANAGER.getFEObject(files[j]));
	}

	std::vector<Json::String> subDirectories = localRoot->operator[]("subDirectories").getMemberNames();
	for (size_t j = 0; j < subDirectories.size(); j++)
	{
		directory->addSubDirectory(localRoot->operator[]("subDirectories")[subDirectories[j]]["name"].asCString(), subDirectories[j]);
		loadStateRecursive(&localRoot->operator[]("subDirectories")[subDirectories[j]], directory, directory->subDirectories.back(), subDirectories[j]);
	}
}

void FEVirtualFileSystem::loadState(std::string fileName)
{
	std::ifstream stateFile;
	stateFile.open(fileName);

	std::string fileData((std::istreambuf_iterator<char>(stateFile)), std::istreambuf_iterator<char>());

	Json::Value JsonRoot;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &JsonRoot, &err))
		return;

	// read state file version
	float fileVersion = JsonRoot["version"].asFloat();
	if (fileVersion != VIRTUAL_FILE_SYSTEM_VERSION)
	{
		//
	}

	std::vector<Json::String> values = JsonRoot.getMemberNames();
	for (size_t i = 0; i < values.size(); i++)
	{
		if (values[i] != "version")
		{
			loadStateRecursive(&JsonRoot[values[i]], root, root, values[i]);
		}
	}
	
	stateFile.close();
}

bool FEVirtualFileSystem::isReadOnly(FEObject* data, std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);

	if (directory == nullptr)
	{
		for (size_t i = 0; i < root->files.size(); i++)
		{
			if (root->files[i].data == data)
			{
				return root->files[i].isReadOnly();
			}
		}

		return true;
	}

	if (directory->isReadOnly())
		return true;

	if (data == nullptr)
		return true;

	for (size_t i = 0; i < directory->files.size(); i++)
	{
		if (directory->files[i].data == data)
		{
			return directory->files[i].isReadOnly();
		}
	}

	return false;
}

void FEVirtualFileSystem::setDirectoryReadOnly(bool newValue, std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);

	if (directory == nullptr)
		return;

	directory->setReadOnly(newValue);
}

void FEVirtualFileSystem::setFileReadOnly(bool newValue, FEObject* data, std::string path)
{
	FEVFSDirectory* directory = pathToDirectory(path);

	if (directory == nullptr)
		return;

	for (size_t i = 0; i < directory->files.size(); i++)
	{
		if (directory->files[i].data == data)
		{
			directory->files[i].setReadOnly(newValue);
			return;
		}
	}
}