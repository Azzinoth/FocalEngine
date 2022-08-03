#include "FELog.h"
using namespace FocalEngine;
FELOG* FELOG::Instance = nullptr;

LogItem::LogItem()
{
	Text = "";
	Severity = FE_LOG_INFO;
	Count = 0;
}

LogItem::~LogItem()
{
}

FELOG::FELOG()
{
	Channels.resize(ChannelCount);
}

FELOG::~FELOG()
{
}

void FELOG::Add(const std::string Text, const LOG_SEVERITY Severity, const LOG_CHANNEL Channel)
{
	if (Severity < 0 || Severity >= SeverityLevelsCount)
	{
		Add("Incorrect severity argument" + std::to_string(Severity) + " in FELOG::add function", FE_LOG_WARNING);
		return;
	}

	if (Channel < 0 || Channel >= ChannelCount)
	{
		Add("Incorrect channel argument" + std::to_string(Channel) + " in FELOG::add function", FE_LOG_WARNING);
		return;
	}

	LogItem TempItem;
	TempItem.Text = Text;
	TempItem.Severity = Severity;
	TempItem.Count = 1;
	TempItem.Channel = Channel;
	TempItem.TimeStamp = static_cast<long>(GetTickCount64());

	const auto CurrentLogItem = Channels[Channel].find(static_cast<int>(std::hash<LogItem>()(TempItem)));
	if (CurrentLogItem == Channels[Channel].end())
	{
		Channels[Channel][static_cast<int>(std::hash<LogItem>()(TempItem))] = TempItem;
		return;
	}

	if (Channels[Channel][static_cast<int>(std::hash<LogItem>()(TempItem))].Count < 1000)
		Channels[Channel][static_cast<int>(std::hash<LogItem>()(TempItem))].Count++;
}

std::vector<LogItem> FELOG::GetLogItems(const LOG_CHANNEL Channel)
{
	if (Channel < 0 || Channel >= ChannelCount)
	{
		Add("Incorrect channel argument" + std::to_string(Channel) + " in FELOG::getLogItems function", FE_LOG_WARNING);
		return std::vector<LogItem>();
	}

	std::vector<LogItem> result;
	auto iterator = Channels[Channel].begin();
	while (iterator != Channels[Channel].end())
	{
		result.push_back(iterator->second);
		iterator++;
	}

	return result;
}

std::string FELOG::SeverityLevelToString(const LOG_SEVERITY Severity)
{
	std::string result;
	if (Severity < 0 || Severity >= SeverityLevelsCount)
	{
		Add("Incorrect severity argument" + std::to_string(Severity) + " in FELOG::severityLevelToString function", FE_LOG_WARNING);
		return result;
	}

	switch (Severity)
	{
		case FocalEngine::FE_LOG_INFO:
		{
			result = "FE_LOG_INFO";
			break;
		}
		case FocalEngine::FE_LOG_DEBUG:
		{
			result = "FE_LOG_DEBUG";
			break;
		}
		case FocalEngine::FE_LOG_WARNING:
		{
			result = "FE_LOG_WARNING";
			break;
		}
		case FocalEngine::FE_LOG_ERROR:
		{
			result = "FE_LOG_ERROR";
			break;
		}
		case FocalEngine::FE_LOG_FATAL_ERROR:
		{
			result = "FE_LOG_FATAL_ERROR";
			break;
		}
	}

	return result;
}

std::string FELOG::ChannelTypeToString(const LOG_CHANNEL Channel)
{
	std::string result;
	if (Channel < 0 || Channel >= ChannelCount)
	{
		Add("Incorrect channel argument" + std::to_string(Channel) + " in FELOG::channelTypeToString function", FE_LOG_WARNING);
		return result;
	}

	switch (Channel)
	{
		case FocalEngine::FE_LOG_GENERAL:
		{
			result = "FE_LOG_GENERAL";
			break;
		}
		case FocalEngine::FE_LOG_LOADING:
		{
			result = "FE_LOG_LOADING";
			break;
		}
		case FocalEngine::FE_LOG_SAVING:
		{
			result = "FE_LOG_SAVING";
			break;
		}
		case FocalEngine::FE_LOG_RENDERING:
		{
			result = "FE_LOG_RENDERING";
			break;
		}
	}

	return result;
}