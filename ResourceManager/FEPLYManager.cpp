#include "FEPLYManager.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPLYManager()
{
	return FEPLYManager::GetInstancePointer();
}
#endif

FEPLYManager::FEPLYManager()
{
	
}

FEPLYManager::~FEPLYManager()
{

}

bool FEPLYManager::IsPLYFile(std::string FilePath)
{
	if (FilePath.empty())
	{
		LOG.Add(std::string("Empty file path in function FEPLYParser::IsPLYFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	std::ifstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Could not open file in function FEPLYParser::IsPLYFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	std::string CurrentLine;
	std::getline(File, CurrentLine);
	if (CurrentLine != "ply")
	{
		LOG.Add(std::string("File is not a PLY file in function FEPLYParser::IsPLYFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	return true;
}

FEPLYHeader* FEPLYManager::ParseHeader(std::ifstream& File)
{
	FEPLYHeader* Header = new FEPLYHeader();

	std::string Line;
	std::getline(File, Line);
	// Trim any whitespace or control characters
	Line.erase(Line.find_last_not_of(" \n\r\t") + 1);
	Line.erase(0, Line.find_first_not_of(" \n\r\t"));

	if (Line != "ply")
	{
		LOG.Add(std::string("File is not a PLY file in function FEPLYParser::ParseHeader."), "FE_LOG_LOADING", FE_LOG_ERROR);
		delete Header;
		return nullptr;
	}

	// Parse format
	std::getline(File, Line);
	if (Line.find("format ascii") != std::string::npos)
	{
		Header->StorageType = PLYFileType::ASCII;
	}
	else if (Line.find("format binary_little_endian") != std::string::npos)
	{
		Header->StorageType = PLYFileType::BINARY_LITTLE_ENDIAN;
	}
	else if (Line.find("format binary_big_endian") != std::string::npos)
	{
		Header->StorageType = PLYFileType::BINARY_BIG_ENDIAN;
	}
	else if (Line.find("format binary") != std::string::npos)
	{
		Header->StorageType = PLYFileType::BINARY_LITTLE_ENDIAN;
	}
	else
	{
		delete Header;
		return nullptr;
	}

	bool bEndOfHeaderFound = false;
	while (Line != "end_header" && !File.eof())
	{
		std::getline(File, Line);

		if (Line.find("comment") != std::string::npos)
		{
			// Find position of "comment" and add 7 to skip "comment" and the space after it
			size_t CommentPosition = Line.find("comment");
			if (CommentPosition != std::string::npos && CommentPosition + 7 < Line.length())
			{
				// Extract everything after "comment "
				Header->Comments.push_back(Line.substr(CommentPosition + 8));
			}
			else
			{
				// In case there's no content after "comment"
				LOG.Add("Empty comment in PLY file.", "FE_LOG_LOADING", FE_LOG_WARNING);
			}
		}
		else if (Line.find("element") != std::string::npos)
		{
			PLYElementSchema NewElement;

			std::istringstream StringStream(Line);
			std::string Keyword;

			// Extract the keyword "element"
			StringStream >> Keyword;

			// Extract the element name and count
			if (!(StringStream >> NewElement.Name >> NewElement.Count))
			{
				delete Header;
				LOG.Add(std::string("Could not parse element in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
				return nullptr;
			}

			Header->ElementSchemas.push_back(NewElement);
		}
		else if (Line.find("property") != std::string::npos)
		{
			if (Header->ElementSchemas.empty())
			{
				delete Header;
				LOG.Add(std::string("Property found before element in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
				return nullptr;
			}

			PLYPropertyDefinition NewProperty;

			std::istringstream StringStream(Line);
			std::string Keyword;

			// Extract the keyword "property"
			StringStream >> Keyword;

			// Check if it's a list property
			std::string TypeOrList;
			StringStream >> TypeOrList;

			if (TypeOrList == "list")
			{
				// Format: "property list <size_type> <value_type> <name>"
				NewProperty.bIsList = true;

				std::string SizeType, ValueType;
				if (!(StringStream >> SizeType >> ValueType >> NewProperty.Name))
				{
					delete Header;
					LOG.Add(std::string("Could not parse list property in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
					return nullptr;
				}

				NewProperty.SizeType = PLYPropertyDefinition::GetType(SizeType);
				NewProperty.Type = PLYPropertyDefinition::GetType(ValueType);

				if (NewProperty.SizeType == PLYPropertyType::UNKNOWN || NewProperty.Type == PLYPropertyType::UNKNOWN)
				{
					delete Header;
					LOG.Add(std::string("Unknown property type in list property in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
					return nullptr;
				}
			}
			else
			{
				// Regular property: "property <type> <name>"
				NewProperty.bIsList = false;

				// We've already read the type into TypeOrList
				std::string TemporaryType = TypeOrList;

				if (!(StringStream >> NewProperty.Name))
				{
					delete Header;
					LOG.Add(std::string("Could not parse property in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
					return nullptr;
				}

				NewProperty.Type = PLYPropertyDefinition::GetType(TemporaryType);
				if (NewProperty.Type == PLYPropertyType::UNKNOWN)
				{
					delete Header;
					LOG.Add(std::string("Unknown property type in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
					return nullptr;
				}
			}

			Header->ElementSchemas.back().PropertyDefinitions.push_back(NewProperty);
		}
		else if (Line.find("end_header") != std::string::npos)
		{
			bEndOfHeaderFound = true;
			break;
		}
		else
		{
			LOG.Add(std::string("Unknown keyword in PLY file."), "FE_LOG_LOADING", FE_LOG_WARNING);
		}
	}

	if (!bEndOfHeaderFound)
	{
		delete Header;
		LOG.Add(std::string("Missing end_header marker in PLY file."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}
	
	return Header;
}

PLYScalarValue FEPLYManager::ReadPropertyValue(std::ifstream& File, PLYPropertyType Type, std::istringstream* StringStream)
{
	PLYScalarValue Value;

	if (LoadedPLY == nullptr || LoadedPLY->Header == nullptr)
	{
		LOG.Add(std::string("No header loaded in function FEPLYParser::ReadPropertyValue."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return Value;
	}

	// Read the value according to its type
	switch (Type)
	{
		case PLYPropertyType::CHAR:
		{
			char TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(char));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				// Read as integer, then cast to unsigned char
				int TemporaryValue = 0;
				*(StringStream) >> TemporaryValue;
				TypedValue = static_cast<char>(TemporaryValue);
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::UCHAR:
		{
			unsigned char TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(unsigned char));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				// Read as integer, then cast to unsigned char
				int TemporaryValue = 0;
				*(StringStream) >> TemporaryValue;
				TypedValue = static_cast<unsigned char>(TemporaryValue);
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::SHORT:
		{
			short TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(short));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				// Read as integer, then cast to unsigned char
				int TemporaryValue = 0;
				*(StringStream) >> TemporaryValue;
				TypedValue = static_cast<short>(TemporaryValue);
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::USHORT:
		{
			unsigned short TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(unsigned short));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				// Read as integer, then cast to unsigned char
				int TemporaryValue = 0;
				*(StringStream) >> TemporaryValue;
				TypedValue = static_cast<unsigned short>(TemporaryValue);
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::INT:
		{
			int TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(int));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				*(StringStream) >> TypedValue;
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::UINT:
		{
			unsigned int TypedValue = 0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(unsigned int));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				*(StringStream) >> TypedValue;
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::FLOAT:
		{
			float TypedValue = 0.0f;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(float));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				*(StringStream) >> TypedValue;
			}

			Value = TypedValue;
			break;
		}
		case PLYPropertyType::DOUBLE:
		{
			double TypedValue = 0.0;
			if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
			{
				File.read(reinterpret_cast<char*>(&TypedValue), sizeof(double));
			}
			else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
			{
				*(StringStream) >> TypedValue;
			}

			Value = TypedValue;
			break;
		}
	}

	return Value;
}

bool FEPLYManager::ParseData(std::ifstream& File)
{
	if (LoadedPLY == nullptr || LoadedPLY->Header == nullptr)
	{
		LOG.Add(std::string("No header loaded in function FEPLYParser::ParseASCIIData."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	// Resize Elements to match the header
	LoadedPLY->Elements.resize(LoadedPLY->Header->ElementSchemas.size());

	// Process each element
	for (size_t ElementIndex = 0; ElementIndex < LoadedPLY->Header->ElementSchemas.size(); ElementIndex++)
	{
		// Set the element description
		LoadedPLY->Elements[ElementIndex].Description = LoadedPLY->Header->ElementSchemas[ElementIndex];
		const PLYElementSchema& ElementDescription = LoadedPLY->Elements[ElementIndex].Description;

		// Resize entries vector
		LoadedPLY->Elements[ElementIndex].Entries.resize(ElementDescription.Count);

		// Read each entry
		for (size_t EntryIndex = 0; EntryIndex < ElementDescription.Count; EntryIndex++)
		{
			std::string Line;
			if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
				std::getline(File, Line);

			std::istringstream StringStream(Line);

			// Resize properties vector
			LoadedPLY->Elements[ElementIndex].Entries[EntryIndex].PropertyValues.resize(ElementDescription.PropertyDefinitions.size());

			// Read each property
			for (size_t PropertyIndex = 0; PropertyIndex < ElementDescription.PropertyDefinitions.size(); PropertyIndex++)
			{
				const PLYPropertyDefinition& PropertyDescription = ElementDescription.PropertyDefinitions[PropertyIndex];

				// Read a scalar property
				if (!PropertyDescription.bIsList)
				{
					LoadedPLY->Elements[ElementIndex].Entries[EntryIndex].PropertyValues[PropertyIndex] = ReadPropertyValue(File, PropertyDescription.Type, &StringStream);
				}
				// Read a list property
				else 
				{
					PLYListValue List;
					PLYScalarValue ListLengthValue = ReadPropertyValue(File, PropertyDescription.SizeType, &StringStream);
					int ListLength = 0;

					if (std::holds_alternative<char>(ListLengthValue))
					{
						ListLength = std::get<char>(ListLengthValue);
					}
					else if (std::holds_alternative<unsigned char>(ListLengthValue))
					{
						ListLength = std::get<unsigned char>(ListLengthValue);
					}
					else if (std::holds_alternative<short>(ListLengthValue))
					{
						ListLength = std::get<short>(ListLengthValue);
					}
					else if (std::holds_alternative<unsigned short>(ListLengthValue))
					{
						ListLength = std::get<unsigned short>(ListLengthValue);
					}
					else if (std::holds_alternative<int>(ListLengthValue))
					{
						ListLength = std::get<int>(ListLengthValue);
					}
					else if (std::holds_alternative<unsigned int>(ListLengthValue))
					{
						ListLength = std::get<unsigned int>(ListLengthValue);
					}
					else if (std::holds_alternative<float>(ListLengthValue))
					{
						ListLength = static_cast<int>(std::get<float>(ListLengthValue));
					}
					else if (std::holds_alternative<double>(ListLengthValue))
					{
						ListLength = static_cast<int>(std::get<double>(ListLengthValue));
					}

					// Read each list element
					for (int i = 0; i < ListLength; i++)
					{
						List.push_back(ReadPropertyValue(File, PropertyDescription.Type, &StringStream));
					}

					LoadedPLY->Elements[ElementIndex].Entries[EntryIndex].PropertyValues[PropertyIndex] = List;
				}
			}
		}
	}

	return true;
}

FERawPLYData* FEPLYManager::ParseFile(std::string FilePath)
{
	if (LoadedPLY != nullptr)
		delete LoadedPLY;
	
	LoadedPLY = new FERawPLYData();
	if (FilePath.empty())
	{
		LOG.Add(std::string("Empty file path in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		delete LoadedPLY;
		return nullptr;
	}

	std::ifstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Could not open file with path: ") + FilePath + " in function FEPLYParser::ParseFile.", "FE_LOG_LOADING", FE_LOG_ERROR);
		delete LoadedPLY;
		return nullptr;
	}

	LoadedPLY->Header = ParseHeader(File);
	if (LoadedPLY->Header == nullptr)
	{
		LOG.Add(std::string("Could not parse header in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		delete LoadedPLY;
		return nullptr;
	}

	if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
	{
		if (!ParseData(File))
		{
			LOG.Add(std::string("Could not parse ASCII data in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
			delete LoadedPLY;
			return nullptr;
		}
	}
	else if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
	{
		if (!ParseData(File))
		{
			LOG.Add(std::string("Could not parse binary data in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
			delete LoadedPLY;
			return nullptr;
		}
	}
	else if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_BIG_ENDIAN)
	{
		LOG.Add(std::string("BINARY_BIG_ENDIAN PLY files are not supported in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}
	else
	{
		LOG.Add(std::string("Unknown PLY file type in function FEPLYParser::ParseFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	return LoadedPLY;
}

void FEPLYManager::WritePropertyValue(std::ofstream& File, const PLYScalarValue& Value)
{
	if (std::holds_alternative<char>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			char TypedValue = std::get<char>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(char));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << static_cast<int>(std::get<char>(Value)) << " ";
		}
	}
	else if (std::holds_alternative<unsigned char>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			unsigned char TypedValue = std::get<unsigned char>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(unsigned char));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << static_cast<int>(std::get<unsigned char>(Value)) << " ";
		}
	}
	else if (std::holds_alternative<short>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			short TypedValue = std::get<short>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(short));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<short>(Value) << " ";
		}
	}
	else if (std::holds_alternative<unsigned short>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			unsigned short TypedValue = std::get<unsigned short>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(unsigned short));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<unsigned short>(Value) << " ";
		}
	}
	else if (std::holds_alternative<int>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			int TypedValue = std::get<int>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(int));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<int>(Value) << " ";
		}
	}
	else if (std::holds_alternative<unsigned int>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			unsigned int TypedValue = std::get<unsigned int>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(unsigned int));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<unsigned int>(Value) << " ";
		}
	}
	else if (std::holds_alternative<float>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			float TypedValue = std::get<float>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(float));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<float>(Value) << " ";
		}
	}
	else if (std::holds_alternative<double>(Value))
	{
		if (LoadedPLY->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		{
			double TypedValue = std::get<double>(Value);
			File.write(reinterpret_cast<const char*>(&TypedValue), sizeof(double));
		}
		else if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
		{
			File << std::get<double>(Value) << " ";
		}
	}
}

bool FEPLYManager::SaveToPLY(const std::string& FilePath, FERawPLYData* Data, PLYFileType StorageType)
{
	if (Data == nullptr)
	{
		LOG.Add(std::string("No data to save in function FEPLYParser::SaveToPLY."), "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	if (Data->Header == nullptr)
	{
		LOG.Add(std::string("No header in data to save in function FEPLYParser::SaveToPLY."), "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	if (FilePath.empty())
	{
		LOG.Add(std::string("Empty file path in function FEPLYParser::SaveToPLY."), "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	std::ofstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Could not open file for writing PLY data: ") + FilePath, "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	if (StorageType != PLYFileType::UNKNOWN)
		Data->Header->StorageType = StorageType;

	if (Data->Header->StorageType == PLYFileType::UNKNOWN)
	{
		LOG.Add(std::string("Unknown storage type in function FEPLYParser::SaveToPLY."), "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	// Small hack to avoid passing the Data pointer to WritePropertyValue.
	LoadedPLY = Data;
	
	File << "ply" << std::endl;
	if (Data->Header->StorageType == PLYFileType::ASCII)
		File << "format ascii 1.0" << std::endl;
	else if (Data->Header->StorageType == PLYFileType::BINARY_LITTLE_ENDIAN)
		File << "format binary_little_endian 1.0" << std::endl;
	else if (Data->Header->StorageType == PLYFileType::BINARY_BIG_ENDIAN)
		File << "format binary_big_endian 1.0" << std::endl;

	for (const auto& Comment : Data->Header->Comments)
		File << "comment " << Comment << std::endl;
	
	for (const auto& Element : Data->Elements)
	{
		// Write element header
		File << "element " << Element.Description.Name << " " << Element.Description.Count << std::endl;

		// Write properties
		for (const auto& Property : Element.Description.PropertyDefinitions)
		{
			if (!Property.bIsList)
			{
				// Regular property
				File << "property ";
				// Convert property type to string
				File << PLYPropertyDefinition::GetTypeString(Property.Type);
				File << " " << Property.Name << std::endl;
			}
			else
			{
				// List property
				File << "property list ";

				// Convert size type to string
				File << PLYPropertyDefinition::GetTypeString(Property.SizeType);
				File << " ";

				// Convert value type to string
				File << PLYPropertyDefinition::GetTypeString(Property.Type);
				File << " " << Property.Name << std::endl;
			}
		}
	}

	// End of header
	File << "end_header" << std::endl;

	// Write data
	for (const auto& Element : Data->Elements)
	{
		for (const auto& Entry : Element.Entries)
		{
			for (size_t i = 0; i < Entry.PropertyValues.size(); i++)
			{
				const auto& PropertyData = Entry.PropertyValues[i];
				const auto& PropertyDescription = Element.Description.PropertyDefinitions[i];

				if (!PropertyDescription.bIsList)
				{
					// Regular property
					const PLYScalarValue& Value = std::get<PLYScalarValue>(PropertyData);
					WritePropertyValue(File, Value);
				}
				else
				{
					// List property
					const PLYListValue& List = std::get<PLYListValue>(PropertyData);

					// Write count
					PLYScalarValue ListLengthValue;
					
					if (PropertyDescription.SizeType == PLYPropertyType::CHAR)
					{
						ListLengthValue = static_cast<char>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::UCHAR)
					{
						ListLengthValue = static_cast<unsigned char>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::SHORT)
					{
						ListLengthValue = static_cast<short>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::USHORT)
					{
						ListLengthValue = static_cast<unsigned short>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::INT)
					{
						ListLengthValue = static_cast<int>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::UINT)
					{
						ListLengthValue = static_cast<unsigned int>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::FLOAT)
					{
						ListLengthValue = static_cast<float>(List.size());
					}
					else if (PropertyDescription.SizeType == PLYPropertyType::DOUBLE)
					{
						ListLengthValue = static_cast<double>(List.size());
					}

					WritePropertyValue(File, ListLengthValue);

					// Write values
					for (const auto& Value : List)
					{
						WritePropertyValue(File, Value);
					}
				}
			}

			if (LoadedPLY->Header->StorageType == PLYFileType::ASCII)
				File << std::endl;
		}
	}

	File.close();
	return true;
}