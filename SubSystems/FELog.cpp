#include "FELog.h"
using namespace FocalEngine;
FELOG* FELOG::_instance = nullptr;

LogItem::LogItem()
{
	text = "";
	severity = FE_LOG_INFO;
	count = 0;
}

LogItem::~LogItem()
{
}

FELOG::FELOG()
{
	channels.resize(channelCount);
}

FELOG::~FELOG()
{
}

void FELOG::add(std::string text, LOG_SEVERITY severity, LOG_CHANNEL channel)
{
	if (severity < 0 || severity >= severityLevelsCount)
	{
		add("Incorrect severity argument" + std::to_string(severity) + " in FELOG::add function", FE_LOG_WARNING);
		return;
	}

	if (channel < 0 || channel >= channelCount)
	{
		add("Incorrect channel argument" + std::to_string(channel) + " in FELOG::add function", FE_LOG_WARNING);
		return;
	}

	LogItem tempItem;
	tempItem.text = text;
	tempItem.severity = severity;
	tempItem.count = 1;
	tempItem.channel = channel;
	tempItem.timeStamp = (long)GetTickCount64();

	auto logItem = channels[channel].find(std::hash<LogItem>()(tempItem));
	if (logItem == channels[channel].end())
	{
		channels[channel][std::hash<LogItem>()(tempItem)] = tempItem;
		return;
	}

	if (channels[channel][std::hash<LogItem>()(tempItem)].count < 1000)
		channels[channel][std::hash<LogItem>()(tempItem)].count++;
}

std::vector<LogItem> FELOG::getLogItems(LOG_CHANNEL channel)
{
	if (channel < 0 || channel >= channelCount)
	{
		add("Incorrect channel argument" + std::to_string(channel) + " in FELOG::getLogItems function", FE_LOG_WARNING);
		return std::vector<LogItem>();
	}

	std::vector<LogItem> result;
	auto iterator = channels[channel].begin();
	while (iterator != channels[channel].end())
	{
		result.push_back(iterator->second);
		iterator++;
	}

	return result;
}

std::string FELOG::severityLevelToString(LOG_SEVERITY severity)
{
	std::string result = "";
	if (severity < 0 || severity >= severityLevelsCount)
	{
		add("Incorrect severity argument" + std::to_string(severity) + " in FELOG::severityLevelToString function", FE_LOG_WARNING);
		return result;
	}

	switch (severity)
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

std::string FELOG::channelTypeToString(LOG_CHANNEL channel)
{
	std::string result = "";
	if (channel < 0 || channel >= channelCount)
	{
		add("Incorrect channel argument" + std::to_string(channel) + " in FELOG::channelTypeToString function", FE_LOG_WARNING);
		return result;
	}

	switch (channel)
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