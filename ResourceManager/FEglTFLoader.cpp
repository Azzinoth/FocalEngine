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

	Images.clear();
	TextureSamplers.clear();
	Textures.clear();
	Materials.clear();

	Nodes.clear();
	Scenes.clear();
	Scene = -1;
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

GLTFTextureInfo FEGLTFLoader::LoadTextureInfo(Json::Value JsonTextureNode)
{
	GLTFTextureInfo TempTextureInfo;

	if (JsonTextureNode.isMember("index"))
	{
		TempTextureInfo.Index = JsonTextureNode["index"].asInt();
	}
	else
	{
		LOG.Add("index is not present in function FEGLTFLoader::LoadTextureInfo.", "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	if (JsonTextureNode.isMember("texCoord"))
	{
		TempTextureInfo.TexCoord = JsonTextureNode["texCoord"].asInt();
	}

	return TempTextureInfo;
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
			GLTFImage NewImage;

			if (JsonImages[static_cast<int>(i)].isMember("uri"))
				NewImage.Uri = JsonImages[static_cast<int>(i)]["uri"].asCString();

			if (JsonImages[static_cast<int>(i)].isMember("mimeType"))
				NewImage.MimeType = JsonImages[static_cast<int>(i)]["mimeType"].asCString();

			if (JsonImages[static_cast<int>(i)].isMember("bufferView"))
				NewImage.BufferView = JsonImages[static_cast<int>(i)]["bufferView"].asInt();

			if (JsonImages[static_cast<int>(i)].isMember("name"))
				NewImage.Name = JsonImages[static_cast<int>(i)]["name"].asCString();

			Images.push_back(NewImage);
		}
	}

	if (Root.isMember("textures"))
	{
		Json::Value JsonTextures = Root["textures"];
		for (size_t i = 0; i < JsonTextures.size(); i++)
		{
			GLTFTexture NewTexture;

			if (JsonTextures[static_cast<int>(i)].isMember("sampler"))
				NewTexture.Sampler = JsonTextures[static_cast<int>(i)]["sampler"].asInt();

			if (JsonTextures[static_cast<int>(i)].isMember("source"))
				NewTexture.Source = JsonTextures[static_cast<int>(i)]["source"].asInt();

			if (JsonTextures[static_cast<int>(i)].isMember("name"))
				NewTexture.Name = JsonTextures[static_cast<int>(i)]["name"].asCString();

			Textures.push_back(NewTexture);
		}
	}

	if (Root.isMember("samplers"))
	{
		Json::Value JsonSamplers = Root["samplers"];
		for (size_t i = 0; i < JsonSamplers.size(); i++)
		{
			GLTFTextureSampler NewSampler;

			if (JsonSamplers[static_cast<int>(i)].isMember("magFilter"))
				NewSampler.MagFilter = JsonSamplers[static_cast<int>(i)]["magFilter"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("minFilter"))
				NewSampler.MinFilter = JsonSamplers[static_cast<int>(i)]["minFilter"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("wrapS"))
				NewSampler.WrapS = JsonSamplers[static_cast<int>(i)]["wrapS"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("wrapT"))
				NewSampler.WrapT = JsonSamplers[static_cast<int>(i)]["wrapT"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("name"))
				NewSampler.Name = JsonSamplers[static_cast<int>(i)]["name"].asCString();

			TextureSamplers.push_back(NewSampler);
		}
	}

	if (Root.isMember("materials"))
	{
		Json::Value JsonMaterials = Root["materials"];
		for (size_t i = 0; i < JsonMaterials.size(); i++)
		{
			GLTFMaterial NewMaterial;

			if (JsonMaterials[static_cast<int>(i)].isMember("name"))
				NewMaterial.Name = JsonMaterials[static_cast<int>(i)]["name"].asCString();

			if (JsonMaterials[static_cast<int>(i)].isMember("pbrMetallicRoughness"))
			{
				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorFactor"))
				{
					NewMaterial.PBRMetallicRoughness.BaseColorFactor = glm::vec4(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][0].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][1].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][2].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][3].asDouble());
				}

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorTexture"))
					NewMaterial.PBRMetallicRoughness.BaseColorTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorTexture"]);
				

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("metallicFactor"))
					NewMaterial.PBRMetallicRoughness.MetallicFactor = static_cast<float>(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["metallicFactor"].asDouble());

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("roughnessFactor"))
					NewMaterial.PBRMetallicRoughness.RoughnessFactor = static_cast<float>(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["roughnessFactor"].asDouble());

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("metallicRoughnessTexture"))
					NewMaterial.PBRMetallicRoughness.MetallicRoughnessTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]);
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("normalTexture"))
			{
				if (JsonMaterials[int(i)]["normalTexture"].isMember("index"))
				{
					NewMaterial.NormalTexture.Index = JsonMaterials[int(i)]["normalTexture"]["index"].asInt();

					if (JsonMaterials[int(i)]["normalTexture"].isMember("texCoord"))
						NewMaterial.NormalTexture.TexCoord = JsonMaterials[int(i)]["normalTexture"]["texCoord"].asInt();

					if (JsonMaterials[int(i)]["normalTexture"].isMember("scale"))
						NewMaterial.NormalTexture.Scale = JsonMaterials[int(i)]["normalTexture"]["scale"].asInt();
				}
				else
				{
					LOG.Add("normalTexture.index is not present in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("occlusionTexture"))
			{
				if (JsonMaterials[int(i)]["occlusionTexture"].isMember("index"))
				{
					NewMaterial.OcclusionTexture.Index = JsonMaterials[int(i)]["occlusionTexture"]["index"].asInt();

					if (JsonMaterials[int(i)]["occlusionTexture"].isMember("texCoord"))
						NewMaterial.OcclusionTexture.TexCoord = JsonMaterials[int(i)]["occlusionTexture"]["texCoord"].asInt();

					if (JsonMaterials[int(i)]["occlusionTexture"].isMember("strength"))
						NewMaterial.OcclusionTexture.Strength = JsonMaterials[int(i)]["occlusionTexture"]["strength"].asInt();
				}
				else
				{
					LOG.Add("occlusionTexture.index is not present in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("emissiveTexture"))
				NewMaterial.EmissiveTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["emissiveTexture"]);
			
			if (JsonMaterials[static_cast<int>(i)].isMember("emissiveFactor"))
			{
				NewMaterial.EmissiveFactor = glm::vec3(JsonMaterials[static_cast<int>(i)]["emissiveFactor"][0].asDouble(),
													   JsonMaterials[static_cast<int>(i)]["emissiveFactor"][1].asDouble(),
													   JsonMaterials[static_cast<int>(i)]["emissiveFactor"][2].asDouble());
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("alphaMode"))
				NewMaterial.AlphaMode = JsonMaterials[static_cast<int>(i)]["alphaMode"].asCString();

			if (JsonMaterials[static_cast<int>(i)].isMember("alphaCutoff"))
				NewMaterial.AlphaCutoff = static_cast<float>(JsonMaterials[static_cast<int>(i)]["alphaCutoff"].asDouble());

			if (JsonMaterials[static_cast<int>(i)].isMember("doubleSided"))
				NewMaterial.bDoubleSided = JsonMaterials[static_cast<int>(i)]["doubleSided"].asBool();

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

			if (JsonNodes[static_cast<int>(i)].isMember("camera"))
			{
				int CameraIndex = JsonNodes[static_cast<int>(i)]["camera"].asInt();
				if (CameraIndex < 0)
				{
					LOG.Add("camera is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Camera = CameraIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("mesh"))
			{
				int MeshIndex = JsonNodes[static_cast<int>(i)]["mesh"].asInt();
				if (MeshIndex < 0)
				{
					LOG.Add("mesh is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Mesh = MeshIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("skin"))
			{
				int SkinIndex = JsonNodes[static_cast<int>(i)]["skin"].asInt();
				if (SkinIndex < 0)
				{
					LOG.Add("skin is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
					
				}
				else
				{
					NewNode.Skin = SkinIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("matrix"))
			{
				if (JsonNodes[static_cast<int>(i)]["matrix"].size() != 16)
				{
					LOG.Add("matrix size is not 16 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Matrix.resize(16);
					for (int j = 0; j < 16; j++)
					{
						NewNode.Matrix[j] = JsonNodes[static_cast<int>(i)]["matrix"][j].asFloat();
					}
					bAnyInfoWasRead = true;
				}
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
				NewNode.Rotation = glm::quat(JsonNodes[static_cast<int>(i)]["rotation"][3].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][0].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][1].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][2].asFloat());

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
				std::unordered_map<int, bool> ChildrensPresent;

				Json::Value JsonChildrens = JsonNodes[static_cast<int>(i)]["children"];
				for (size_t j = 0; j < JsonChildrens.size(); j++)
				{
					int NewChild = JsonChildrens[static_cast<int>(j)].asInt();
					// Each element in the array MUST be greater than or equal to 0.
					if (NewChild < 0)
					{
						LOG.Add("child is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					// Each element in the array MUST be unique.
					if (ChildrensPresent.find(NewChild) != ChildrensPresent.end())
					{
						LOG.Add("child is not unique in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					ChildrensPresent[NewChild] = true;
					NewNode.Children.push_back(NewChild);
				}
			}

			if (bAnyInfoWasRead)
				Nodes.push_back(NewNode);
		}
	}

	if (Root.isMember("scenes"))
	{
		Json::Value JsonScenes = Root["scenes"];
		for (size_t i = 0; i < JsonScenes.size(); i++)
		{
			GLTFScene NewScene;

			if (JsonScenes[static_cast<int>(i)].isMember("name"))
				NewScene.Name = JsonScenes[static_cast<int>(i)]["name"].asCString();

			if (JsonScenes[static_cast<int>(i)].isMember("nodes"))
			{
				Json::Value JsonNodeIndexes = JsonScenes[static_cast<int>(i)]["nodes"];
				for (size_t j = 0; j < JsonNodeIndexes.size(); j++)
				{
					int NewNodeIndex = JsonNodeIndexes[static_cast<int>(j)].asInt();
					// Each element in the array MUST be greater than or equal to 0.
					if (NewNodeIndex < 0)
					{
						LOG.Add("node is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					NewScene.Nodes.push_back(NewNodeIndex);
				}
			}

			Scenes.push_back(NewScene);
		}
	}

	if (Root.isMember("scene"))
	{
		int SceneIndex = Root["scene"].asInt();
		if (SceneIndex < 0)
		{
			LOG.Add("scene is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
		}
		else
		{
			Scene = Root["scene"].asInt();
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