#include "FEglTFLoader.h"
using namespace FocalEngine;

FEglTFLoader* FEglTFLoader::_instance = nullptr;

FEglTFLoader::FEglTFLoader()
{
    
}

FEglTFLoader::~FEglTFLoader()
{
   
}

void FEglTFLoader::clear()
{
	buffers.clear();
	bufferViews.clear();
	accessors.clear();
	primitives.clear();
	meshes.clear();
	entities.clear();

	images.clear();
	textures.clear();
	materials.clear();
	gameModels.clear();
}

void FEglTFLoader::loadPrimitive(Json::Value jsonPrimitive, glTFPrimitive& newPrimitive)
{
	if (jsonPrimitive.isMember("attributes"))
	{
		if (jsonPrimitive["attributes"].isMember("POSITION"))
			newPrimitive.attributes["POSITION"] = jsonPrimitive["attributes"]["POSITION"].asInt();

		if (jsonPrimitive["attributes"].isMember("NORMAL"))
			newPrimitive.attributes["NORMAL"] = jsonPrimitive["attributes"]["NORMAL"].asInt();

		if (jsonPrimitive["attributes"].isMember("TANGENT"))
			newPrimitive.attributes["TANGENT"] = jsonPrimitive["attributes"]["TANGENT"].asInt();

		std::string attributeName;

		for (size_t i = 0; i < 16; i++)
		{
			attributeName = std::string("TEXCOORD_") + std::to_string(i);
			if (jsonPrimitive["attributes"].isMember(attributeName.c_str()))
			{
				newPrimitive.attributes[attributeName] = jsonPrimitive["attributes"][attributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			attributeName = std::string("COLOR_") + std::to_string(i);
			if (jsonPrimitive["attributes"].isMember(attributeName.c_str()))
			{
				newPrimitive.attributes[attributeName] = jsonPrimitive["attributes"][attributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			attributeName = std::string("JOINTS_") + std::to_string(i);
			if (jsonPrimitive["attributes"].isMember(attributeName.c_str()))
			{
				newPrimitive.attributes[attributeName] = jsonPrimitive["attributes"][attributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			attributeName = std::string("WEIGHTS_") + std::to_string(i);
			if (jsonPrimitive["attributes"].isMember(attributeName.c_str()))
			{
				newPrimitive.attributes[attributeName] = jsonPrimitive["attributes"][attributeName].asInt();
			}
			else
			{
				break;
			}
		}
	}

	if (jsonPrimitive.isMember("indices"))
		newPrimitive.indices = jsonPrimitive["indices"].asInt();

	if (jsonPrimitive.isMember("material"))
		newPrimitive.material = jsonPrimitive["material"].asInt();

	
	if (loadMeshRawData(newPrimitive))
	{
		if (newPrimitive.material != -1)
		{
			glTFGameModel currentGameModel;
			currentGameModel.primitive = int(primitives.size());
			currentGameModel.material = newPrimitive.material;
			currentGameModel.meshParent = int(meshes.size());

			gameModels.push_back(currentGameModel);
		}
		else
		{
			int y = 0;
			y++;
		}
	}
	else
	{
		int y = 0;
		y++;
	}
}

glTFMaterialTexture FEglTFLoader::loadMaterialTexture(Json::Value jsonTextureNode)
{
	glTFMaterialTexture tempTexture;

	if (jsonTextureNode.isMember("index"))
	{
		tempTexture.index = jsonTextureNode["index"].asInt();
	}

	if (jsonTextureNode.isMember("texCoord"))
	{
		tempTexture.texCoord = jsonTextureNode["texCoord"].asInt();
	}

	if (jsonTextureNode.isMember("scale"))
	{
		tempTexture.texCoord = jsonTextureNode["scale"].asInt();
	}

	return tempTexture;
}

void FEglTFLoader::load(const char* fileName)
{
	clear();

	std::ifstream GLTFFile;
	GLTFFile.open(fileName);

	std::string fileData((std::istreambuf_iterator<char>(GLTFFile)), std::istreambuf_iterator<char>());
	GLTFFile.close();

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		return;

	std::string directory = FILE_SYSTEM.getDirectoryPath(fileName);

	if (root.isMember("buffers"))
	{
		Json::Value jsonBuffers = root["buffers"];
		for (size_t i = 0; i < jsonBuffers.size(); i++)
		{
			glTFBuffer currentBuffer;

			if (jsonBuffers[int(i)].isMember("uri"))
				currentBuffer.uri = jsonBuffers[int(i)]["uri"].asString();

			std::fstream file;
			file.open(directory + "\\" + currentBuffer.uri, std::ios::in | std::ios::binary | std::ios::ate);
			std::streamsize fileSize = file.tellg();
			if (fileSize < 0)
				LOG.add(std::string("can't load buffer from: ") + currentBuffer.uri + " in function FEglTFLoader::load.", FE_LOG_ERROR, FE_LOG_LOADING);

			file.seekg(0, std::ios::beg);
			currentBuffer.rawData = new char[int(fileSize)];
			file.read(currentBuffer.rawData, fileSize);
			file.close();

			if (jsonBuffers[int(i)].isMember("byteLength"))
				currentBuffer.byteLength = jsonBuffers[int(i)]["byteLength"].asInt();

			if (currentBuffer.byteLength != fileSize)
				LOG.add("byteLength and fileSize is not equal in function FEglTFLoader::load.", FE_LOG_ERROR, FE_LOG_LOADING);

			buffers.push_back(currentBuffer);
		}
	}

	if (root.isMember("bufferViews"))
	{
		Json::Value jsonBufferViews = root["bufferViews"];
		for (size_t i = 0; i < jsonBufferViews.size(); i++)
		{
			glTFBufferView currentBufferView;

			if (jsonBufferViews[int(i)].isMember("buffer"))
				currentBufferView.buffer = jsonBufferViews[int(i)]["buffer"].asInt();

			if (jsonBufferViews[int(i)].isMember("byteLength"))
				currentBufferView.byteLength = jsonBufferViews[int(i)]["byteLength"].asInt();

			if (jsonBufferViews[int(i)].isMember("byteOffset"))
				currentBufferView.byteOffset = jsonBufferViews[int(i)]["byteOffset"].asInt();

			if (jsonBufferViews[int(i)].isMember("target"))
				currentBufferView.target = jsonBufferViews[int(i)]["target"].asInt();

			bufferViews.push_back(currentBufferView);
		}
	}

	if (root.isMember("accessors"))
	{
		Json::Value jsonAccessors = root["accessors"];
		for (size_t i = 0; i < jsonAccessors.size(); i++)
		{
			glTFAccessor currentAccessor;

			if (jsonAccessors[int(i)].isMember("bufferView"))
				currentAccessor.bufferView = jsonAccessors[int(i)]["bufferView"].asInt();

			if (jsonAccessors[int(i)].isMember("byteOffset"))
				currentAccessor.byteOffset = jsonAccessors[int(i)]["byteOffset"].asInt();

			if (jsonAccessors[int(i)].isMember("componentType"))
				currentAccessor.componentType = jsonAccessors[int(i)]["componentType"].asInt();

			if (jsonAccessors[int(i)].isMember("count"))
				currentAccessor.count = jsonAccessors[int(i)]["count"].asInt();

			if (jsonAccessors[int(i)].isMember("type"))
				currentAccessor.type = jsonAccessors[int(i)]["type"].asString();

			accessors.push_back(currentAccessor);
		}
	}

	if (root.isMember("meshes"))
	{
		Json::Value jsonMeshes = root["meshes"];
		for (size_t i = 0; i < jsonMeshes.size(); i++)
		{
			glTFMesh currentMesh;
			currentMesh.name = jsonMeshes[int(i)]["name"].asCString();

			if (jsonMeshes[int(i)].isMember("primitives"))
			{
				Json::Value jsonPrimitives = jsonMeshes[int(i)]["primitives"];
				for (size_t j = 0; j < jsonPrimitives.size(); j++)
				{
					glTFPrimitive currentPrimitive;
					loadPrimitive(jsonPrimitives[int(j)], currentPrimitive);
					primitives.push_back(currentPrimitive);
					currentMesh.primitives.push_back(currentPrimitive);
				}
			}

			meshes.push_back(currentMesh);
		}
	}

	if (root.isMember("images"))
	{
		Json::Value jsonImages = root["images"];
		for (size_t i = 0; i < jsonImages.size(); i++)
		{
			if (jsonImages[int(i)].isMember("uri"))
			{
				std::string imageUri = (directory + jsonImages[int(i)]["uri"].asCString());
				images.push_back(imageUri);
			}
		}
	}

	if (root.isMember("textures"))
	{
		Json::Value jsonTextures = root["textures"];
		for (size_t i = 0; i < jsonTextures.size(); i++)
		{
			if (i == 811)
			{
				int y = 0;
				y++;
			}
			//std::string textureFile = (directory + jsonTextures[int(i)]["name"].asCString() + ".png");

			if (jsonTextures[int(i)].isMember("source"))
			{
				int imageIndex = jsonTextures[int(i)]["source"].asInt();
				if (imageIndex < images.size())
				{
					std::string textureFile = images[imageIndex];
					textures.push_back(textureFile);
				}
			}
		}
	}

	if (root.isMember("materials"))
	{
		Json::Value jsonMaterials = root["materials"];
		for (size_t i = 0; i < jsonMaterials.size(); i++)
		{
			glTFMaterial newMaterial;
			newMaterial.name = jsonMaterials[int(i)]["name"].asCString();

			/*if (newMaterial.name == "MTL_RW_Plants_20k_HierarchicalInstancedStaticMeshComponent_324")
			{
				int y = 0;
				y++;
			}*/

			if (jsonMaterials[int(i)].isMember("pbrMetallicRoughness"))
			{
				if (jsonMaterials[int(i)]["pbrMetallicRoughness"].isMember("baseColorFactor"))
				{
					newMaterial.baseColor = glm::vec4(jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorFactor"][0].asDouble(),
													  jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorFactor"][1].asDouble(),
													  jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorFactor"][2].asDouble(),
													  jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorFactor"][3].asDouble());
				}

				if (jsonMaterials[int(i)]["pbrMetallicRoughness"].isMember("baseColorTexture"))
				{
					newMaterial.baseColorTexture = loadMaterialTexture(jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"]);
					/*if (jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"].isMember("index"))
					{
						newMaterial.baseColorTexture.index = jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"]["index"].asInt();
					}

					if (jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"].isMember("texCoord"))
					{
						newMaterial.baseColorTexture.texCoord = jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"]["texCoord"].asInt();
					}

					if (jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"].isMember("scale"))
					{
						newMaterial.baseColorTexture.texCoord = jsonMaterials[int(i)]["pbrMetallicRoughness"]["baseColorTexture"]["scale"].asInt();
					}*/
				}

				if (jsonMaterials[int(i)]["pbrMetallicRoughness"].isMember("metallicRoughnessTexture"))
				{
					newMaterial.metallicRoughnessTexture = loadMaterialTexture(jsonMaterials[int(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]);
					//if (jsonMaterials[int(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"].isMember("index"))
					//{
					//	newMaterial.metallicRoughnessTexture = jsonMaterials[int(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"].asInt();
					//}
				}
			}

			if (jsonMaterials[int(i)].isMember("normalTexture"))
			{
				newMaterial.normalTexture = loadMaterialTexture(jsonMaterials[int(i)]["normalTexture"]);
				/*if (jsonMaterials[int(i)]["normalTexture"].isMember("index"))
				{
					newMaterial.normalTexture = jsonMaterials[int(i)]["normalTexture"]["index"].asInt();
				}*/
			}

			if (jsonMaterials[int(i)].isMember("occlusionTexture"))
			{
				newMaterial.occlusionTexture = loadMaterialTexture(jsonMaterials[int(i)]["occlusionTexture"]);
				/*if (jsonMaterials[int(i)]["occlusionTexture"].isMember("index"))
				{
					newMaterial.occlusionTexture = jsonMaterials[int(i)]["occlusionTexture"]["index"].asInt();
				}*/
			}

			materials.push_back(newMaterial);
		}
	}

	if (root.isMember("nodes"))
	{
		Json::Value jsonNodes = root["nodes"];
		for (size_t i = 0; i < jsonNodes.size(); i++)
		{
			glTFEntity newEntity;
			bool anyInfoWasRead = false;

			if (jsonNodes[int(i)].isMember("name"))
			{
				newEntity.name = jsonNodes[int(i)]["name"].asCString();
				anyInfoWasRead = true;
			}

			if (jsonNodes[int(i)].isMember("mesh"))
			{
				newEntity.mesh = jsonNodes[int(i)]["mesh"].asInt();
				anyInfoWasRead = true;
			}

			if (jsonNodes[int(i)].isMember("translation"))
			{
				newEntity.translation = glm::vec3(jsonNodes[int(i)]["translation"][0].asDouble(),
												  jsonNodes[int(i)]["translation"][1].asDouble(),
											      jsonNodes[int(i)]["translation"][2].asDouble());

				anyInfoWasRead = true;
			}

			if (jsonNodes[int(i)].isMember("rotation"))
			{
				newEntity.rotation = glm::vec4(jsonNodes[int(i)]["rotation"][0].asDouble(),
											   jsonNodes[int(i)]["rotation"][1].asDouble(),
											   jsonNodes[int(i)]["rotation"][2].asDouble(),
											   jsonNodes[int(i)]["rotation"][3].asDouble());

				anyInfoWasRead = true;
			}

			if (jsonNodes[int(i)].isMember("scale"))
			{
				newEntity.scale = glm::vec3(jsonNodes[int(i)]["scale"][0].asDouble(),
											jsonNodes[int(i)]["scale"][1].asDouble(),
											jsonNodes[int(i)]["scale"][2].asDouble());

				anyInfoWasRead = true;
			}

			if (jsonNodes[int(i)].isMember("children"))
			{
				Json::Value jsonChildrens = root["children"];
				for (size_t j = 0; j < jsonChildrens.size(); j++)
				{

				}
			}

			if (anyInfoWasRead)
				entities.push_back(newEntity);
		}
	}
	int y = 0;
	y++;
	

}

bool FEglTFLoader::loadPositions(glTFPrimitive& primitive)
{
	if (accessors.size() <= primitive.attributes["POSITION"])
	{
		LOG.add("primitive.attributes[\"POSITION\"] is out of bounds of accessors.size() function FEglTFLoader::loadPositions.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int accessorID = primitive.attributes["POSITION"];
	glTFAccessor currentAccessor = accessors[accessorID];
	glTFBufferView currentBufferView = bufferViews[currentAccessor.bufferView];
	glTFBuffer currentBuffer = buffers[currentBufferView.buffer];

	if (currentAccessor.componentType != 5126)
	{
		LOG.add("componentType is not float in function FEglTFLoader::loadPositions.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int byteOffset = 0;
	if (currentBufferView.byteOffset > 0)
		byteOffset = currentBufferView.byteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	primitive.rawData.positions.resize(currentAccessor.count * 3);
	memcpy_s(primitive.rawData.positions.data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);

	return true;
}

bool FEglTFLoader::loadNomals(glTFPrimitive& primitive)
{
	if (accessors.size() <= primitive.attributes["NORMAL"])
	{
		LOG.add("primitive.attributes[\"NORMAL\"] is out of bounds of accessors.size() function FEglTFLoader::loadNomals.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int accessorID = primitive.attributes["NORMAL"];
	glTFAccessor currentAccessor = accessors[accessorID];
	glTFBufferView currentBufferView = bufferViews[currentAccessor.bufferView];
	glTFBuffer currentBuffer = buffers[currentBufferView.buffer];

	if (currentAccessor.componentType != 5126)
	{
		LOG.add("componentType is not float in function FEglTFLoader::loadNomals.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int byteOffset = 0;
	if (currentBufferView.byteOffset > 0)
		byteOffset = currentBufferView.byteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	primitive.rawData.normals.resize(currentAccessor.count * 3);
	memcpy_s(primitive.rawData.normals.data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);

	return true;
}

bool FEglTFLoader::loadTangents(glTFPrimitive& primitive)
{
	if (accessors.size() <= primitive.attributes["TANGENT"])
	{
		LOG.add("primitive.attributes[\"TANGENT\"] is out of bounds of accessors.size() function FEglTFLoader::loadTangents.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int accessorID = primitive.attributes["TANGENT"];
	glTFAccessor currentAccessor = accessors[accessorID];
	glTFBufferView currentBufferView = bufferViews[currentAccessor.bufferView];
	glTFBuffer currentBuffer = buffers[currentBufferView.buffer];

	if (currentAccessor.componentType != 5126)
	{
		LOG.add("componentType is not float in function FEglTFLoader::loadTangents.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int byteOffset = 0;
	if (currentBufferView.byteOffset > 0)
		byteOffset = currentBufferView.byteOffset;

	if (currentAccessor.type == "VEC3")
	{
		primitive.rawData.tangents.resize(currentAccessor.count * 3);
		memcpy_s(primitive.rawData.tangents.data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);
	}
	else if (currentAccessor.type == "VEC4")
	{
		std::vector<float> tempBuffer;
		tempBuffer.resize(currentAccessor.count * 4);
		memcpy_s(tempBuffer.data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);

		int iteration = 0;
		for (size_t i = 0; i < tempBuffer.size(); i++)
		{
			iteration++;
			if (iteration == 4)
			{
				iteration = 0;
				continue;
			}
			primitive.rawData.tangents.push_back(tempBuffer[i]);
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool FEglTFLoader::loadUV(glTFPrimitive& primitive)
{
	for (size_t i = 0; i < 16; i++)
	{
		std::string attributeName = std::string("TEXCOORD_") + std::to_string(i);
		if (primitive.attributes.find(attributeName) != primitive.attributes.end())
		{
			if (accessors.size() <= primitive.attributes[attributeName])
			{
				LOG.add("primitive.attributes[" + attributeName + "] is out of bounds of accessors.size() function FEglTFLoader::loadUV.", FE_LOG_ERROR, FE_LOG_LOADING);
				//return false;
			}

			int accessorID = primitive.attributes[attributeName];
			glTFAccessor currentAccessor = accessors[accessorID];
			glTFBufferView currentBufferView = bufferViews[currentAccessor.bufferView];
			glTFBuffer currentBuffer = buffers[currentBufferView.buffer];

			if (currentAccessor.componentType != 5126)
			{
				LOG.add("componentType is not float in function FEglTFLoader::loadUV.", FE_LOG_ERROR, FE_LOG_LOADING);
				//return false;
			}

			int byteOffset = 0;
			if (currentBufferView.byteOffset > 0)
				byteOffset = currentBufferView.byteOffset;

			primitive.rawData.UVs.resize(primitive.rawData.UVs.size() + 1);
			// It is vec2 so size should be currentAccessor.count * 2.
			primitive.rawData.UVs[i].resize(currentAccessor.count * 2);
			memcpy_s(primitive.rawData.UVs[i].data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);
		}
	}

	return true;
}

bool FEglTFLoader::loadIndices(glTFPrimitive& primitive)
{
	if (accessors.size() <= primitive.indices)
	{
		LOG.add("primitive.indices is out of bounds of accessors.size() function FEglTFLoader::loadIndices.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int accessorID = primitive.indices;
	glTFAccessor currentAccessor = accessors[accessorID];
	glTFBufferView currentBufferView = bufferViews[currentAccessor.bufferView];
	glTFBuffer currentBuffer = buffers[currentBufferView.buffer];

	if (currentAccessor.componentType != 5123)
	{
		LOG.add("componentType is not unsigned short in function FEglTFLoader::loadIndices.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	int byteOffset = 0;
	if (currentBufferView.byteOffset > 0)
		byteOffset = currentBufferView.byteOffset;

	std::vector<unsigned short> tempBuffer;
	tempBuffer.resize(currentAccessor.count);
	memcpy_s(tempBuffer.data(), currentBufferView.byteLength, reinterpret_cast<void*>(currentBuffer.rawData + byteOffset), currentBufferView.byteLength);

	for (size_t i = 0; i < tempBuffer.size(); i++)
	{
		primitive.rawData.indices.push_back((int)tempBuffer[i]);
	}

	return true;
}

bool FEglTFLoader::loadMeshRawData(glTFPrimitive& primitive)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end())
	{
		LOG.add("primitive.attributes does not contain \"POSITION\" in function FEglTFLoader::loadMeshRawData.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	if (primitive.attributes.find("NORMAL") == primitive.attributes.end())
	{
		LOG.add("primitive.attributes does not contain \"NORMAL\" in function FEglTFLoader::loadMeshRawData.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	if (primitive.attributes.find("TANGENT") == primitive.attributes.end())
	{
		LOG.add("primitive.attributes does not contain \"TANGENT\" in function FEglTFLoader::loadMeshRawData.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	if (primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end())
	{
		LOG.add("primitive.attributes does not contain \"TEXCOORD_0\" in function FEglTFLoader::loadMeshRawData.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	if (primitive.indices == -1)
	{
		LOG.add("primitive.indices is -1 in function FEglTFLoader::loadMeshRawData.", FE_LOG_ERROR, FE_LOG_LOADING);
		return false;
	}

	if (!loadPositions(primitive))
		return false;

	if (!loadNomals(primitive))
		return false;

	if (!loadTangents(primitive))
		return false;

	if (!loadUV(primitive))
		return false;

	if (!loadIndices(primitive))
		return false;

	return true;
}