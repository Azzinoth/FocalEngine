#include "FEGLTFLoader.h"
using namespace FocalEngine;

FEGLTFLoader* FEGLTFLoader::Instance = nullptr;

FEGLTFLoader::FEGLTFLoader()
{
    
}

FEGLTFLoader::~FEGLTFLoader()
{
   
}

void FEGLTFLoader::Clear()
{
	Buffers.clear();
	BufferViews.clear();
	Accessors.clear();
	Meshes.clear();
	Nodes.clear();

	Images.clear();
	Textures.clear();
	Materials.clear();
}

void FEGLTFLoader::LoadPrimitive(Json::Value JsonPrimitive, GLTFPrimitive& NewPrimitive)
{
	if (JsonPrimitive.isMember("attributes"))
	{
		if (JsonPrimitive["attributes"].isMember("POSITION"))
			NewPrimitive.Attributes["POSITION"] = JsonPrimitive["attributes"]["POSITION"].asInt();

		if (JsonPrimitive["attributes"].isMember("NORMAL"))
			NewPrimitive.Attributes["NORMAL"] = JsonPrimitive["attributes"]["NORMAL"].asInt();

		if (JsonPrimitive["attributes"].isMember("TANGENT"))
			NewPrimitive.Attributes["TANGENT"] = JsonPrimitive["attributes"]["TANGENT"].asInt();

		std::string AttributeName;

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("TEXCOORD_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("COLOR_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("JOINTS_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("WEIGHTS_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}
	}

	if (JsonPrimitive.isMember("indices"))
		NewPrimitive.Indices = JsonPrimitive["indices"].asInt();

	if (JsonPrimitive.isMember("material"))
		NewPrimitive.Material = JsonPrimitive["material"].asInt();

	LoadMeshRawData(NewPrimitive);
}

GLTFMaterialTexture FEGLTFLoader::LoadMaterialTexture(Json::Value JsonTextureNode)
{
	GLTFMaterialTexture TempTexture;

	if (JsonTextureNode.isMember("index"))
	{
		TempTexture.Index = JsonTextureNode["index"].asInt();
	}

	if (JsonTextureNode.isMember("texCoord"))
	{
		TempTexture.TexCoord = JsonTextureNode["texCoord"].asInt();
	}

	if (JsonTextureNode.isMember("scale"))
	{
		TempTexture.TexCoord = JsonTextureNode["scale"].asInt();
	}

	return TempTexture;
}

void FEGLTFLoader::Load(const char* FileName)
{
	Clear();

	std::ifstream GLTFFile;
	GLTFFile.open(FileName);

	std::string FileData((std::istreambuf_iterator<char>(GLTFFile)), std::istreambuf_iterator<char>());
	GLTFFile.close();

	Json::Value Root;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Err))
		return;

	std::string Directory = FILE_SYSTEM.GetDirectoryPath(FileName);

	if (Root.isMember("buffers"))
	{
		Json::Value JsonBuffers = Root["buffers"];
		for (size_t i = 0; i < JsonBuffers.size(); i++)
		{
			GLTFBuffer CurrentBuffer;

			if (JsonBuffers[static_cast<int>(i)].isMember("uri"))
				CurrentBuffer.Uri = JsonBuffers[static_cast<int>(i)]["uri"].asString();

			std::fstream File;
			File.open(Directory + "\\" + CurrentBuffer.Uri, std::ios::in | std::ios::binary | std::ios::ate);
			std::streamsize FileSize = File.tellg();
			if (FileSize < 0)
			{
				LOG.Add(std::string("can't load buffer from: ") + CurrentBuffer.Uri + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
			else
			{
				File.seekg(0, std::ios::beg);
				CurrentBuffer.RawData = new char[static_cast<int>(FileSize)];
				File.read(CurrentBuffer.RawData, FileSize);
			}
			File.close();

			if (JsonBuffers[static_cast<int>(i)].isMember("byteLength"))
				CurrentBuffer.ByteLength = JsonBuffers[static_cast<int>(i)]["byteLength"].asInt();

			if (CurrentBuffer.ByteLength != FileSize)
				LOG.Add("byteLength and fileSize is not equal in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);

			Buffers.push_back(CurrentBuffer);
		}
	}

	if (Root.isMember("bufferViews"))
	{
		Json::Value JsonBufferViews = Root["bufferViews"];
		for (size_t i = 0; i < JsonBufferViews.size(); i++)
		{
			GLTFBufferView CurrentBufferView;

			if (JsonBufferViews[static_cast<int>(i)].isMember("buffer"))
				CurrentBufferView.Buffer = JsonBufferViews[static_cast<int>(i)]["buffer"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("byteLength"))
				CurrentBufferView.ByteLength = JsonBufferViews[static_cast<int>(i)]["byteLength"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("byteOffset"))
				CurrentBufferView.ByteOffset = JsonBufferViews[static_cast<int>(i)]["byteOffset"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("target"))
				CurrentBufferView.Target = JsonBufferViews[static_cast<int>(i)]["target"].asInt();

			BufferViews.push_back(CurrentBufferView);
		}
	}

	if (Root.isMember("accessors"))
	{
		Json::Value JsonAccessors = Root["accessors"];
		for (size_t i = 0; i < JsonAccessors.size(); i++)
		{
			GLTFAccessor CurrentAccessor;

			if (JsonAccessors[static_cast<int>(i)].isMember("bufferView"))
				CurrentAccessor.BufferView = JsonAccessors[static_cast<int>(i)]["bufferView"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("byteOffset"))
				CurrentAccessor.ByteOffset = JsonAccessors[static_cast<int>(i)]["byteOffset"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("componentType"))
				CurrentAccessor.ComponentType = JsonAccessors[static_cast<int>(i)]["componentType"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("count"))
				CurrentAccessor.Count = JsonAccessors[static_cast<int>(i)]["count"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("type"))
				CurrentAccessor.Type = JsonAccessors[static_cast<int>(i)]["type"].asString();

			Accessors.push_back(CurrentAccessor);
		}
	}

	if (Root.isMember("meshes"))
	{
		Json::Value JsonMeshes = Root["meshes"];
		for (size_t i = 0; i < JsonMeshes.size(); i++)
		{
			GLTFMesh CurrentMesh;
			if (JsonMeshes[static_cast<int>(i)].isMember("name"))
				CurrentMesh.Name = JsonMeshes[static_cast<int>(i)]["name"].asCString();

			if (JsonMeshes[static_cast<int>(i)].isMember("primitives"))
			{
				Json::Value JsonPrimitives = JsonMeshes[static_cast<int>(i)]["primitives"];
				for (size_t j = 0; j < JsonPrimitives.size(); j++)
				{
					GLTFPrimitive CurrentPrimitive;
					LoadPrimitive(JsonPrimitives[static_cast<int>(j)], CurrentPrimitive);
					CurrentMesh.Primitives.push_back(CurrentPrimitive);
				}
			}

			Meshes.push_back(CurrentMesh);
		}
	}

	if (Root.isMember("images"))
	{
		Json::Value JsonImages = Root["images"];
		for (size_t i = 0; i < JsonImages.size(); i++)
		{
			if (JsonImages[static_cast<int>(i)].isMember("uri"))
			{
				std::string ImageUri = (Directory + JsonImages[static_cast<int>(i)]["uri"].asCString());
				Images.push_back(ImageUri);
			}
		}
	}

	if (Root.isMember("textures"))
	{
		Json::Value JsonTextures = Root["textures"];
		for (size_t i = 0; i < JsonTextures.size(); i++)
		{
			if (JsonTextures[static_cast<int>(i)].isMember("source"))
			{
				int ImageIndex = JsonTextures[static_cast<int>(i)]["source"].asInt();
				if (ImageIndex < Images.size())
				{
					std::string TextureFile = Images[ImageIndex];
					Textures.push_back(TextureFile);
				}
			}
		}
	}

	if (Root.isMember("materials"))
	{
		Json::Value JsonMaterials = Root["materials"];
		for (size_t i = 0; i < JsonMaterials.size(); i++)
		{
			GLTFMaterial NewMaterial;
			NewMaterial.Name = JsonMaterials[static_cast<int>(i)]["name"].asCString();

			if (JsonMaterials[static_cast<int>(i)].isMember("pbrMetallicRoughness"))
			{
				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorFactor"))
				{
					NewMaterial.BaseColor = glm::vec4(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][0].asDouble(),
													  JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][1].asDouble(),
													  JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][2].asDouble(),
													  JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][3].asDouble());
				}

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorTexture"))
				{
					NewMaterial.BaseColorTexture = LoadMaterialTexture(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorTexture"]);
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

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("metallicRoughnessTexture"))
				{
					NewMaterial.MetallicRoughnessTexture = LoadMaterialTexture(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]);
					//if (jsonMaterials[int(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"].isMember("index"))
					//{
					//	newMaterial.metallicRoughnessTexture = jsonMaterials[int(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"].asInt();
					//}
				}
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("normalTexture"))
			{
				NewMaterial.NormalTexture = LoadMaterialTexture(JsonMaterials[static_cast<int>(i)]["normalTexture"]);
				/*if (jsonMaterials[int(i)]["normalTexture"].isMember("index"))
				{
					newMaterial.normalTexture = jsonMaterials[int(i)]["normalTexture"]["index"].asInt();
				}*/
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("occlusionTexture"))
			{
				NewMaterial.OcclusionTexture = LoadMaterialTexture(JsonMaterials[static_cast<int>(i)]["occlusionTexture"]);
				/*if (jsonMaterials[int(i)]["occlusionTexture"].isMember("index"))
				{
					newMaterial.occlusionTexture = jsonMaterials[int(i)]["occlusionTexture"]["index"].asInt();
				}*/
			}

			Materials.push_back(NewMaterial);
		}
	}

	if (Root.isMember("nodes"))
	{
		Json::Value JsonNodes = Root["nodes"];
		for (size_t i = 0; i < JsonNodes.size(); i++)
		{
			GLTFNodes NewNode;
			bool bAnyInfoWasRead = false;

			if (JsonNodes[static_cast<int>(i)].isMember("name"))
			{
				NewNode.Name = JsonNodes[static_cast<int>(i)]["name"].asCString();
				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("mesh"))
			{
				NewNode.Mesh = JsonNodes[static_cast<int>(i)]["mesh"].asInt();
				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("translation"))
			{
				NewNode.Translation = glm::vec3(JsonNodes[static_cast<int>(i)]["translation"][0].asDouble(),
												JsonNodes[static_cast<int>(i)]["translation"][1].asDouble(),
											    JsonNodes[static_cast<int>(i)]["translation"][2].asDouble());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("rotation"))
			{
				// glm::quat takes w,x,y,z. But gltf stores x,y,z,w.
				NewNode.Rotation = glm::quat(JsonNodes[static_cast<int>(i)]["rotation"][3].asDouble(),
											 JsonNodes[static_cast<int>(i)]["rotation"][0].asDouble(),
											 JsonNodes[static_cast<int>(i)]["rotation"][1].asDouble(),
											 JsonNodes[static_cast<int>(i)]["rotation"][2].asDouble());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("scale"))
			{
				NewNode.Scale = glm::vec3(JsonNodes[static_cast<int>(i)]["scale"][0].asDouble(),
										  JsonNodes[static_cast<int>(i)]["scale"][1].asDouble(),
										  JsonNodes[static_cast<int>(i)]["scale"][2].asDouble());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("children"))
			{
				Json::Value JsonChildrens = Root["children"];
				for (size_t j = 0; j < JsonChildrens.size(); j++)
				{

				}
			}

			if (bAnyInfoWasRead)
				Nodes.push_back(NewNode);
		}
	}
}

bool FEGLTFLoader::LoadPositions(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["POSITION"])
	{
		LOG.Add("primitive.attributes[\"POSITION\"] is out of bounds of accessors.size() function FEGLTFLoader::loadPositions.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["POSITION"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("componentType is not float in function FEGLTFLoader::loadPositions.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	if (CurrentBufferView.ByteOffset > 0)
		ByteOffset = CurrentBufferView.ByteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	Primitive.RawData.Positions.resize(CurrentAccessor.Count * 3);
	memcpy_s(Primitive.RawData.Positions.data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);

	return true;
}

bool FEGLTFLoader::LoadNomals(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["NORMAL"])
	{
		LOG.Add("primitive.attributes[\"NORMAL\"] is out of bounds of accessors.size() function FEGLTFLoader::loadNomals.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["NORMAL"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("componentType is not float in function FEGLTFLoader::loadNomals.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	if (CurrentBufferView.ByteOffset > 0)
		ByteOffset = CurrentBufferView.ByteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	Primitive.RawData.Normals.resize(CurrentAccessor.Count * 3);
	memcpy_s(Primitive.RawData.Normals.data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);

	return true;
}

bool FEGLTFLoader::LoadTangents(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["TANGENT"])
	{
		LOG.Add("primitive.attributes[\"TANGENT\"] is out of bounds of accessors.size() function FEGLTFLoader::loadTangents.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["TANGENT"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("componentType is not float in function FEGLTFLoader::loadTangents.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	if (CurrentBufferView.ByteOffset > 0)
		ByteOffset = CurrentBufferView.ByteOffset;

	if (CurrentAccessor.Type == "VEC3")
	{
		Primitive.RawData.Tangents.resize(CurrentAccessor.Count * 3);
		memcpy_s(Primitive.RawData.Tangents.data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);
	}
	else if (CurrentAccessor.Type == "VEC4")
	{
		std::vector<float> TempBuffer;
		TempBuffer.resize(CurrentAccessor.Count * 4);
		memcpy_s(TempBuffer.data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);

		int iteration = 0;
		for (size_t i = 0; i < TempBuffer.size(); i++)
		{
			iteration++;
			if (iteration == 4)
			{
				iteration = 0;
				continue;
			}
			Primitive.RawData.Tangents.push_back(TempBuffer[i]);
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool FEGLTFLoader::LoadUV(GLTFPrimitive& Primitive)
{
	for (size_t i = 0; i < 16; i++)
	{
		std::string AttributeName = std::string("TEXCOORD_") + std::to_string(i);
		if (Primitive.Attributes.find(AttributeName) != Primitive.Attributes.end())
		{
			if (Accessors.size() <= Primitive.Attributes[AttributeName])
			{
				LOG.Add("primitive.attributes[" + AttributeName + "] is out of bounds of accessors.size() function FEGLTFLoader::loadUV.", "FE_LOG_LOADING", FE_LOG_ERROR);
				//return false;
			}

			const int AccessorID = Primitive.Attributes[AttributeName];
			const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
			const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
			const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

			if (CurrentAccessor.ComponentType != 5126)
			{
				LOG.Add("componentType is not float in function FEGLTFLoader::loadUV.", "FE_LOG_LOADING", FE_LOG_ERROR);
				//return false;
			}

			int ByteOffset = 0;
			if (CurrentBufferView.ByteOffset > 0)
				ByteOffset = CurrentBufferView.ByteOffset;

			Primitive.RawData.UVs.resize(Primitive.RawData.UVs.size() + 1);
			// It is vec2 so size should be currentAccessor.count * 2.
			Primitive.RawData.UVs[i].resize(CurrentAccessor.Count * 2);
			memcpy_s(Primitive.RawData.UVs[i].data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);
		}
	}

	return true;
}

bool FEGLTFLoader::LoadIndices(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Indices)
	{
		LOG.Add("primitive.indices is out of bounds of accessors.size() function FEGLTFLoader::loadIndices.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Indices;
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5123)
	{
		LOG.Add("componentType is not unsigned short in function FEGLTFLoader::loadIndices.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	if (CurrentBufferView.ByteOffset > 0)
		ByteOffset = CurrentBufferView.ByteOffset;

	std::vector<unsigned short> TempBuffer;
	TempBuffer.resize(CurrentAccessor.Count);
	memcpy_s(TempBuffer.data(), CurrentBufferView.ByteLength, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), CurrentBufferView.ByteLength);

	for (size_t i = 0; i < TempBuffer.size(); i++)
	{
		Primitive.RawData.Indices.push_back((int)TempBuffer[i]);
	}

	return true;
}

bool FEGLTFLoader::LoadMeshRawData(GLTFPrimitive& Primitive)
{
	if (Primitive.Attributes.find("POSITION") == Primitive.Attributes.end())
	{
		LOG.Add("primitive.attributes does not contain \"POSITION\" in function FEGLTFLoader::loadMeshRawData.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (Primitive.Attributes.find("NORMAL") == Primitive.Attributes.end())
	{
		LOG.Add("primitive.attributes does not contain \"NORMAL\" in function FEGLTFLoader::loadMeshRawData.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (Primitive.Attributes.find("TANGENT") == Primitive.Attributes.end())
	{
		LOG.Add("primitive.attributes does not contain \"TANGENT\" in function FEGLTFLoader::loadMeshRawData.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (Primitive.Attributes.find("TEXCOORD_0") == Primitive.Attributes.end())
	{
		LOG.Add("primitive.attributes does not contain \"TEXCOORD_0\" in function FEGLTFLoader::loadMeshRawData.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (Primitive.Indices == -1)
	{
		LOG.Add("primitive.indices is -1 in function FEGLTFLoader::loadMeshRawData.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (!LoadPositions(Primitive))
		return false;

	if (!LoadNomals(Primitive))
		return false;

	if (!LoadTangents(Primitive))
		return false;

	if (!LoadUV(Primitive))
		return false;

	if (!LoadIndices(Primitive))
		return false;

	return true;
}