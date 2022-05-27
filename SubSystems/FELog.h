#pragma once

#include "FECoreIncludes.h"

namespace FocalEngine
{
	enum LOG_SEVERITY
	{
		FE_LOG_INFO = 0,
		FE_LOG_DEBUG = 1,
		FE_LOG_WARNING = 2,
		FE_LOG_ERROR = 3,
		FE_LOG_FATAL_ERROR = 4
	};

	enum LOG_CHANNEL
	{
		FE_LOG_GENERAL = 0,
		FE_LOG_LOADING = 1,
		FE_LOG_SAVING = 2,
		FE_LOG_RENDERING = 3
	};

	struct LogItem
	{
		std::string text;
		LOG_SEVERITY severity;
		int count;
		LOG_CHANNEL channel;
		long timeStamp;

		LogItem();
		~LogItem();

		bool operator==(const LogItem& other) const
		{
			return (text == other.text && severity == other.severity);
		}
	};

	class FELOG
	{
	public:
		SINGLETON_PUBLIC_PART(FELOG)

		void add(std::string text, LOG_SEVERITY severity = FE_LOG_INFO, LOG_CHANNEL channel = FE_LOG_GENERAL);
		std::vector<LogItem> getLogItems(LOG_CHANNEL channel);

		const int severityLevelsCount = 5;
		const int channelCount = 4;

		std::string severityLevelToString(LOG_SEVERITY severity);
		std::string channelTypeToString(LOG_CHANNEL channel);
	private:
		SINGLETON_PRIVATE_PART(FELOG)

		std::vector<std::unordered_map<int, LogItem>> channels;
	};

	#define LOG FELOG::getInstance()
}

namespace std
{
	template <> struct std::hash<FocalEngine::LogItem>
	{
		size_t operator()(const FocalEngine::LogItem& object) const
		{
			return ((std::hash<std::string>()(object.text) ^ (object.severity << 1)) >> 1);
		}
	};
}