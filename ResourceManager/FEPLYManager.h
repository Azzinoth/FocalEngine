#pragma once

#include "../SubSystems/FileSystem/FEFileSystem.h"
#include "../Core/Geometry/FEGeometry.h"
#include <variant>

namespace FocalEngine
{
	enum class PLYFileType
	{
		ASCII,
		BINARY_LITTLE_ENDIAN,
		BINARY_BIG_ENDIAN,
		UNKNOWN
	};

	enum class PLYPropertyType
	{
		CHAR,
		UCHAR,
		SHORT,
		USHORT,
		INT,
		UINT,
		FLOAT,
		DOUBLE,
		UNKNOWN
	};

	struct PLYPropertyDefinition
	{
		PLYPropertyType Type = PLYPropertyType::CHAR;
		std::string Name = "";
		bool bIsList = false;
		PLYPropertyType SizeType = PLYPropertyType::UNKNOWN;

		static PLYPropertyType GetType(std::string Type)
		{
			if (Type == "char")
			{
				return PLYPropertyType::CHAR;
			}
			else if (Type == "uchar")
			{
				return PLYPropertyType::UCHAR;
			}
			else if (Type == "short")
			{
				return PLYPropertyType::SHORT;
			}
			else if (Type == "ushort")
			{
				return PLYPropertyType::USHORT;
			}
			else if (Type == "int")
			{
				return PLYPropertyType::INT;
			}
			else if (Type == "uint")
			{
				return PLYPropertyType::UINT;
			}
			else if (Type == "float")
			{
				return PLYPropertyType::FLOAT;
			}
			else if (Type == "double")
			{
				return PLYPropertyType::DOUBLE;
			}

			return PLYPropertyType::UNKNOWN;
		}

		static std::string GetTypeString(PLYPropertyType Type)
		{
			switch (Type)
			{
				case PLYPropertyType::CHAR: return "char";
				case PLYPropertyType::UCHAR: return "uchar";
				case PLYPropertyType::SHORT: return "short";
				case PLYPropertyType::USHORT: return "ushort";
				case PLYPropertyType::INT: return "int";
				case PLYPropertyType::UINT: return "uint";
				case PLYPropertyType::FLOAT: return "float";
				case PLYPropertyType::DOUBLE: return "double";
				default: return "unknown";
			}
		}
	};

	struct PLYElementSchema
	{
		std::string Name = "";
		size_t Count = 0;

		std::vector<PLYPropertyDefinition> PropertyDefinitions;
	};

	struct FEPLYHeader
	{
		PLYFileType StorageType = PLYFileType::ASCII;

		std::vector<PLYElementSchema> ElementSchemas;
		std::vector<std::string> Comments;
	};

	// First, define a variant type that can hold different property types
	using PLYScalarValue = std::variant<char, unsigned char, short, unsigned short, int, unsigned int, float, double>;

	// For list properties, we need a vector of values
	using PLYListValue = std::vector<PLYScalarValue>;

	// A single property can be either a single value or a list
	using PLYPropertyData = std::variant<PLYScalarValue, PLYListValue>;

	// Define a struct for a single element's entry (e.g., one vertex or one face)
	struct PLYElementEntry
	{
		std::vector<PLYPropertyData> PropertyValues;
	};

	// Define a struct for all entries of a single element type
	struct PLYElementData
	{
		PLYElementSchema Description;
		std::vector<PLYElementEntry> Entries;
	};

	struct FERawPLYData
	{
		FEPLYHeader* Header = nullptr;
		std::vector<PLYElementData> Elements;

		~FERawPLYData()
		{
			delete Header;
		}
	};

	class FEPLYManager
	{
		friend class FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEPLYManager)

		bool IsPLYFile(std::string FilePath);

		FERawPLYData* ParseFile(std::string FilePath);
		bool SaveToPLY(const std::string& FilePath, FERawPLYData* Data, PLYFileType StorageType = PLYFileType::UNKNOWN);
	private:
		SINGLETON_PRIVATE_PART(FEPLYManager)
			
		FERawPLYData* LoadedPLY = nullptr;
		FEPLYHeader* ParseHeader(std::ifstream& File);
		bool ParseData(std::ifstream& File);

		PLYScalarValue ReadPropertyValue(std::ifstream& File, PLYPropertyType Type, std::istringstream* StringStream);
		void WritePropertyValue(std::ofstream& File, const PLYScalarValue& Value);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPLYManager();
#define PLY_MANAGER (*static_cast<FEPLYManager*>(GetPLYManager()))
#else
#define PLY_MANAGER FEPLYManager::GetInstance()
#endif
}