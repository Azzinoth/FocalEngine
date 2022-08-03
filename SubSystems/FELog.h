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
		std::string Text;
		LOG_SEVERITY Severity;
		int Count;
		LOG_CHANNEL Channel;
		long TimeStamp;

		LogItem();
		~LogItem();

		bool operator==(const LogItem& Other) const
		{
			return (Text == Other.Text && Severity == Other.Severity);
		}
	};

	class FELOG
	{
	public:
		SINGLETON_PUBLIC_PART(FELOG)

		void Add(std::string Text, LOG_SEVERITY Severity = FE_LOG_INFO, LOG_CHANNEL Channel = FE_LOG_GENERAL);
		std::vector<LogItem> GetLogItems(LOG_CHANNEL Channel);

		const int SeverityLevelsCount = 5;
		const int ChannelCount = 4;

		std::string SeverityLevelToString(LOG_SEVERITY Severity);
		std::string ChannelTypeToString(LOG_CHANNEL Channel);
	private:
		SINGLETON_PRIVATE_PART(FELOG)

		std::vector<std::unordered_map<int, LogItem>> Channels;
	};

	#define LOG FELOG::getInstance()
}

namespace std
{
	template <> struct std::hash<FocalEngine::LogItem>
	{
		size_t operator()(const FocalEngine::LogItem& Object) const
		{
			return ((std::hash<std::string>()(Object.Text) ^ (Object.Severity << 1)) >> 1);
		}
	};
}